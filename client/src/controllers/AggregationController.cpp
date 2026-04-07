#include "controllers/AggregationController.hpp"

#include "models/AggregationTableModel.hpp"
#include "models/AppState.hpp"
#include "services/ServerSyncService.hpp"
#include "views/pages/AggregationPage.hpp"
#include "views/pages/DashboardPage.hpp"
#include "views/widgets/AggregationSummaryWidget.hpp"

#include <QPushButton>

namespace currency::client::controllers {

AggregationController::AggregationController(
    views::AggregationPage& aggregationPage,
    views::DashboardPage& dashboardPage,
    models::AppState& appState,
    services::ServerSyncService& serverSyncService,
    models::AggregationTableModel& aggregationTableModel,
    QObject* parent)
    : QObject(parent),
      aggregationPage_(aggregationPage),
      dashboardPage_(dashboardPage),
      appState_(appState),
      serverSyncService_(serverSyncService),
      aggregationTableModel_(aggregationTableModel) {
    aggregationPage_.summaryWidget()->setModel(&aggregationTableModel_);
    connect(&appState_, &models::AppState::aggregationChanged, this, &AggregationController::updateView);
    connect(aggregationPage_.syncButton(), &QPushButton::clicked, this, &AggregationController::syncToServer);
    updateView();
}

void AggregationController::updateView() {
    aggregationPage_.summaryWidget()->setSummary(appState_.aggregationSummary());
    dashboardPage_.aggregationSummaryWidget()->setSummary(appState_.aggregationSummary());
}

void AggregationController::syncToServer() {
    const auto result = serverSyncService_.syncQuotes(appState_.normalizedQuotes());
    if (!result.ok()) {
        emit appState_.errorRaised("Server sync failed", result.error().message);
        return;
    }

    emit appState_.infoRaised("Server sync", "Normalized quotes were successfully sent to the server.");
}

}