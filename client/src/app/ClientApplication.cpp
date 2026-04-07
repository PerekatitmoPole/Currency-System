#include "app/ClientApplication.hpp"

#include "common/ThemeManager.hpp"
#include "dto/ApiDtos.hpp"

#include <QMessageBox>

namespace currency::client::app {

ClientApplication::ClientApplication(QApplication& application)
    : application_(application),
      frankfurterMapper_(jsonPayloadParser_),
      ecbMapper_(xmlPayloadParser_),
      cbrMapper_(xmlPayloadParser_),
      frankfurterClient_(networkManager_, frankfurterMapper_),
      ecbClient_(networkManager_, ecbMapper_),
      currencyApiClient_(networkManager_),
      openExchangeRatesClient_(networkManager_),
      exchangeRateHostClient_(networkManager_),
      fixerClient_(networkManager_),
      currencyFreaksClient_(networkManager_),
      alphaVantageClient_(networkManager_),
      cbrClient_(networkManager_, cbrMapper_),
      serverGateway_(textProtocolCodec_),
      orchestratorService_(normalizationService_),
      serverSyncService_(serverGateway_),
      navigationController_(*mainWindow_.burgerMenu(), mainWindow_),
      startPageController_(*mainWindow_.startPage(), mainWindow_, appState_, orchestratorService_, aggregationService_, currencyTableModel_, aggregationTableModel_),
      dashboardController_(*mainWindow_.dashboardPage(), appState_, orchestratorService_, aggregationService_, chartPreparationService_, serverSyncService_, currencyTableModel_, aggregationTableModel_),
      ratesController_(*mainWindow_.ratesPage(), appState_, orchestratorService_, aggregationService_, currencyTableModel_, aggregationTableModel_),
      historyController_(*mainWindow_.historyPage(), appState_, orchestratorService_, chartPreparationService_, historyTableModel_),
      aggregationController_(*mainWindow_.aggregationPage(), *mainWindow_.dashboardPage(), appState_, serverSyncService_, aggregationTableModel_),
      settingsController_(*mainWindow_.settingsPage(), appState_, orchestratorService_, serverSyncService_) {
    common::ThemeManager::apply(application_);

    orchestratorService_.registerClient(frankfurterClient_);
    orchestratorService_.registerClient(ecbClient_);
    orchestratorService_.registerClient(currencyApiClient_);
    orchestratorService_.registerClient(openExchangeRatesClient_);
    orchestratorService_.registerClient(exchangeRateHostClient_);
    orchestratorService_.registerClient(fixerClient_);
    orchestratorService_.registerClient(currencyFreaksClient_);
    orchestratorService_.registerClient(alphaVantageClient_);
    orchestratorService_.registerClient(cbrClient_);

    seedInitialState();
    wireMessages();
}

int ClientApplication::run() {
    mainWindow_.show();
    return application_.exec();
}

void ClientApplication::seedInitialState() {
    appState_.setSelectedSources({dto::ApiSource::Frankfurter, dto::ApiSource::Ecb, dto::ApiSource::Cbr});
    appState_.setAvailableCurrencies({
        {.code = "USD", .name = "US Dollar", .minorUnits = 2},
        {.code = "EUR", .name = "Euro", .minorUnits = 2},
        {.code = "GBP", .name = "Pound Sterling", .minorUnits = 2},
        {.code = "RUB", .name = "Russian Ruble", .minorUnits = 2},
        {.code = "JPY", .name = "Japanese Yen", .minorUnits = 0},
        {.code = "CNY", .name = "Chinese Yuan", .minorUnits = 2},
        {.code = "CHF", .name = "Swiss Franc", .minorUnits = 2},
    });

    appState_.setConnectionStatus(dto::ConnectionStatusDto{
        .host = appState_.serverHost(),
        .port = appState_.serverPort(),
        .connected = false,
        .message = "Use Settings to establish a TCP connection to the existing server.",
    });
}

void ClientApplication::wireMessages() {
    QObject::connect(&appState_, &models::AppState::errorRaised, &mainWindow_, [this](const QString& title, const QString& message) {
        QMessageBox::critical(&mainWindow_, title, message);
    });
    QObject::connect(&appState_, &models::AppState::infoRaised, &mainWindow_, [this](const QString& title, const QString& message) {
        QMessageBox::information(&mainWindow_, title, message);
    });
}

}