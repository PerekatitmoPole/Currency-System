#include "app/ClientApplication.hpp"

#include "common/ThemeManager.hpp"
#include "dto/ApiDtos.hpp"

#include <QMessageBox>
#include <QMetaObject>

namespace currency::client::app {

ClientApplication::ClientApplication(QApplication& application)
    : application_(application),
      serverGateway_(textProtocolCodec_),
      orchestratorService_(serverGateway_),
      serverSyncService_(serverGateway_),
      navigationController_(*mainWindow_.burgerMenu(), mainWindow_),
      startPageController_(*mainWindow_.startPage(), mainWindow_, appState_),
      dashboardController_(*mainWindow_.dashboardPage(), appState_, orchestratorService_, aggregationService_, chartPreparationService_, exportService_, currencyTableModel_, aggregationTableModel_, serverGateway_),
      settingsController_(*mainWindow_.settingsPage(), appState_, orchestratorService_, serverSyncService_, serverGateway_) {
    networkThread_.setObjectName("client-network-thread");
    serverGateway_.moveToThread(&networkThread_);
    networkThread_.start();

    common::ThemeManager::apply(application_);
    seedInitialState();
    wireMessages();
    settingsController_.refreshConnectionStatus(true);
}

ClientApplication::~ClientApplication() {
    if (networkThread_.isRunning()) {
        QMetaObject::invokeMethod(&serverGateway_, [this] {
            serverGateway_.disconnectFromServer();
        }, Qt::BlockingQueuedConnection);
        networkThread_.quit();
        networkThread_.wait();
    }
}

int ClientApplication::run() {
    mainWindow_.show();
    return application_.exec();
}

void ClientApplication::seedInitialState() {
    appState_.setSelectedSources({dto::ApiSource::Cbr});
    appState_.markServerDisconnected("Запустите сервер или откройте настройки, чтобы проверить подключение.");
    mainWindow_.setSelectedSources(appState_.selectedSources());
    mainWindow_.setConnectionStatus(appState_.connectionStatus());
}

void ClientApplication::wireMessages() {
    QObject::connect(&appState_, &models::AppState::connectionStatusChanged, &mainWindow_, [this] {
        mainWindow_.setConnectionStatus(appState_.connectionStatus());
    });
    QObject::connect(&appState_, &models::AppState::selectedSourcesChanged, &mainWindow_, [this] {
        mainWindow_.setSelectedSources(appState_.selectedSources());
    });
    QObject::connect(&appState_, &models::AppState::errorRaised, &mainWindow_, [this](const QString& title, const QString& message) {
        mainWindow_.showFeedbackMessage(title, message, true);
        QMessageBox::critical(&mainWindow_, title, message);
    });
    QObject::connect(&appState_, &models::AppState::infoRaised, &mainWindow_, [this](const QString& title, const QString& message) {
        mainWindow_.showFeedbackMessage(title, message, false);
    });
}

}
