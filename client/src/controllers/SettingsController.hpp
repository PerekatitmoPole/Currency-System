#pragma once

#include <QObject>

namespace currency::client::models {
class AppState;
}

namespace currency::client::services {
class ApiOrchestratorService;
class ServerSyncService;
}

namespace currency::client::views {
class SettingsPage;
}

namespace currency::client::controllers {

class SettingsController : public QObject {
    Q_OBJECT

public:
    SettingsController(
        views::SettingsPage& page,
        models::AppState& appState,
        services::ApiOrchestratorService& orchestrator,
        services::ServerSyncService& serverSyncService,
        QObject* parent = nullptr);

private:
    void loadStateIntoView();
    void applySettings();
    void testConnection();
    void applySettingsInternal(bool notifyUser);

    views::SettingsPage& page_;
    models::AppState& appState_;
    services::ApiOrchestratorService& orchestrator_;
    services::ServerSyncService& serverSyncService_;
};

}