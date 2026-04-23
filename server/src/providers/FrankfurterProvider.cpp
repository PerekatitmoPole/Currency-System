#include "providers/FrankfurterProvider.hpp"

#include "common/Exceptions.hpp"
#include "common/CurrencyNames.hpp"
#include "common/TimeUtils.hpp"
#include "common/Validation.hpp"

#include <nlohmann/json.hpp>

#include <algorithm>
#include <cstdint>
#include <sstream>

namespace currency::providers {

namespace {

constexpr auto kProviderTimeout = std::chrono::seconds(10);

std::string joinList(const std::vector<std::string>& values) {
    std::ostringstream stream;
    for (std::size_t index = 0; index < values.size(); ++index) {
        if (index != 0) {
            stream << ',';
        }
        stream << values[index];
    }
    return stream.str();
}

std::string toDateString(const std::chrono::system_clock::time_point value) {
    return common::toIsoString(value).substr(0, 10);
}

}

FrankfurterProvider::FrankfurterProvider(const HttpClient& httpClient)
    : httpClient_(httpClient) {}

std::string FrankfurterProvider::key() const {
    return "frankfurter";
}

std::vector<QuoteSnapshot> FrankfurterProvider::fetchLatest(
    const std::string& baseCode,
    const std::vector<std::string>& quoteCodes) const {
    std::string url = "https://api.frankfurter.dev/v1/latest?base=" + common::normalizeCurrencyCode(baseCode);
    if (!quoteCodes.empty()) {
        std::vector<std::string> normalizedQuotes;
        normalizedQuotes.reserve(quoteCodes.size());
        for (const auto& quoteCode : quoteCodes) {
            normalizedQuotes.push_back(common::normalizeCurrencyCode(quoteCode));
        }
        url += "&symbols=" + joinList(normalizedQuotes);
    }

    const auto payload = httpClient_.get(url, kProviderTimeout);
    const auto json = nlohmann::json::parse(payload);
    const auto responseBase = json.at("base").get<std::string>();
    const auto timestamp = common::fromIsoString(json.at("date").get<std::string>() + "T00:00:00Z");

    std::vector<QuoteSnapshot> snapshots;
    for (const auto& [quoteCode, rateValue] : json.at("rates").items()) {
        snapshots.push_back(QuoteSnapshot{
            .baseCode = responseBase,
            .quoteCode = quoteCode,
            .rate = rateValue.get<double>(),
            .timestamp = timestamp,
        });
    }

    if (snapshots.empty()) {
        throw common::ExternalApiError("Frankfurter returned no latest rates");
    }

    return snapshots;
}

std::vector<domain::HistoryPoint> FrankfurterProvider::fetchHistory(
    const std::string& baseCode,
    const std::string& quoteCode,
    const std::chrono::system_clock::time_point from,
    const std::chrono::system_clock::time_point to) const {
    const auto normalizedBase = common::normalizeCurrencyCode(baseCode);
    const auto normalizedQuote = common::normalizeCurrencyCode(quoteCode);
    const auto url =
        "https://api.frankfurter.dev/v1/" + toDateString(from) + ".." + toDateString(to) +
        "?base=" + normalizedBase + "&symbols=" + normalizedQuote;

    const auto payload = httpClient_.get(url, kProviderTimeout);
    const auto json = nlohmann::json::parse(payload);

    std::vector<domain::HistoryPoint> points;
    for (const auto& [date, values] : json.at("rates").items()) {
        if (!values.contains(normalizedQuote)) {
            continue;
        }

        points.push_back(domain::HistoryPoint{
            .timestamp = common::fromIsoString(date + "T00:00:00Z"),
            .rate = values.at(normalizedQuote).get<double>(),
        });
    }

    std::ranges::sort(points, {}, &domain::HistoryPoint::timestamp);

    if (points.empty()) {
        throw common::ExternalApiError("Frankfurter returned no history for the requested pair");
    }

    return points;
}

std::vector<domain::Currency> FrankfurterProvider::fetchCurrencies() const {
    const auto payload = httpClient_.get("https://api.frankfurter.dev/v1/currencies", kProviderTimeout);
    const auto json = nlohmann::json::parse(payload);

    std::vector<domain::Currency> currencies;
    currencies.reserve(json.size());
    for (const auto& [code, name] : json.items()) {
        const auto normalizedCode = common::normalizeCurrencyCode(code);
        currencies.push_back(domain::Currency{
            .code = normalizedCode,
            .name = common::russianCurrencyName(normalizedCode, name.get<std::string>()),
            .minorUnits = normalizedCode == "JPY" ? static_cast<std::uint8_t>(0) : static_cast<std::uint8_t>(2),
        });
    }

    std::ranges::sort(currencies, {}, &domain::Currency::code);
    return currencies;
}

}
