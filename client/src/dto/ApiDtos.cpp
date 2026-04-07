#include "dto/ApiDtos.hpp"

#include <type_traits>

namespace currency::client::dto {

QString toDisplayName(const ApiSource source) {
    switch (source) {
    case ApiSource::Frankfurter:
        return "Frankfurter";
    case ApiSource::Ecb:
        return "ECB";
    case ApiSource::CurrencyApi:
        return "CurrencyAPI";
    case ApiSource::OpenExchangeRates:
        return "Open Exchange Rates";
    case ApiSource::ExchangeRateHost:
        return "exchangerate.host";
    case ApiSource::Fixer:
        return "Fixer";
    case ApiSource::CurrencyFreaks:
        return "CurrencyFreaks";
    case ApiSource::AlphaVantage:
        return "Alpha Vantage";
    case ApiSource::Cbr:
        return "ЦБ РФ";
    }

    return "Unknown";
}

QString toStableKey(const ApiSource source) {
    switch (source) {
    case ApiSource::Frankfurter:
        return "frankfurter";
    case ApiSource::Ecb:
        return "ecb";
    case ApiSource::CurrencyApi:
        return "currencyapi";
    case ApiSource::OpenExchangeRates:
        return "openexchangerates";
    case ApiSource::ExchangeRateHost:
        return "exchangeratehost";
    case ApiSource::Fixer:
        return "fixer";
    case ApiSource::CurrencyFreaks:
        return "currencyfreaks";
    case ApiSource::AlphaVantage:
        return "alphavantage";
    case ApiSource::Cbr:
        return "cbr";
    }

    return "unknown";
}

bool requiresApiKey(const ApiSource source) {
    switch (source) {
    case ApiSource::CurrencyApi:
    case ApiSource::OpenExchangeRates:
    case ApiSource::Fixer:
    case ApiSource::CurrencyFreaks:
    case ApiSource::AlphaVantage:
        return true;
    case ApiSource::Frankfurter:
    case ApiSource::Ecb:
    case ApiSource::ExchangeRateHost:
    case ApiSource::Cbr:
        return false;
    }

    return false;
}

QList<ApiSource> allApiSources() {
    return {
        ApiSource::Frankfurter,
        ApiSource::Ecb,
        ApiSource::CurrencyApi,
        ApiSource::OpenExchangeRates,
        ApiSource::ExchangeRateHost,
        ApiSource::Fixer,
        ApiSource::CurrencyFreaks,
        ApiSource::AlphaVantage,
        ApiSource::Cbr,
    };
}

}

uint qHash(const currency::client::dto::ApiSource source, const uint seed) noexcept {
    return ::qHash(static_cast<std::underlying_type_t<currency::client::dto::ApiSource>>(source), seed);
}