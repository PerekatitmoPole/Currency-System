#pragma once

#include "providers/HttpClient.hpp"
#include "providers/MarketDataProvider.hpp"

#include <chrono>

namespace currency::providers {

class FrankfurterProvider final : public MarketDataProvider {
public:
    explicit FrankfurterProvider(const HttpClient& httpClient);

    std::string key() const override;
    std::vector<QuoteSnapshot> fetchLatest(
        const std::string& baseCode,
        const std::vector<std::string>& quoteCodes) const override;
    std::vector<domain::HistoryPoint> fetchHistory(
        const std::string& baseCode,
        const std::string& quoteCode,
        std::chrono::system_clock::time_point from,
        std::chrono::system_clock::time_point to) const override;
    std::vector<domain::Currency> fetchCurrencies() const override;

private:
    const HttpClient& httpClient_;
};

}
