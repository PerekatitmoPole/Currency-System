#pragma once

#include <chrono>
#include <compare>
#include <string>

namespace currency::domain {

struct QuoteKey {
    std::string provider;
    std::string baseCurrency;
    std::string quoteCurrency;

    auto operator<=>(const QuoteKey&) const = default;

    std::string toString() const {
        return provider + '|' + baseCurrency + '|' + quoteCurrency;
    }
};

struct Quote {
    QuoteKey key;
    double rate{0.0};
    std::chrono::system_clock::time_point sourceTimestamp{};
    std::chrono::system_clock::time_point receivedAt{};
};

}
