#pragma once

#include <QObject>

namespace currency::client::models {
class AppState;
class HistoryTableModel;
}

namespace currency::client::services {
class ApiOrchestratorService;
class ChartPreparationService;
}

namespace currency::client::views {
class HistoryPage;
}

namespace currency::client::controllers {

class HistoryController : public QObject {
    Q_OBJECT

public:
    HistoryController(
        views::HistoryPage& page,
        models::AppState& appState,
        services::ApiOrchestratorService& orchestrator,
        services::ChartPreparationService& chartPreparationService,
        models::HistoryTableModel& historyTableModel,
        QObject* parent = nullptr);

private:
    void populateFilters();
    void loadHistory();
    void updateHistoryView();

    views::HistoryPage& page_;
    models::AppState& appState_;
    services::ApiOrchestratorService& orchestrator_;
    services::ChartPreparationService& chartPreparationService_;
    models::HistoryTableModel& historyTableModel_;
};

}