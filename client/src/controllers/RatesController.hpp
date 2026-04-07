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
}

namespace currency::client::views {
class RatesPage;
}

namespace currency::client::controllers {

class RatesController : public QObject {
    Q_OBJECT

public:
    RatesController(
        views::RatesPage& page,
        models::AppState& appState,
        services::ApiOrchestratorService& orchestrator,
        services::AggregationService& aggregationService,
        models::CurrencyTableModel& currencyTableModel,
        models::AggregationTableModel& aggregationTableModel,
        QObject* parent = nullptr);

private:
    void populateBaseCurrencies();
    void refreshRates();

    views::RatesPage& page_;
    models::AppState& appState_;
    services::ApiOrchestratorService& orchestrator_;
    services::AggregationService& aggregationService_;
    models::CurrencyTableModel& currencyTableModel_;
    models::AggregationTableModel& aggregationTableModel_;
};

}