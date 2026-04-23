#include "providers/EcbProvider.hpp"

#include "common/Exceptions.hpp"
#include "common/CurrencyNames.hpp"
#include "common/TimeUtils.hpp"
#include "common/Validation.hpp"

#include <tinyxml2.h>

#include <algorithm>
#include <cstdint>
#include <map>
#include <optional>

namespace currency::providers {

namespace {

constexpr auto kProviderTimeout = std::chrono::seconds(10);

std::optional<double> resolveRate(
    const std::map<std::string, double>& eurRates,
    const std::string& baseCode,
    const std::string& quoteCode) {
    if (baseCode == quoteCode) {
        return 1.0;
    }

    const auto base = eurRates.find(baseCode);
    const auto quote = eurRates.find(quoteCode);
    if (base == eurRates.end() || quote == eurRates.end() || base->second <= 0.0 || quote->second <= 0.0) {
        return std::nullopt;
    }

    return quote->second / base->second;
}

std::map<std::string, double> parseCubeRates(tinyxml2::XMLElement* timeCube) {
    std::map<std::string, double> rates;
    rates.emplace("EUR", 1.0);

    for (auto* cube = timeCube->FirstChildElement(); cube != nullptr; cube = cube->NextSiblingElement()) {
        const auto* currency = cube->Attribute("currency");
        const auto* rate = cube->Attribute("rate");
        if (currency == nullptr || rate == nullptr) {
            continue;
        }

        rates.emplace(common::normalizeCurrencyCode(currency), std::stod(rate));
    }

    return rates;
}

}

EcbProvider::EcbProvider(const HttpClient& httpClient)
    : httpClient_(httpClient) {}

std::string EcbProvider::key() const {
    return "ecb";
}

std::vector<QuoteSnapshot> EcbProvider::fetchLatest(
    const std::string& baseCode,
    const std::vector<std::string>& quoteCodes) const {
    const auto normalizedBase = common::normalizeCurrencyCode(baseCode);
    std::vector<std::string> normalizedQuotes;
    normalizedQuotes.reserve(quoteCodes.size());
    for (const auto& quoteCode : quoteCodes) {
        normalizedQuotes.push_back(common::normalizeCurrencyCode(quoteCode));
    }

    const auto payload = httpClient_.get("https://www.ecb.europa.eu/stats/eurofxref/eurofxref-daily.xml", kProviderTimeout);
    tinyxml2::XMLDocument document;
    if (document.Parse(payload.c_str()) != tinyxml2::XML_SUCCESS) {
        throw common::ExternalApiError("ECB latest response is not valid XML");
    }

    auto* root = document.RootElement();
    auto* envelopeCube = root != nullptr ? root->FirstChildElement("Cube") : nullptr;
    auto* timeCube = envelopeCube != nullptr ? envelopeCube->FirstChildElement("Cube") : nullptr;
    if (timeCube == nullptr || timeCube->Attribute("time") == nullptr) {
        throw common::ExternalApiError("ECB latest response does not contain a time cube");
    }

    const auto timestamp = common::fromIsoString(std::string(timeCube->Attribute("time")) + "T00:00:00Z");
    const auto eurRates = parseCubeRates(timeCube);

    std::vector<QuoteSnapshot> snapshots;
    for (const auto& quoteCode : normalizedQuotes) {
        const auto resolved = resolveRate(eurRates, normalizedBase, quoteCode);
        if (!resolved.has_value()) {
            continue;
        }

        snapshots.push_back(QuoteSnapshot{
            .baseCode = normalizedBase,
            .quoteCode = quoteCode,
            .rate = *resolved,
            .timestamp = timestamp,
        });
    }

    if (snapshots.empty()) {
        throw common::ExternalApiError("ECB does not expose the requested latest pair set");
    }

    return snapshots;
}

std::vector<domain::HistoryPoint> EcbProvider::fetchHistory(
    const std::string& baseCode,
    const std::string& quoteCode,
    const std::chrono::system_clock::time_point from,
    const std::chrono::system_clock::time_point to) const {
    const auto normalizedBase = common::normalizeCurrencyCode(baseCode);
    const auto normalizedQuote = common::normalizeCurrencyCode(quoteCode);

    const auto payload = httpClient_.get("https://www.ecb.europa.eu/stats/eurofxref/eurofxref-hist.xml", kProviderTimeout);
    tinyxml2::XMLDocument document;
    if (document.Parse(payload.c_str()) != tinyxml2::XML_SUCCESS) {
        throw common::ExternalApiError("ECB history response is not valid XML");
    }

    auto* root = document.RootElement();
    auto* envelopeCube = root != nullptr ? root->FirstChildElement("Cube") : nullptr;
    if (envelopeCube == nullptr) {
        throw common::ExternalApiError("ECB history response does not contain envelope data");
    }

    std::vector<domain::HistoryPoint> points;
    for (auto* timeCube = envelopeCube->FirstChildElement("Cube"); timeCube != nullptr; timeCube = timeCube->NextSiblingElement("Cube")) {
        const auto* timeValue = timeCube->Attribute("time");
        if (timeValue == nullptr) {
            continue;
        }

        const auto timestamp = common::fromIsoString(std::string(timeValue) + "T00:00:00Z");
        if (timestamp < from || timestamp > to) {
            continue;
        }

        const auto eurRates = parseCubeRates(timeCube);
        const auto resolved = resolveRate(eurRates, normalizedBase, normalizedQuote);
        if (!resolved.has_value()) {
            continue;
        }

        points.push_back(domain::HistoryPoint{
            .timestamp = timestamp,
            .rate = *resolved,
        });
    }

    std::ranges::sort(points, {}, &domain::HistoryPoint::timestamp);

    if (points.empty()) {
        throw common::ExternalApiError("ECB does not expose history for the requested pair");
    }

    return points;
}

std::vector<domain::Currency> EcbProvider::fetchCurrencies() const {
    const auto payload = httpClient_.get("https://www.ecb.europa.eu/stats/eurofxref/eurofxref-daily.xml", kProviderTimeout);
    tinyxml2::XMLDocument document;
    if (document.Parse(payload.c_str()) != tinyxml2::XML_SUCCESS) {
        throw common::ExternalApiError("ECB currency catalog response is not valid XML");
    }

    auto* root = document.RootElement();
    auto* envelopeCube = root != nullptr ? root->FirstChildElement("Cube") : nullptr;
    auto* timeCube = envelopeCube != nullptr ? envelopeCube->FirstChildElement("Cube") : nullptr;
    if (timeCube == nullptr) {
        throw common::ExternalApiError("ECB currency catalog response does not contain currency data");
    }

    const auto eurRates = parseCubeRates(timeCube);
    std::vector<domain::Currency> currencies;
    currencies.reserve(eurRates.size());
    for (const auto& [code, _] : eurRates) {
        currencies.push_back(domain::Currency{
            .code = code,
            .name = common::russianCurrencyName(code, code),
            .minorUnits = code == "JPY" ? static_cast<std::uint8_t>(0) : static_cast<std::uint8_t>(2),
        });
    }

    std::ranges::sort(currencies, {}, &domain::Currency::code);
    return currencies;
}

}
