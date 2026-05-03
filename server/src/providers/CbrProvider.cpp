#include "providers/CbrProvider.hpp"

#include "common/Exceptions.hpp"
#include "common/CurrencyNames.hpp"
#include "common/TimeUtils.hpp"
#include "common/Validation.hpp"

#if defined(_WIN32)
#include <windows.h>
#else
#include <iconv.h>
#endif

#include <tinyxml2.h>

#include <algorithm>
#include <cstdint>
#include <iomanip>
#include <map>
#include <optional>
#include <sstream>
#include <string_view>

namespace currency::providers {

namespace {

constexpr auto kProviderTimeout = std::chrono::seconds(10);

double parseLocalizedDouble(std::string value) {
    std::replace(value.begin(), value.end(), ',', '.');
    return std::stod(value);
}

std::string toCbrDate(const std::chrono::system_clock::time_point value) {
    const auto iso = common::toIsoString(value);
    return iso.substr(8, 2) + "/" + iso.substr(5, 2) + "/" + iso.substr(0, 4);
}

std::chrono::system_clock::time_point parseCbrDay(const std::string& value) {
    std::tm timeInfo{};
    std::istringstream input(value);
    input >> std::get_time(&timeInfo, "%d.%m.%Y");
    if (input.fail()) {
        throw common::ExternalApiError("CBR returned an invalid date value");
    }

    std::ostringstream iso;
    iso << std::put_time(&timeInfo, "%Y-%m-%d");
    return common::fromIsoString(iso.str() + "T00:00:00Z");
}

std::string windows1251ToUtf8(const std::string_view value) {
    if (value.empty()) {
        return {};
    }

#if defined(_WIN32)
    const auto wideSize = MultiByteToWideChar(1251, 0, value.data(), static_cast<int>(value.size()), nullptr, 0);
    if (wideSize <= 0) {
        return std::string(value);
    }

    std::wstring wideValue(wideSize, L'\0');
    MultiByteToWideChar(1251, 0, value.data(), static_cast<int>(value.size()), wideValue.data(), wideSize);

    const auto utf8Size = WideCharToMultiByte(CP_UTF8, 0, wideValue.data(), wideSize, nullptr, 0, nullptr, nullptr);
    if (utf8Size <= 0) {
        return std::string(value);
    }

    std::string result(utf8Size, '\0');
    WideCharToMultiByte(CP_UTF8, 0, wideValue.data(), wideSize, result.data(), utf8Size, nullptr, nullptr);
    return result;
#else
    iconv_t descriptor = iconv_open("UTF-8", "CP1251");
    if (descriptor == reinterpret_cast<iconv_t>(-1)) {
        return std::string(value);
    }

    std::string input(value);
    size_t inputLeft = input.size();
    char* inputPtr = input.data();

    std::string output(input.size() * 4 + 16, '\0');
    size_t outputLeft = output.size();
    char* outputPtr = output.data();

    if (iconv(descriptor, &inputPtr, &inputLeft, &outputPtr, &outputLeft) == static_cast<size_t>(-1)) {
        iconv_close(descriptor);
        return std::string(value);
    }

    iconv_close(descriptor);
    output.resize(output.size() - outputLeft);
    return output;
#endif
}

std::optional<double> resolveRate(
    const std::map<std::string, CbrProvider::DailyCurrency>& snapshot,
    const std::string& baseCode,
    const std::string& quoteCode) {
    if (baseCode == quoteCode) {
        return 1.0;
    }

    const auto base = snapshot.find(baseCode);
    const auto quote = snapshot.find(quoteCode);
    if (base == snapshot.end() || quote == snapshot.end() || base->second.rubRate <= 0.0 || quote->second.rubRate <= 0.0) {
        return std::nullopt;
    }

    return base->second.rubRate / quote->second.rubRate;
}

}

CbrProvider::CbrProvider(const HttpClient& httpClient)
    : httpClient_(httpClient) {}

std::string CbrProvider::key() const {
    return "cbr";
}

std::vector<QuoteSnapshot> CbrProvider::fetchLatest(
    const std::string& baseCode,
    const std::vector<std::string>& quoteCodes) const {
    const auto normalizedBase = common::normalizeCurrencyCode(baseCode);
    std::vector<std::string> normalizedQuotes;
    normalizedQuotes.reserve(quoteCodes.size());
    for (const auto& quoteCode : quoteCodes) {
        normalizedQuotes.push_back(common::normalizeCurrencyCode(quoteCode));
    }

    const auto [timestamp, snapshot] = fetchDailySnapshot();
    std::vector<QuoteSnapshot> quotes;

    for (const auto& quoteCode : normalizedQuotes) {
        const auto resolved = resolveRate(snapshot, normalizedBase, quoteCode);
        if (!resolved.has_value()) {
            continue;
        }

        quotes.push_back(QuoteSnapshot{
            .baseCode = normalizedBase,
            .quoteCode = quoteCode,
            .rate = *resolved,
            .timestamp = timestamp,
        });
    }

    if (quotes.empty()) {
        throw common::ExternalApiError("CBR does not expose the requested latest pair set");
    }

    return quotes;
}

std::vector<domain::HistoryPoint> CbrProvider::fetchHistory(
    const std::string& baseCode,
    const std::string& quoteCode,
    const std::chrono::system_clock::time_point from,
    const std::chrono::system_clock::time_point to) const {
    const auto normalizedBase = common::normalizeCurrencyCode(baseCode);
    const auto normalizedQuote = common::normalizeCurrencyCode(quoteCode);

    if (normalizedBase == normalizedQuote) {
        return {domain::HistoryPoint{from, 1.0}, domain::HistoryPoint{to, 1.0}};
    }

    const auto [_, snapshot] = fetchDailySnapshot();
    const auto baseIt = snapshot.find(normalizedBase);
    const auto quoteIt = snapshot.find(normalizedQuote);
    if (baseIt == snapshot.end() || quoteIt == snapshot.end()) {
        throw common::ExternalApiError("CBR does not expose one of the requested currencies");
    }

    if (normalizedBase == "RUB") {
        const auto quoteSeries = fetchDynamicHistory(quoteIt->second.id, from, to);
        std::vector<domain::HistoryPoint> points;
        for (const auto& [timestamp, quoteRubRate] : quoteSeries) {
            if (quoteRubRate <= 0.0) {
                continue;
            }
            points.push_back(domain::HistoryPoint{
                .timestamp = timestamp,
                .rate = 1.0 / quoteRubRate,
            });
        }
        std::ranges::sort(points, {}, &domain::HistoryPoint::timestamp);
        if (points.empty()) {
            throw common::ExternalApiError("CBR does not expose history for the requested pair");
        }
        return points;
    }

    if (normalizedQuote == "RUB") {
        const auto baseSeries = fetchDynamicHistory(baseIt->second.id, from, to);
        std::vector<domain::HistoryPoint> points;
        for (const auto& [timestamp, baseRubRate] : baseSeries) {
            if (baseRubRate <= 0.0) {
                continue;
            }
            points.push_back(domain::HistoryPoint{
                .timestamp = timestamp,
                .rate = baseRubRate,
            });
        }
        std::ranges::sort(points, {}, &domain::HistoryPoint::timestamp);
        if (points.empty()) {
            throw common::ExternalApiError("CBR does not expose history for the requested pair");
        }
        return points;
    }

    const auto baseSeries = fetchDynamicHistory(baseIt->second.id, from, to);
    const auto quoteSeries = fetchDynamicHistory(quoteIt->second.id, from, to);

    std::vector<domain::HistoryPoint> points;
    for (const auto& [timestamp, baseRubRate] : baseSeries) {
        const auto quoteRate = quoteSeries.find(timestamp);
        if (quoteRate == quoteSeries.end() || baseRubRate <= 0.0 || quoteRate->second <= 0.0) {
            continue;
        }

        points.push_back(domain::HistoryPoint{
            .timestamp = timestamp,
            .rate = baseRubRate / quoteRate->second,
        });
    }

    std::ranges::sort(points, {}, &domain::HistoryPoint::timestamp);

    if (points.empty()) {
        throw common::ExternalApiError("CBR does not expose history for the requested pair");
    }

    return points;
}

std::vector<domain::Currency> CbrProvider::fetchCurrencies() const {
    const auto [_, snapshot] = fetchDailySnapshot();
    std::vector<domain::Currency> currencies;
    currencies.reserve(snapshot.size());

    for (const auto& [code, currency] : snapshot) {
        currencies.push_back(domain::Currency{
            .code = code,
            .name = currency.name.empty() ? code : currency.name,
            .minorUnits = code == "JPY" ? static_cast<std::uint8_t>(0) : static_cast<std::uint8_t>(2),
        });
    }

    std::ranges::sort(currencies, {}, &domain::Currency::code);
    return currencies;
}

std::pair<std::chrono::system_clock::time_point, std::map<std::string, CbrProvider::DailyCurrency>> CbrProvider::fetchDailySnapshot() const {
    const auto payload = httpClient_.get("https://www.cbr.ru/scripts/XML_daily.asp", kProviderTimeout);
    tinyxml2::XMLDocument document;
    if (document.Parse(payload.c_str()) != tinyxml2::XML_SUCCESS) {
        throw common::ExternalApiError("CBR latest response is not valid XML");
    }

    auto* root = document.RootElement();
    if (root == nullptr || root->Attribute("Date") == nullptr) {
        throw common::ExternalApiError("CBR latest response does not contain a root date");
    }

    std::map<std::string, DailyCurrency> result;
    result.emplace("RUB", DailyCurrency{.id = "RUB", .name = common::russianCurrencyName("RUB", "Российский рубль"), .rubRate = 1.0});

    for (auto* valute = root->FirstChildElement("Valute"); valute != nullptr; valute = valute->NextSiblingElement("Valute")) {
        const auto* id = valute->Attribute("ID");
        const auto* charCode = valute->FirstChildElement("CharCode");
        const auto* name = valute->FirstChildElement("Name");
        const auto* nominal = valute->FirstChildElement("Nominal");
        const auto* value = valute->FirstChildElement("Value");
        if (id == nullptr || charCode == nullptr || nominal == nullptr || value == nullptr || charCode->GetText() == nullptr || nominal->GetText() == nullptr || value->GetText() == nullptr) {
            continue;
        }

        const auto currencyCode = common::normalizeCurrencyCode(charCode->GetText());
        const auto nominalValue = std::stod(nominal->GetText());
        const auto rawValue = parseLocalizedDouble(value->GetText());

        result[currencyCode] = DailyCurrency{
            .id = id,
            .name = name != nullptr && name->GetText() != nullptr ? windows1251ToUtf8(name->GetText()) : common::russianCurrencyName(currencyCode, currencyCode),
            .rubRate = nominalValue > 0.0 ? rawValue / nominalValue : rawValue,
        };
    }

    return {parseCbrDay(root->Attribute("Date")), result};
}

std::map<std::chrono::system_clock::time_point, double> CbrProvider::fetchDynamicHistory(
    const std::string& currencyId,
    const std::chrono::system_clock::time_point from,
    const std::chrono::system_clock::time_point to) const {
    const auto url =
        "https://www.cbr.ru/scripts/XML_dynamic.asp?date_req1=" + toCbrDate(from) +
        "&date_req2=" + toCbrDate(to) +
        "&VAL_NM_RQ=" + currencyId;

    const auto payload = httpClient_.get(url, kProviderTimeout);
    tinyxml2::XMLDocument document;
    if (document.Parse(payload.c_str()) != tinyxml2::XML_SUCCESS) {
        throw common::ExternalApiError("CBR history response is not valid XML");
    }

    std::map<std::chrono::system_clock::time_point, double> points;
    auto* root = document.RootElement();
    if (root == nullptr) {
        return points;
    }

    for (auto* record = root->FirstChildElement("Record"); record != nullptr; record = record->NextSiblingElement("Record")) {
        const auto* dateValue = record->Attribute("Date");
        const auto* nominal = record->FirstChildElement("Nominal");
        const auto* value = record->FirstChildElement("Value");
        if (dateValue == nullptr || nominal == nullptr || value == nullptr || nominal->GetText() == nullptr || value->GetText() == nullptr) {
            continue;
        }

        const auto timestamp = parseCbrDay(dateValue);
        const auto nominalValue = std::stod(nominal->GetText());
        const auto rawValue = parseLocalizedDouble(value->GetText());
        points[timestamp] = nominalValue > 0.0 ? rawValue / nominalValue : rawValue;
    }

    return points;
}

}
