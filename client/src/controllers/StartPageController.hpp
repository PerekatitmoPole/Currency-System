#pragma once

#include <QObject>

namespace currency::client::app {
class MainWindow;
}

namespace currency::client::models {
class AppState;
class CurrencyTableModel;
class AggregationTableModel;
}

namespace currency::client::services {
class ApiOrchestratorService;
class AggregationService;
}

namespace currency::client::views {
class StartPage;
}

namespace currency::client::controllers {

class StartPageController : public QObject {
    Q_OBJECT

public:
    StartPageController(
        views::StartPage& page,
        app::MainWindow& mainWindow,
        models::AppState& appState,
        services::ApiOrchestratorService& orchestrator,
        services::AggregationService& aggregationService,
        models::CurrencyTableModel& currencyTableModel,
        models::AggregationTableModel& aggregationTableModel,
        QObject* parent = nullptr);

private:
    void runAggregation();

    views::StartPage& page_;
    app::MainWindow& mainWindow_;
    models::AppState& appState_;
    services::ApiOrchestratorService& orchestrator_;
    services::AggregationService& aggregationService_;
    models::CurrencyTableModel& currencyTableModel_;
    models::AggregationTableModel& aggregationTableModel_;
};

}