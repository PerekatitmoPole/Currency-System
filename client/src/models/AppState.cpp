#include "models/AppState.hpp"

namespace currency::client::models {

AppState::AppState(QObject* parent)
    : QObject(parent) {
    connectionStatus_ = dto::ConnectionStatusDto{
        .host = serverHost_,
        .port = serverPort_,
        .connected = false,
        .message = "Server disconnected",
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
    emit availableCurrenciesChanged();
}

QList<CurrencyViewModel> AppState::availableCurrencies() const {
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
    defaultBaseCurrency_ = baseCurrency.trimmed().toUpper();
    emit settingsChanged();
}

QString AppState::defaultBaseCurrency() const {
    return defaultBaseCurrency_;
}

void AppState::setDefaultQuoteCurrencies(const QStringList& quoteCurrencies) {
    defaultQuoteCurrencies_ = quoteCurrencies;
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

void AppState::setApiKey(const dto::ApiSource source, const QString& apiKey) {
    apiKeys_[source] = apiKey.trimmed();
    emit settingsChanged();
}

QString AppState::apiKey(const dto::ApiSource source) const {
    return apiKeys_.value(source);
}

}