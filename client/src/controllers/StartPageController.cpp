#include "controllers/StartPageController.hpp"

#include "app/MainWindow.hpp"
#include "models/AggregationTableModel.hpp"
#include "models/AppState.hpp"
#include "models/CurrencyTableModel.hpp"
#include "services/AggregationService.hpp"
#include "services/ApiOrchestratorService.hpp"
#include "views/pages/StartPage.hpp"
#include "views/widgets/ApiSelectorWidget.hpp"
#include "views/widgets/ConnectionStatusWidget.hpp"
#include "views/widgets/PreviewRatesWidget.hpp"

#include <QPushButton>

namespace currency::client::controllers {

StartPageController::StartPageController(
    views::StartPage& page,
    app::MainWindow& mainWindow,
    models::AppState& appState,
    services::ApiOrchestratorService& orchestrator,
    services::AggregationService& aggregationService,
    models::CurrencyTableModel& currencyTableModel,
    models::AggregationTableModel& aggregationTableModel,
    QObject* parent)
    : QObject(parent),
      page_(page),
      mainWindow_(mainWindow),
      appState_(appState),
      orchestrator_(orchestrator),
      aggregationService_(aggregationService),
      currencyTableModel_(currencyTableModel),
      aggregationTableModel_(aggregationTableModel) {
    connect(page_.apiSelector(), &views::ApiSelectorWidget::selectionChanged, &appState_, &models::AppState::setSelectedSources);
    connect(page_.aggregateButton(), &QPushButton::clicked, this, &StartPageController::runAggregation);
    connect(&appState_, &models::AppState::selectedSourcesChanged, &page_, [this] {
        page_.apiSelector()->setSelectedSources(appState_.selectedSources());
    });
    connect(&appState_, &models::AppState::latestRatesChanged, &page_, [this] {
        page_.previewRatesWidget()->setRates(appState_.latestRates());
    });
    connect(&appState_, &models::AppState::connectionStatusChanged, &page_, [this] {
        page_.connectionStatusWidget()->setStatus(appState_.connectionStatus());
    });

    page_.apiSelector()->setSelectedSources(appState_.selectedSources());
    page_.connectionStatusWidget()->setStatus(appState_.connectionStatus());
    page_.previewRatesWidget()->setRates(appState_.latestRates());
}

void StartPageController::runAggregation() {
    const auto selectedSources = page_.apiSelector()->selectedSources();
    if (selectedSources.isEmpty()) {
        emit appState_.errorRaised("Aggregation", "Select at least one API source before starting aggregation.");
        return;
    }

    appState_.setSelectedSources(selectedSources);
    const auto result = orchestrator_.fetchLatest(selectedSources, appState_.defaultBaseCurrency(), appState_.defaultQuoteCurrencies());
    if (!result.ok()) {
        emit appState_.errorRaised("Aggregation failed", result.error().message);
        return;
    }

    appState_.setNormalizedQuotes(result.value().quotes);
    const auto rateRows = aggregationService_.toRateViewModels(result.value().quotes);
    const auto aggregationRows = aggregationService_.aggregate(result.value().quotes);
    appState_.setLatestRates(rateRows);
    appState_.setAggregationResults(aggregationRows);
    appState_.setAggregationSummary(aggregationService_.summarize(result.value().quotes));
    currencyTableModel_.setRates(rateRows);
    aggregationTableModel_.setResults(aggregationRows);

    if (!result.value().warnings.isEmpty()) {
        emit appState_.infoRaised("Aggregation warnings", result.value().warnings.first().message);
    }

    mainWindow_.showPage(views::BurgerMenuWidget::Page::Dashboard);
}

}