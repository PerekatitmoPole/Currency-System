#pragma once

#include "common/BusyIndicatorGuard.hpp"

#include <QObject>
#include <memory>

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
        QObject& networkContext,
        QObject* parent = nullptr);

    void refreshConnectionStatus(bool silent = false);

private:
    void loadStateIntoView();
    void applySettings();
    void testConnection();
    void applySettingsInternal(bool notifyUser);

    views::SettingsPage& page_;
    models::AppState& appState_;
    services::ApiOrchestratorService& orchestrator_;
    services::ServerSyncService& serverSyncService_;
    QObject& networkContext_;
    std::unique_ptr<common::BusyIndicatorGuard> connectionGuard_;
    bool connectionCheckInFlight_{false};
};

}
