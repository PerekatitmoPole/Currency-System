#include "controllers/DashboardController.hpp"

#include "models/AggregationTableModel.hpp"
#include "models/AppState.hpp"
#include "models/CurrencyTableModel.hpp"
#include "services/AggregationService.hpp"
#include "services/ApiOrchestratorService.hpp"
#include "services/ChartPreparationService.hpp"
#include "services/ServerSyncService.hpp"
#include "views/pages/DashboardPage.hpp"
#include "views/widgets/AggregationSummaryWidget.hpp"
#include "views/widgets/HistoryChartWidget.hpp"
#include "views/widgets/RatesTableWidget.hpp"

#include <QComboBox>
#include <QPushButton>

namespace currency::client::controllers {

DashboardController::DashboardController(
    views::DashboardPage& page,
    models::AppState& appState,
    services::ApiOrchestratorService& orchestrator,
    services::AggregationService& aggregationService,
    services::ChartPreparationService& chartPreparationService,
    services::ServerSyncService& serverSyncService,
    models::CurrencyTableModel& currencyTableModel,
    models::AggregationTableModel& aggregationTableModel,
    QObject* parent)
    : QObject(parent),
      page_(page),
      appState_(appState),
      orchestrator_(orchestrator),
      aggregationService_(aggregationService),
      chartPreparationService_(chartPreparationService),
      serverSyncService_(serverSyncService),
      currencyTableModel_(currencyTableModel),
      aggregationTableModel_(aggregationTableModel) {
    page_.ratesTableWidget()->setModel(&currencyTableModel_);
    page_.aggregationSummaryWidget()->setModel(&aggregationTableModel_);

    connect(page_.refreshButton(), &QPushButton::clicked, this, &DashboardController::refreshQuotes);
    connect(page_.loadHistoryButton(), &QPushButton::clicked, this, &DashboardController::loadHistory);
    connect(page_.syncButton(), &QPushButton::clicked, this, &DashboardController::syncToServer);
    connect(&appState_, &models::AppState::availableCurrenciesChanged, this, &DashboardController::populateCurrencyCombos);
    connect(&appState_, &models::AppState::historyChanged, &page_, [this] {
        page_.historyChartWidget()->setPoints(
            chartPreparationService_.prepareChartPoints(appState_.normalizedHistory()),
            QString("%1/%2 history").arg(page_.baseCurrencyCombo()->currentText(), page_.quoteCurrencyCombo()->currentText()));
    });
    connect(&appState_, &models::AppState::aggregationChanged, &page_, [this] {
        page_.aggregationSummaryWidget()->setSummary(appState_.aggregationSummary());
    });

    populateCurrencyCombos();
    page_.aggregationSummaryWidget()->setSummary(appState_.aggregationSummary());
}

void DashboardController::populateCurrencyCombos() {
    const auto currentBase = page_.baseCurrencyCombo()->currentText();
    const auto currentQuote = page_.quoteCurrencyCombo()->currentText();
    page_.baseCurrencyCombo()->clear();
    page_.quoteCurrencyCombo()->clear();
    for (const auto& currency : appState_.availableCurrencies()) {
        page_.baseCurrencyCombo()->addItem(currency.code);
        page_.quoteCurrencyCombo()->addItem(currency.code);
    }

    const auto baseIndex = page_.baseCurrencyCombo()->findText(currentBase.isEmpty() ? appState_.defaultBaseCurrency() : currentBase);
    const auto quoteIndex = page_.quoteCurrencyCombo()->findText(currentQuote.isEmpty() ? appState_.defaultQuoteCurrencies().value(0) : currentQuote);
    if (baseIndex >= 0) {
        page_.baseCurrencyCombo()->setCurrentIndex(baseIndex);
    }
    if (quoteIndex >= 0) {
        page_.quoteCurrencyCombo()->setCurrentIndex(quoteIndex);
    }
}

void DashboardController::refreshQuotes() {
    const auto selectedSources = appState_.selectedSources();
    const auto baseCurrency = page_.baseCurrencyCombo()->currentText();
    auto quoteCurrencies = appState_.defaultQuoteCurrencies();
    const auto currentQuote = page_.quoteCurrencyCombo()->currentText();
    if (!currentQuote.isEmpty() && !quoteCurrencies.contains(currentQuote)) {
        quoteCurrencies.push_back(currentQuote);
    }

    const auto result = orchestrator_.fetchLatest(selectedSources, baseCurrency, quoteCurrencies);
    if (!result.ok()) {
        emit appState_.errorRaised("Dashboard refresh failed", result.error().message);
        return;
    }

    appState_.setNormalizedQuotes(result.value().quotes);
    const auto rateRows = aggregationService_.toRateViewModels(result.value().quotes);
    const auto aggregationRows = aggregationService_.aggregate(result.value().quotes);
    currencyTableModel_.setRates(rateRows);
    aggregationTableModel_.setResults(aggregationRows);
    appState_.setLatestRates(rateRows);
    appState_.setAggregationResults(aggregationRows);
    appState_.setAggregationSummary(aggregationService_.summarize(result.value().quotes));
}

void DashboardController::loadHistory() {
    const auto selectedSources = appState_.selectedSources();
    if (selectedSources.isEmpty()) {
        emit appState_.errorRaised("History", "Select at least one API source before loading history.");
        return;
    }

    const auto result = orchestrator_.fetchHistory(
        selectedSources.first(),
        page_.baseCurrencyCombo()->currentText(),
        page_.quoteCurrencyCombo()->currentText(),
        appState_.historyFromDate(),
        appState_.historyToDate());

    if (!result.ok()) {
        emit appState_.errorRaised("History failed", result.error().message);
        return;
    }

    appState_.setNormalizedHistory(result.value());
    appState_.setHistoryPoints(chartPreparationService_.toHistoryViewModels(result.value()));
}

void DashboardController::syncToServer() {
    const auto result = serverSyncService_.syncQuotes(appState_.normalizedQuotes());
    if (!result.ok()) {
        emit appState_.errorRaised("Server sync failed", result.error().message);
        return;
    }

    emit appState_.infoRaised("Server sync", "Normalized quotes were successfully sent to the server.");
}

}