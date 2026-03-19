#pragma once

#include <chrono>
#include <string>

namespace currency::domain {

struct ConversionResult {
    std::string provider;
    std::string fromCurrency;
    std::string toCurrency;
    double amount{0.0};
    double rate{0.0};
    double result{0.0};
    std::chrono::system_clock::time_point timestamp{};
};

}
