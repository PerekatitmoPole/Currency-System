#pragma once

#include <QList>
#include <QString>
#include <QtGlobal>
#include <optional>

namespace currency::client::dto {

enum class ApiSource {
    Cbr,
    Ecb,
    Frankfurter,
    ExchangeRateHost,
    CurrencyApi,
    OpenExchangeRates,
    AlphaVantage,
    TwelveData,
    Fixer,
    FreeCurrencyApi,
    CurrencyFreaks,
};

struct ApiSourceDescriptor {
    ApiSource source;
    QString stableKey;
    QString displayName;
    QString shortDescription;
    QString baseUrl;
    bool requiresApiKey{false};
    bool availableViaServer{false};
    bool supportsLatest{false};
    bool supportsHistory{false};
    bool supportsCurrencyCatalog{false};
};

const ApiSourceDescriptor& describeApiSource(ApiSource source);
QList<ApiSourceDescriptor> apiSourceCatalog();
QString toDisplayName(ApiSource source);
QString toStableKey(ApiSource source);
std::optional<ApiSource> fromStableKey(const QString& stableKey);
bool requiresApiKey(ApiSource source);
bool isServerAvailable(ApiSource source);
bool supportsLatest(ApiSource source);
bool supportsHistory(ApiSource source);
bool supportsCurrencyCatalog(ApiSource source);
QList<ApiSource> allApiSources();
QList<ApiSource> sourcesWithLatest();
QList<ApiSource> sourcesWithHistory();

}

uint qHash(currency::client::dto::ApiSource source, uint seed = 0) noexcept;
