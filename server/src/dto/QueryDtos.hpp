#pragma once

#include "dto/CommonDtos.hpp"

#include <string>
#include <vector>

namespace currency::dto {

struct GetCurrenciesResponseDto {
    std::vector<CurrencyDto> currencies;
};

struct GetRatesRequestDto {
    std::string provider;
    std::string baseCode;
    std::vector<std::string> quoteCodes;
};

struct GetRatesResponseDto {
    std::vector<RateDto> quotes;
};

struct GetHistoryRequestDto {
    std::string provider;
    std::string baseCode;
    std::string quoteCode;
    std::string from;
    std::string to;
    std::string step;
};

struct GetHistoryResponseDto {
    std::string provider;
    std::string baseCode;
    std::string quoteCode;
    std::string from;
    std::string to;
    std::string step;
    std::vector<HistoryPointDto> points;
};

struct ConvertRequestDto {
    std::string provider;
    std::string fromCurrency;
    std::string toCurrency;
    double amount{0.0};
};

struct ConvertResponseDto {
    std::string provider;
    std::string fromCurrency;
    std::string toCurrency;
    double amount{0.0};
    double rate{0.0};
    double result{0.0};
    std::string timestamp;
};

}
