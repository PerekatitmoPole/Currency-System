#include "models/AppState.hpp"

#include "common/UiText.hpp"

#include <QSet>
#include <algorithm>

namespace currency::client::models {

namespace {

QString normalizeCurrencyCode(const QString& value) {
    return value.trimmed().toUpper();
}

QStringList normalizeCurrencyList(const QStringList& values) {
    QStringList result;
    QSet<QString> seen;
    for (const auto& value : values) {
        const auto code = normalizeCurrencyCode(value);
        if (!code.isEmpty() && !seen.contains(code)) {
            seen.insert(code);
            result.push_back(code);
        }
    }
    return result;
}

QStringList chooseQuoteDefaults(const QList<CurrencyViewModel>& currencies, const QString& baseCurrency) {
    const QStringList preferred{"USD", "GBP", "RUB", "JPY", "CNY", "CHF"};
    QSet<QString> available;
    for (const auto& currency : currencies) {
        available.insert(normalizeCurrencyCode(currency.code));
    }

    QStringList result;
    for (const auto& code : preferred) {
        if (code != baseCurrency && available.contains(code)) {
            result.push_back(code);
        }
    }

    for (const auto& currency : currencies) {
        const auto code = normalizeCurrencyCode(currency.code);
        if (code != baseCurrency && !result.contains(code)) {
            result.push_back(code);
        }
        if (result.size() >= 5) {
            break;
        }
    }

    return result;
}

}

AppState::AppState(QObject* parent)
    : QObject(parent) {
    connectionStatus_ = dto::ConnectionStatusDto{
        .host = serverHost_,
        .port = serverPort_,
        .connected = false,
        .message = common::UiText::defaultDisconnectedState(),
    };
}

void AppState::setSelectedSources(const QList<dto::ApiSource>& sources) {
    apiSelectionState_.setSelectedSources(sources);
    emit selectedSourcesChanged();
}

QList<dto::ApiSource> AppState::selectedSources() const {
    return apiSelectionState_.selectedSources();
}

void AppState::setAvailableCurrencies(const QList<CurrencyViewModel>& currencies) {
    currencies_ = currencies;
    if (!currencies_.isEmpty()) {
        QSet<QString> available;
        for (const auto& currency : currencies_) {
            available.insert(normalizeCurrencyCode(currency.code));
        }

        auto nextBase = normalizeCurrencyCode(defaultBaseCurrency_);
        if (!available.contains(nextBase)) {
            nextBase = available.contains("EUR") ? "EUR" : normalizeCurrencyCode(currencies_.first().code);
        }

        auto nextQuotes = normalizeCurrencyList(defaultQuoteCurrencies_);
        QStringList filteredQuotes;
        for (const auto& code : nextQuotes) {
            if (code != nextBase && available.contains(code)) {
                filteredQuotes.push_back(code);
            }
        }
        if (filteredQuotes.isEmpty()) {
            filteredQuotes = chooseQuoteDefaults(currencies_, nextBase);
        }

        const auto defaultsChanged = defaultBaseCurrency_ != nextBase || defaultQuoteCurrencies_ != filteredQuotes;
        defaultBaseCurrency_ = nextBase;
        defaultQuoteCurrencies_ = filteredQuotes;
        if (defaultsChanged) {
            emit settingsChanged();
        }
    }
    emit availableCurrenciesChanged();
}

QList<CurrencyViewModel> AppState::availableCurrencies() const {
    return currencies_;
}

void AppState::setCurrenciesForSource(const dto::ApiSource source, const QList<CurrencyViewModel>& currencies) {
    currenciesBySource_[source] = currencies;

    QHash<QString, CurrencyViewModel> merged;
    for (const auto& sourceCurrencies : currenciesBySource_) {
        for (const auto& currency : sourceCurrencies) {
            merged.insert(normalizeCurrencyCode(currency.code), currency);
        }
    }

    QList<CurrencyViewModel> allCurrencies;
    allCurrencies.reserve(merged.size());
    for (auto iterator = merged.cbegin(); iterator != merged.cend(); ++iterator) {
        allCurrencies.push_back(iterator.value());
    }
    std::sort(allCurrencies.begin(), allCurrencies.end(), [](const auto& left, const auto& right) {
        return left.code < right.code;
    });

    setAvailableCurrencies(allCurrencies);
    emit providerCurrenciesChanged();
}

QList<CurrencyViewModel> AppState::currenciesForSource(const dto::ApiSource source) const {
    const auto iterator = currenciesBySource_.find(source);
    if (iterator != currenciesBySource_.end() && !iterator.value().isEmpty()) {
        return iterator.value();
    }

    return currencies_;
}

void AppState::setNormalizedQuotes(const QList<dto::NormalizedQuoteDto>& quotes) {
    normalizedQuotes_ = quotes;
}

QList<dto::NormalizedQuoteDto> AppState::normalizedQuotes() const {
    return normalizedQuotes_;
}

void AppState::setLatestRates(const QList<RateViewModel>& rates) {
    latestRates_ = rates;
    emit latestRatesChanged();
}

QList<RateViewModel> AppState::latestRates() const {
    return latestRates_;
}

void AppState::setNormalizedHistory(const QList<dto::NormalizedHistoryPointDto>& points) {
    normalizedHistory_ = points;
}

QList<dto::NormalizedHistoryPointDto> AppState::normalizedHistory() const {
    return normalizedHistory_;
}

void AppState::setHistoryPoints(const QList<HistoryPointViewModel>& points) {
    historyPoints_ = points;
    emit historyChanged();
}

QList<HistoryPointViewModel> AppState::historyPoints() const {
    return historyPoints_;
}

void AppState::setAggregationResults(const QList<AggregationResultViewModel>& results) {
    aggregationResults_ = results;
    emit aggregationChanged();
}

QList<AggregationResultViewModel> AppState::aggregationResults() const {
    return aggregationResults_;
}

void AppState::setAggregationSummary(const dto::AggregationSummaryDto& summary) {
    aggregationSummary_ = summary;
    emit aggregationChanged();
}

dto::AggregationSummaryDto AppState::aggregationSummary() const {
    return aggregationSummary_;
}

void AppState::setConnectionStatus(const dto::ConnectionStatusDto& status) {
    connectionStatus_ = status;
    connectionStatus_.host = serverHost_;
    connectionStatus_.port = serverPort_;
    emit connectionStatusChanged();
}

dto::ConnectionStatusDto AppState::connectionStatus() const {
    return connectionStatus_;
}

void AppState::markServerConnected(const QString& message) {
    setConnectionStatus(dto::ConnectionStatusDto{
        .host = serverHost_,
        .port = serverPort_,
        .connected = true,
        .message = message.isEmpty() ? common::UiText::defaultConnectedState() : message,
    });
}

void AppState::markServerDisconnected(const QString& message) {
    setConnectionStatus(dto::ConnectionStatusDto{
        .host = serverHost_,
        .port = serverPort_,
        .connected = false,
        .message = message.isEmpty() ? common::UiText::defaultDisconnectedState() : message,
    });
}

void AppState::setServerHost(const QString& host) {
    serverHost_ = host.trimmed();
    connectionStatus_.host = serverHost_;
    emit settingsChanged();
    emit connectionStatusChanged();
}

QString AppState::serverHost() const {
    return serverHost_;
}

void AppState::setServerPort(const quint16 port) {
    serverPort_ = port;
    connectionStatus_.port = serverPort_;
    emit settingsChanged();
    emit connectionStatusChanged();
}

quint16 AppState::serverPort() const {
    return serverPort_;
}

void AppState::setDefaultBaseCurrency(const QString& baseCurrency) {
    defaultBaseCurrency_ = normalizeCurrencyCode(baseCurrency);
    defaultQuoteCurrencies_.removeAll(defaultBaseCurrency_);
    emit settingsChanged();
}

QString AppState::defaultBaseCurrency() const {
    return defaultBaseCurrency_;
}

void AppState::setDefaultQuoteCurrencies(const QStringList& quoteCurrencies) {
    defaultQuoteCurrencies_.clear();
    const auto normalizedBase = normalizeCurrencyCode(defaultBaseCurrency_);
    for (const auto& code : normalizeCurrencyList(quoteCurrencies)) {
        if (code != normalizedBase) {
            defaultQuoteCurrencies_.push_back(code);
        }
    }
    emit settingsChanged();
}

QStringList AppState::defaultQuoteCurrencies() const {
    return defaultQuoteCurrencies_;
}

void AppState::setHistoryRange(const QDate& from, const QDate& to) {
    historyFromDate_ = from;
    historyToDate_ = to;
    emit settingsChanged();
}

QDate AppState::historyFromDate() const {
    return historyFromDate_;
}

QDate AppState::historyToDate() const {
    return historyToDate_;
}

void AppState::setHistoryStep(const QString& step) {
    historyStep_ = step.trimmed();
    emit settingsChanged();
}

QString AppState::historyStep() const {
    return historyStep_;
}

void AppState::setConversionResult(const dto::ConversionResultDto& result) {
    conversionResult_ = result;
    emit conversionChanged();
}

dto::ConversionResultDto AppState::conversionResult() const {
    return conversionResult_;
}

}
