#pragma once

#include <QObject>

namespace currency::client::models {
class AppState;
class AggregationTableModel;
}

namespace currency::client::services {
class ServerSyncService;
}

namespace currency::client::views {
class AggregationPage;
class DashboardPage;
}

namespace currency::client::controllers {

class AggregationController : public QObject {
    Q_OBJECT

public:
    AggregationController(
        views::AggregationPage& aggregationPage,
        views::DashboardPage& dashboardPage,
        models::AppState& appState,
        services::ServerSyncService& serverSyncService,
        models::AggregationTableModel& aggregationTableModel,
        QObject* parent = nullptr);

private:
    void updateView();
    void syncToServer();

    views::AggregationPage& aggregationPage_;
    views::DashboardPage& dashboardPage_;
    models::AppState& appState_;
    services::ServerSyncService& serverSyncService_;
    models::AggregationTableModel& aggregationTableModel_;
};

}