#include "controllers/HistoryController.hpp"

#include "dto/ApiDtos.hpp"
#include "models/AppState.hpp"
#include "models/HistoryTableModel.hpp"
#include "services/ApiOrchestratorService.hpp"
#include "services/ChartPreparationService.hpp"
#include "views/pages/HistoryPage.hpp"
#include "views/widgets/HistoryChartWidget.hpp"

#include <QComboBox>
#include <QDateEdit>
#include <QPushButton>
#include <QTableView>

namespace currency::client::controllers {

HistoryController::HistoryController(
    views::HistoryPage& page,
    models::AppState& appState,
    services::ApiOrchestratorService& orchestrator,
    services::ChartPreparationService& chartPreparationService,
    models::HistoryTableModel& historyTableModel,
    QObject* parent)
    : QObject(parent),
      page_(page),
      appState_(appState),
      orchestrator_(orchestrator),
      chartPreparationService_(chartPreparationService),
      historyTableModel_(historyTableModel) {
    page_.tableView()->setModel(&historyTableModel_);
    connect(page_.loadButton(), &QPushButton::clicked, this, &HistoryController::loadHistory);
    connect(&appState_, &models::AppState::availableCurrenciesChanged, this, &HistoryController::populateFilters);
    connect(&appState_, &models::AppState::selectedSourcesChanged, this, &HistoryController::populateFilters);
    connect(&appState_, &models::AppState::historyChanged, this, &HistoryController::updateHistoryView);
    populateFilters();
    updateHistoryView();
}

void HistoryController::populateFilters() {
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

    page_.sourceCombo()->clear();
    const auto selectedSources = appState_.selectedSources();
    const auto sources = selectedSources.isEmpty() ? dto::allApiSources() : selectedSources;
    for (const auto source : sources) {
        page_.sourceCombo()->addItem(dto::toDisplayName(source), static_cast<int>(source));
    }

    page_.fromDateEdit()->setDate(appState_.historyFromDate());
    page_.toDateEdit()->setDate(appState_.historyToDate());
}

void HistoryController::loadHistory() {
    const auto source = static_cast<dto::ApiSource>(page_.sourceCombo()->currentData().toInt());
    const auto from = page_.fromDateEdit()->date();
    const auto to = page_.toDateEdit()->date();
    appState_.setHistoryRange(from, to);

    const auto result = orchestrator_.fetchHistory(
        source,
        page_.baseCurrencyCombo()->currentText(),
        page_.quoteCurrencyCombo()->currentText(),
        from,
        to);

    if (!result.ok()) {
        emit appState_.errorRaised("History", result.error().message);
        return;
    }

    appState_.setNormalizedHistory(result.value());
    appState_.setHistoryPoints(chartPreparationService_.toHistoryViewModels(result.value()));
}

void HistoryController::updateHistoryView() {
    historyTableModel_.setHistoryPoints(appState_.historyPoints());
    page_.chartWidget()->setPoints(
        chartPreparationService_.prepareChartPoints(appState_.normalizedHistory()),
        QString("%1/%2 history").arg(page_.baseCurrencyCombo()->currentText(), page_.quoteCurrencyCombo()->currentText()));
}

}