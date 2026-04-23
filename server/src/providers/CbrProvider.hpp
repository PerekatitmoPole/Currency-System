#pragma once

#include "providers/HttpClient.hpp"
#include "providers/MarketDataProvider.hpp"

#include <chrono>
#include <map>
#include <string>

namespace currency::providers {

class CbrProvider final : public MarketDataProvider {
public:
    struct DailyCurrency {
        std::string id;
        std::string name;
        double rubRate{0.0};
    };

    explicit CbrProvider(const HttpClient& httpClient);

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
    std::pair<std::chrono::system_clock::time_point, std::map<std::string, DailyCurrency>> fetchDailySnapshot() const;
    std::map<std::chrono::system_clock::time_point, double> fetchDynamicHistory(
        const std::string& currencyId,
        std::chrono::system_clock::time_point from,
        std::chrono::system_clock::time_point to) const;

    const HttpClient& httpClient_;
};

}
