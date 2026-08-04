// PicoADSB mbedtls config wrapper
// Includes the standard mbedtls config at the correct path for mbedtls 3.x

#ifndef PICOADSB_MBEDTLS_CONFIG_H
#define PICOADSB_MBEDTLS_CONFIG_H

#define MBEDTLS_NO_PLATFORM_ENTROPY
#define MBEDTLS_PLATFORM_MS_TIME_ALT

// Required for PEM certificate parsing (like pico-examples)
#define MBEDTLS_PEM_PARSE_C
#define MBEDTLS_BASE64_C

#include "mbedtls/mbedtls_config.h"

#undef MBEDTLS_TIMING_C
#undef MBEDTLS_FS_IO
#undef MBEDTLS_NET_C
#undef MBEDTLS_PSA_ITS_FILE_C
#undef MBEDTLS_PSA_CRYPTO_STORAGE_C

#endif // PICOADSB_MBEDTLS_CONFIG_H