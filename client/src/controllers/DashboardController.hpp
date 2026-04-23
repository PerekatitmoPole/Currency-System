#pragma once

#include "common/BusyIndicatorGuard.hpp"

#include <QObject>
#include <memory>

class QString;

namespace currency::client::models {
class AppState;
class CurrencyTableModel;
class AggregationTableModel;
}

namespace currency::client::services {
class ApiOrchestratorService;
class AggregationService;
class ChartPreparationService;
class ExportService;
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
        services::ExportService& exportService,
        models::CurrencyTableModel& currencyTableModel,
        models::AggregationTableModel& aggregationTableModel,
        QObject& networkContext,
        QObject* parent = nullptr);

private:
    void populateSources();
    void populateCurrencyCombos();
    void repopulateQuoteCurrencies(const QString& preferredQuote = {});
    void refreshQuotes();
    void loadHistory();
    void loadPeriodRates(bool updateChart);
    void loadDayRate();
    void runConversion();
    void swapCurrencies();
    void exportRates();
    void exportHistory();
    void saveChart();

    views::DashboardPage& page_;
    models::AppState& appState_;
    services::ApiOrchestratorService& orchestrator_;
    services::AggregationService& aggregationService_;
    services::ChartPreparationService& chartPreparationService_;
    services::ExportService& exportService_;
    models::CurrencyTableModel& currencyTableModel_;
    models::AggregationTableModel& aggregationTableModel_;
    QObject& networkContext_;
    std::unique_ptr<common::BusyIndicatorGuard> busyGuard_;
    bool requestInFlight_{false};
};

}
