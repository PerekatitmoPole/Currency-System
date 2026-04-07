#pragma once

#include <QList>
#include <QString>
#include <QtGlobal>

namespace currency::client::dto {

enum class ApiSource {
    Frankfurter,
    Ecb,
    CurrencyApi,
    OpenExchangeRates,
    ExchangeRateHost,
    Fixer,
    CurrencyFreaks,
    AlphaVantage,
    Cbr
};

QString toDisplayName(ApiSource source);
QString toStableKey(ApiSource source);
bool requiresApiKey(ApiSource source);
QList<ApiSource> allApiSources();

}

uint qHash(currency::client::dto::ApiSource source, uint seed = 0) noexcept;