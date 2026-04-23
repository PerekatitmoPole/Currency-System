#pragma once

#include "domain/Currency.hpp"
#include "domain/HistoryPoint.hpp"

#include <chrono>
#include <string>
#include <vector>

namespace currency::providers {

struct QuoteSnapshot {
    std::string baseCode;
    std::string quoteCode;
    double rate{0.0};
    std::chrono::system_clock::time_point timestamp{};
};

class MarketDataProvider {
public:
    virtual ~MarketDataProvider() = default;

    virtual std::string key() const = 0;
    virtual std::vector<QuoteSnapshot> fetchLatest(
        const std::string& baseCode,
        const std::vector<std::string>& quoteCodes) const = 0;
    virtual std::vector<domain::HistoryPoint> fetchHistory(
        const std::string& baseCode,
        const std::string& quoteCode,
        std::chrono::system_clock::time_point from,
        std::chrono::system_clock::time_point to) const = 0;
    virtual std::vector<domain::Currency> fetchCurrencies() const {
        return {};
    }
};

}
