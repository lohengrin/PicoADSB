#pragma once

// Maps ICAO aircraft type designators (API field `t`, e.g. "A339") to the
// usual commercial names (e.g. "A330-900neo") for display.
// Names are kept <= 12 characters so they fit the details pane.
// Unknown codes are displayed as-is.

#include <algorithm>
#include <string>

namespace adsb {

struct TypeEntry {
    const char* code;
    const char* name;
};

inline constexpr TypeEntry kTypeNames[]{
    // Airbus
    {"A306", "A300-600"},
    {"A310", "A310"},
    {"A318", "A318"},
    {"A319", "A319"},
    {"A19N", "A319neo"},
    {"A320", "A320"},
    {"A20N", "A320neo"},
    {"A321", "A321"},
    {"A21N", "A321neo"},
    {"A332", "A330-200"},
    {"A333", "A330-300"},
    {"A337", "A330 MRTT"},
    {"A338", "A330-800neo"},
    {"A339", "A330-900neo"},
    {"A359", "A350-900"},
    {"A35K", "A350-1000"},
    {"A388", "A380-800"},
    {"A3ST", "Beluga"},
    {"BCS1", "A220-100"},
    {"BCS3", "A220-300"},

    // Boeing
    {"B736", "B737-600"},
    {"B737", "B737-700"},
    {"B738", "B737-800"},
    {"B739", "B737-900"},
    {"B38M", "B737 MAX8"},
    {"B39M", "B737 MAX9"},
    {"B3XM", "B737 MAX10"},
    {"B744", "B747-400"},
    {"B748", "B747-8"},
    {"B752", "B757-200"},
    {"B753", "B757-300"},
    {"B762", "B767-200"},
    {"B763", "B767-300"},
    {"B764", "B767-400"},
    {"B772", "B777-200"},
    {"B77L", "B777-200LR"},
    {"B77E", "B777-200ER"},
    {"B773", "B777-300"},
    {"B77W", "B777-300ER"},
    {"B77F", "B777F"},
    {"B788", "B787-8"},
    {"B789", "B787-9"},
    {"B78X", "B787-10"},

    // Military / special
    {"A400", "A400M"},
    {"C130", "C-130"},
    {"C30J", "C-130J"},
    {"C17", "C-17"},
    {"KC46", "KC-46"},
    {"K35R", "KC-135"},
    {"E3TF", "E-3 Sentry"},
    {"F16", "F-16"},
    {"F15", "F-15"},

    // Embraer
    {"E135", "ERJ-135"},
    {"E145", "ERJ-145"},
    {"E170", "E170"},
    {"E175", "E175"},
    {"E75L", "E175-L"},
    {"E190", "E190"},
    {"E195", "E195"},
    {"E290", "E190-E2"},
    {"E295", "E195-E2"},
    {"E50P", "Phenom 100"},
    {"E55P", "Phenom 300"},

    // ATR / turboprops
    {"AT42", "ATR 42"},
    {"AT43", "ATR 42-300"},
    {"AT45", "ATR 42-500"},
    {"AT72", "ATR 72"},
    {"AT75", "ATR 72-500"},
    {"AT76", "ATR 72-600"},
    {"DH8A", "Dash 8-100"},
    {"DH8B", "Dash 8-200"},
    {"DH8C", "Dash 8-300"},
    {"DH8D", "Dash 8-Q400"},
    {"SF34", "Saab 340"},
    {"SB20", "Saab 2000"},
    {"C208", "Caravan"},
    {"PC12", "PC-12"},
    {"PC24", "PC-24"},
    {"TBM7", "TBM 700"},
    {"TBM8", "TBM 850"},
    {"TBM9", "TBM 900"},
    {"BE20", "King Air 200"},
    {"B350", "King Air 350"},
    {"M28", "Skytruck"},

    // Regional jets
    {"CRJ2", "CRJ-200"},
    {"CRJ7", "CRJ-700"},
    {"CRJ9", "CRJ-900"},
    {"CRJX", "CRJ-1000"},

    // Business jets
    {"CL35", "Chall 350"},
    {"CL60", "Chall 600"},
    {"GLEX", "Global"},
    {"GLF4", "G-IV"},
    {"GLF5", "G550"},
    {"GLF6", "G650"},
    {"FA20", "Falcon 20"},
    {"FA50", "Falcon 50"},
    {"F2TH", "Falcon 2000"},
    {"FA7X", "Falcon 7X"},
    {"FA8X", "Falcon 8X"},
    {"C25A", "Citation CJ1"},
    {"C25B", "Citation CJ2"},
    {"C25C", "Citation CJ3"},
    {"C25M", "Citation CJ4"},
    {"C56X", "Citation XLS"},
    {"C68A", "Citation Lat"},
    {"C700", "Citation Lon"},
    {"LJ45", "Learjet 45"},
    {"LJ75", "Learjet 75"},
    {"H25B", "Hawker 800"},

    // General aviation
    {"C152", "Cessna 152"},
    {"C172", "Cessna 172"},
    {"C182", "Cessna 182"},
    {"SR22", "Cirrus SR22"},
    {"PA28", "PA-28"},
    {"DA40", "DA40"},
    {"DA42", "DA42"},

    // Helicopters
    {"AS50", "AS350"},
    {"A139", "AW139"},
    {"EC35", "H135"},
    {"EC45", "H145"},
    {"B407", "Bell 407"},
    {"R44", "Robinson R44"},
};

// Maximum displayable name length in the details pane
// (12 chars x 7 px = 84 px, the value column width).
inline constexpr size_t kMaxTypeNameLen = 12;

// Returns the friendly name for an ICAO type code, or the code itself when it
// is not in the table. The result is clamped to kMaxTypeNameLen characters.
inline std::string typeName(const std::string& icao) {
    const auto it = std::find_if(std::begin(kTypeNames), std::end(kTypeNames),
                                 [&icao](const TypeEntry& e) { return icao == e.code; });
    std::string name = (it != std::end(kTypeNames)) ? it->name : icao;
    if (name.size() > kMaxTypeNameLen) name.resize(kMaxTypeNameLen);
    return name;
}

}  // namespace adsb
