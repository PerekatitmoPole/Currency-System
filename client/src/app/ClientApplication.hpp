#pragma once

#include "app/MainWindow.hpp"
#include "controllers/AggregationController.hpp"
#include "controllers/DashboardController.hpp"
#include "controllers/HistoryController.hpp"
#include "controllers/NavigationController.hpp"
#include "controllers/RatesController.hpp"
#include "controllers/SettingsController.hpp"
#include "controllers/StartPageController.hpp"
#include "gateways/AlphaVantageClient.hpp"
#include "gateways/CbrClient.hpp"
#include "gateways/CurrencyApiClient.hpp"
#include "gateways/CurrencyFreaksClient.hpp"
#include "gateways/EcbClient.hpp"
#include "gateways/ExchangeRateHostClient.hpp"
#include "gateways/FixerClient.hpp"
#include "gateways/FrankfurterClient.hpp"
#include "gateways/OpenExchangeRatesClient.hpp"
#include "gateways/ServerGateway.hpp"
#include "models/AggregationTableModel.hpp"
#include "models/AppState.hpp"
#include "models/CurrencyTableModel.hpp"
#include "models/HistoryTableModel.hpp"
#include "serialization/CbrMapper.hpp"
#include "serialization/CsvPayloadParser.hpp"
#include "serialization/EcbMapper.hpp"
#include "serialization/FrankfurterMapper.hpp"
#include "serialization/JsonPayloadParser.hpp"
#include "serialization/TextProtocolCodec.hpp"
#include "serialization/XmlPayloadParser.hpp"
#include "services/AggregationService.hpp"
#include "services/ApiOrchestratorService.hpp"
#include "services/ChartPreparationService.hpp"
#include "services/NormalizationService.hpp"
#include "services/ServerSyncService.hpp"

#include <QApplication>
#include <QNetworkAccessManager>

namespace currency::client::app {

class ClientApplication {
public:
    ClientApplication(QApplication& application);

    int run();

private:
    void seedInitialState();
    void wireMessages();

    QApplication& application_;
    QNetworkAccessManager networkManager_;
    serialization::JsonPayloadParser jsonPayloadParser_;
    serialization::XmlPayloadParser xmlPayloadParser_;
    serialization::CsvPayloadParser csvPayloadParser_;
    serialization::TextProtocolCodec textProtocolCodec_;
    serialization::FrankfurterMapper frankfurterMapper_;
    serialization::EcbMapper ecbMapper_;
    serialization::CbrMapper cbrMapper_;
    gateways::FrankfurterClient frankfurterClient_;
    gateways::EcbClient ecbClient_;
    gateways::CurrencyApiClient currencyApiClient_;
    gateways::OpenExchangeRatesClient openExchangeRatesClient_;
    gateways::ExchangeRateHostClient exchangeRateHostClient_;
    gateways::FixerClient fixerClient_;
    gateways::CurrencyFreaksClient currencyFreaksClient_;
    gateways::AlphaVantageClient alphaVantageClient_;
    gateways::CbrClient cbrClient_;
    gateways::ServerGateway serverGateway_;
    models::AppState appState_;
    models::CurrencyTableModel currencyTableModel_;
    models::HistoryTableModel historyTableModel_;
    models::AggregationTableModel aggregationTableModel_;
    services::NormalizationService normalizationService_;
    services::AggregationService aggregationService_;
    services::ChartPreparationService chartPreparationService_;
    services::ApiOrchestratorService orchestratorService_;
    services::ServerSyncService serverSyncService_;
    MainWindow mainWindow_;
    controllers::NavigationController navigationController_;
    controllers::StartPageController startPageController_;
    controllers::DashboardController dashboardController_;
    controllers::RatesController ratesController_;
    controllers::HistoryController historyController_;
    controllers::AggregationController aggregationController_;
    controllers::SettingsController settingsController_;
};

}