#ifndef METAF_WRAPPER_HPP
#define METAF_WRAPPER_HPP

#include <cstdint>  // For uint32_t
#include <cstring>  // For strncpy, strlen
#include "./include/metaf.hpp"
#include <string>
#include <vector>

struct MetafResult {
    std::string reportType;
    std::string error;
    std::string location;
    std::string timestamp;
    bool isSpeci;
    bool isAutomated;
    bool isNil;
    bool isCancelled;
    bool isAmended;
    bool isCorrectional;

    std::vector<std::string> rawGroups;
};

// Parse a METAR/TAF report
MetafResult ParseMetaf(const std::string& report);

#endif // METAF_WRAPPER_HPP
