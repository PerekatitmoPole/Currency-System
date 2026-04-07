#pragma once

#include <QObject>

namespace currency::client::models {
class AppState;
class CurrencyTableModel;
class AggregationTableModel;
}

namespace currency::client::services {
class ApiOrchestratorService;
class AggregationService;
class ChartPreparationService;
class ServerSyncService;
}

namespace currency::client::views {
class DashboardPage;
}

namespace currency::client::controllers {
    
class DashboardController : public QObject {
    Q_OBJECT

public:
    DashboardController(
        views::DashboardPage& page,
        models::AppState& appState,
        services::ApiOrchestratorService& orchestrator,
        services::AggregationService& aggregationService,
        services::ChartPreparationService& chartPreparationService,
        services::ServerSyncService& serverSyncService,
        models::CurrencyTableModel& currencyTableModel,
        models::AggregationTableModel& aggregationTableModel,
        QObject* parent = nullptr);

private:
    void populateCurrencyCombos();
    void refreshQuotes();
    void loadHistory();
    void syncToServer();

    views::DashboardPage& page_;
    models::AppState& appState_;
    services::ApiOrchestratorService& orchestrator_;
    services::AggregationService& aggregationService_;
    services::ChartPreparationService& chartPreparationService_;
    services::ServerSyncService& serverSyncService_;
    models::CurrencyTableModel& currencyTableModel_;
    models::AggregationTableModel& aggregationTableModel_;
};

}