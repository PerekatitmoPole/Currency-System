#pragma once

#include "dto/ApiDtos.hpp"
#include "dto/NormalizedDtos.hpp"
#include "dto/UiDtos.hpp"
#include "models/ApiSelectionState.hpp"
#include "models/viewmodels/AggregationResultViewModel.hpp"
#include "models/viewmodels/CurrencyViewModel.hpp"
#include "models/viewmodels/HistoryPointViewModel.hpp"
#include "models/viewmodels/RateViewModel.hpp"

#include <QDate>
#include <QHash>
#include <QList>
#include <QObject>
#include <QStringList>

namespace currency::client::models {

class AppState : public QObject {
    Q_OBJECT

public:
    AppState(QObject* parent = nullptr);

    void setSelectedSources(const QList<dto::ApiSource>& sources);
    QList<dto::ApiSource> selectedSources() const;

    void setAvailableCurrencies(const QList<CurrencyViewModel>& currencies);
    QList<CurrencyViewModel> availableCurrencies() const;

    void setNormalizedQuotes(const QList<dto::NormalizedQuoteDto>& quotes);
    QList<dto::NormalizedQuoteDto> normalizedQuotes() const;

    void setLatestRates(const QList<RateViewModel>& rates);
    QList<RateViewModel> latestRates() const;

    void setNormalizedHistory(const QList<dto::NormalizedHistoryPointDto>& points);
    QList<dto::NormalizedHistoryPointDto> normalizedHistory() const;

    void setHistoryPoints(const QList<HistoryPointViewModel>& points);
    QList<HistoryPointViewModel> historyPoints() const;

    void setAggregationResults(const QList<AggregationResultViewModel>& results);
    QList<AggregationResultViewModel> aggregationResults() const;

    void setAggregationSummary(const dto::AggregationSummaryDto& summary);
    dto::AggregationSummaryDto aggregationSummary() const;

    void setConnectionStatus(const dto::ConnectionStatusDto& status);
    dto::ConnectionStatusDto connectionStatus() const;

    void setServerHost(const QString& host);
    QString serverHost() const;

    void setServerPort(quint16 port);
    quint16 serverPort() const;

    void setDefaultBaseCurrency(const QString& baseCurrency);
    QString defaultBaseCurrency() const;

    void setDefaultQuoteCurrencies(const QStringList& quoteCurrencies);
    QStringList defaultQuoteCurrencies() const;

    void setHistoryRange(const QDate& from, const QDate& to);
    QDate historyFromDate() const;
    QDate historyToDate() const;

    void setApiKey(dto::ApiSource source, const QString& apiKey);
    QString apiKey(dto::ApiSource source) const;

signals:
    void selectedSourcesChanged();
    void availableCurrenciesChanged();
    void latestRatesChanged();
    void historyChanged();
    void aggregationChanged();
    void connectionStatusChanged();
    void settingsChanged();
    void errorRaised(const QString& title, const QString& message);
    void infoRaised(const QString& title, const QString& message);

private:
    ApiSelectionState apiSelectionState_;
    QList<CurrencyViewModel> currencies_;
    QList<dto::NormalizedQuoteDto> normalizedQuotes_;
    QList<RateViewModel> latestRates_;
    QList<dto::NormalizedHistoryPointDto> normalizedHistory_;
    QList<HistoryPointViewModel> historyPoints_;
    QList<AggregationResultViewModel> aggregationResults_;
    dto::AggregationSummaryDto aggregationSummary_;
    dto::ConnectionStatusDto connectionStatus_;
    QString serverHost_{"127.0.0.1"};
    quint16 serverPort_{5555};
    QString defaultBaseCurrency_{"EUR"};
    QStringList defaultQuoteCurrencies_{"USD", "GBP", "RUB", "JPY", "CNY"};
    QDate historyFromDate_{QDate::currentDate().addDays(-30)};
    QDate historyToDate_{QDate::currentDate()};
    QHash<dto::ApiSource, QString> apiKeys_;
};

}