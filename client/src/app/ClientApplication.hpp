#pragma once

#include "MainWindow.hpp"
#include "controllers/DashboardController.hpp"
#include "controllers/NavigationController.hpp"
#include "controllers/SettingsController.hpp"
#include "controllers/StartPageController.hpp"
#include "gateways/ServerGateway.hpp"
#include "models/AggregationTableModel.hpp"
#include "models/AppState.hpp"
#include "models/CurrencyTableModel.hpp"
#include "serialization/TextProtocolCodec.hpp"
#include "services/AggregationService.hpp"
#include "services/ApiOrchestratorService.hpp"
#include "services/ChartPreparationService.hpp"
#include "services/ExportService.hpp"
#include "services/ServerSyncService.hpp"

#include <QApplication>
#include <QThread>

namespace currency::client::app {

class ClientApplication {
public:
    ClientApplication(QApplication& application);
    ~ClientApplication();

    int run();

private:
    void seedInitialState();
    void wireMessages();

    QApplication& application_;
    QThread networkThread_;
    serialization::TextProtocolCodec textProtocolCodec_;
    gateways::ServerGateway serverGateway_;
    models::AppState appState_;
    models::CurrencyTableModel currencyTableModel_;
    models::AggregationTableModel aggregationTableModel_;
    services::AggregationService aggregationService_;
    services::ChartPreparationService chartPreparationService_;
    services::ExportService exportService_;
    services::ApiOrchestratorService orchestratorService_;
    services::ServerSyncService serverSyncService_;
    MainWindow mainWindow_;
    controllers::NavigationController navigationController_;
    controllers::StartPageController startPageController_;
    controllers::DashboardController dashboardController_;
    controllers::SettingsController settingsController_;
};

}
