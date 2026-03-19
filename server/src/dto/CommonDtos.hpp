#pragma once

#include <string>
#include <vector>

namespace currency::dto {

struct ErrorDto {
    std::string code;
    std::string message;
};

struct CurrencyDto {
    std::string code;
    std::string name;
    int minorUnits{2};
};

struct RateDto {
    std::string provider;
    std::string baseCode;
    std::string quoteCode;
    double rate{0.0};
    std::string sourceTimestamp;
};

struct HistoryPointDto {
    std::string timestamp;
    double rate{0.0};
};

}
