#include "net/https_client.hpp"

#include <cstring>
#include <cstdio>

#include "pico/stdlib.h"
#include "pico/cyw43_arch.h"
#include "lwip/pbuf.h"
#include "lwip/altcp_tcp.h"
#include "lwip/altcp_tls.h"
#include "lwip/dns.h"
#include "mbedtls/ssl.h"

#include "config.h"
#include "net/isrg_root_yr_pem.h"

namespace net {

static struct altcp_tls_config* tls_config = nullptr;

struct TlsState {
    struct altcp_pcb* pcb = nullptr;
    bool complete = false;
    int error = 0;
    const char* request = nullptr;
    int timeout = 15;
    std::string* body = nullptr;
    std::string raw;
    int statusCode = 0;
    bool headersDone = false;
    size_t headerEnd = 0;
    int contentLength = -1;
};

static err_t tlsClose(TlsState* s) {
    s->complete = true;
    if (s->pcb) {
        altcp_arg(s->pcb, nullptr);
        altcp_poll(s->pcb, nullptr, 0);
        altcp_recv(s->pcb, nullptr);
        altcp_err(s->pcb, nullptr);
        err_t err = altcp_close(s->pcb);
        if (err != ERR_OK) {
            printf("close failed %d, aborting\n", err);
            altcp_abort(s->pcb);
            err = ERR_ABRT;
        }
        s->pcb = nullptr;
    }
    return ERR_OK;
}

static err_t tlsConnected(void* arg, struct altcp_pcb* pcb, err_t err) {
    TlsState* s = static_cast<TlsState*>(arg);
    if (err != ERR_OK) {
        printf("connect failed %d\n", err);
        return tlsClose(s);
    }
    printf("connected, sending request\n");
    err = altcp_write(pcb, s->request, strlen(s->request), TCP_WRITE_FLAG_COPY);
    if (err != ERR_OK) {
        printf("write error %d\n", err);
        return tlsClose(s);
    }
    return ERR_OK;
}

static err_t tlsPoll(void* arg, struct altcp_pcb* pcb) {
    TlsState* s = static_cast<TlsState*>(arg);
    printf("timeout\n");
    s->error = PICO_ERROR_TIMEOUT;
    return tlsClose(s);
}

static void tlsErr(void* arg, err_t err) {
    TlsState* s = static_cast<TlsState*>(arg);
    printf("tls err %d\n", err);
    tlsClose(s);
    s->error = PICO_ERROR_GENERIC;
}

static err_t tlsRecv(void* arg, struct altcp_pcb* pcb, struct pbuf* p, err_t err) {
    TlsState* s = static_cast<TlsState*>(arg);
    if (!p) {
        printf("connection closed\n");
        tlsClose(s);
        return ERR_OK;
    }

    if (p->tot_len > 0) {
        // Append received data to raw buffer (cap to avoid OOM)
        const size_t maxRaw = HTTP_MAX_BODY + 8192;
        if (s->raw.size() < maxRaw) {
            size_t copyLen = p->tot_len;
            if (s->raw.size() + copyLen > maxRaw) copyLen = maxRaw - s->raw.size();
            char* buf = new char[copyLen];
            pbuf_copy_partial(p, buf, copyLen, 0);
            s->raw.append(buf, copyLen);
            delete[] buf;
        }
        altcp_recved(pcb, p->tot_len);
    }
    pbuf_free(p);

    // Check if headers are complete
    if (!s->headersDone) {
        size_t h = s->raw.find("\r\n\r\n");
        if (h != std::string::npos) {
            s->headersDone = true;
            s->headerEnd = h + 4;
            // Parse status line and headers
            std::string_view headers(s->raw.c_str(), h);
            // Status line
            size_t sp1 = headers.find(' ');
            if (sp1 != std::string::npos) {
                size_t sp2 = headers.find(' ', sp1 + 1);
                if (sp2 != std::string::npos) {
                    std::string codeStr = std::string(headers.substr(sp1 + 1, sp2 - sp1 - 1));
                    s->statusCode = atoi(codeStr.c_str());
                }
            }
            // Content-Length
            size_t clPos = headers.find("content-length:");
            if (clPos != std::string::npos) {
                size_t valStart = headers.find_first_not_of(" \t", clPos + 15);
                size_t valEnd = headers.find("\r\n", valStart);
                if (valStart != std::string::npos) {
                    std::string lenStr = std::string(headers.substr(valStart, valEnd - valStart));
                    s->contentLength = atoi(lenStr.c_str());
                }
            }
        }
    }
    return ERR_OK;
}

static void tlsConnectIp(const ip_addr_t* ipaddr, TlsState* s) {
    printf("connecting to %s:%d\n", ipaddr_ntoa(ipaddr), 443);
    err_t err = altcp_connect(s->pcb, ipaddr, 443, tlsConnected);
    if (err != ERR_OK) {
        fprintf(stderr, "connect err %d\n", err);
        tlsClose(s);
    }
}

static void tlsDnsFound(const char* hostname, const ip_addr_t* ipaddr, void* arg) {
    if (ipaddr) {
        printf("DNS resolved\n");
        tlsConnectIp(ipaddr, static_cast<TlsState*>(arg));
    } else {
        printf("DNS failed for %s\n", hostname);
        tlsClose(static_cast<TlsState*>(arg));
    }
}

static bool tlsOpen(const char* hostname, TlsState* s) {
    s->pcb = altcp_tls_new(tls_config, IPADDR_TYPE_ANY);
    if (!s->pcb) {
        printf("failed to create pcb\n");
        return false;
    }

    altcp_arg(s->pcb, s);
    altcp_poll(s->pcb, tlsPoll, s->timeout * 2);
    altcp_recv(s->pcb, tlsRecv);
    altcp_err(s->pcb, tlsErr);

    // Set SNI
    mbedtls_ssl_set_hostname((mbedtls_ssl_context*)altcp_tls_context(s->pcb), hostname);

    cyw43_arch_lwip_begin();
    err_t err = dns_gethostbyname(hostname, nullptr, tlsDnsFound, s);
    if (err == ERR_OK) {
        // In cache, will be called synchronously? Need to handle.
        // Actually dns_gethostbyname returns ERR_OK only if immediately available,
        // and tlsDnsFound is NOT called synchronously. So we just wait.
    } else if (err != ERR_INPROGRESS) {
        printf("DNS start failed %d\n", err);
        tlsClose(s);
    }
    cyw43_arch_lwip_end();
    return err == ERR_OK || err == ERR_INPROGRESS;
}

static bool ensureTlsConfig() {
    if (tls_config) return true;
    printf("Creating TLS config with ISRG Root YR PEM cert len=%zu...\n", isrg_root_yr_pem_len);
    tls_config = altcp_tls_create_config_client((const uint8_t*)isrg_root_yr_pem, isrg_root_yr_pem_len);
    if (!tls_config) {
        printf("failed to create TLS config (altcp_tls_create_config_client returned NULL)\n");
        return false;
    }
    // Auth mode is set via ALTCP_MBEDTLS_AUTHMODE in lwipopts.h (VERIFY_REQUIRED)
    printf("TLS config created successfully\n");
    return true;
}

bool httpsGet(const char* host, const char* path, std::string& outBody) {
    outBody.clear();

    if (!ensureTlsConfig()) return false;

    // Build HTTP request
    std::string request = "GET ";
    request += path;
    request += " HTTP/1.1\r\nHost: ";
    request += host;
    request += "\r\nConnection: close\r\nUser-Agent: PicoADSB/1.0\r\n\r\n";

    TlsState state;
    state.request = request.c_str();
    state.timeout = 20;
    state.body = &outBody;

    if (!tlsOpen(host, &state)) return false;

    // Wait for completion
    while (!state.complete) {
        sleep_ms(1000);
    }

    int err = state.error;
    if (err != 0) {
        printf("TLS error %d\n", err);
        return false;
    }
    if (state.statusCode != 200) {
        printf("HTTP %d\n", state.statusCode);
        return false;
    }

    // Extract body
    if (state.headersDone) {
        if (state.contentLength >= 0 && state.raw.size() >= state.headerEnd + (size_t)state.contentLength) {
            outBody = state.raw.substr(state.headerEnd, state.contentLength);
        } else {
            // Fallback: everything after headers
            if (state.raw.size() > state.headerEnd) {
                outBody = state.raw.substr(state.headerEnd);
            }
        }
    }

    return !outBody.empty();
}

}  // namespace net