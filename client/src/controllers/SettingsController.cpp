#include "controllers/SettingsController.hpp"

#include "common/BusyIndicatorGuard.hpp"
#include "common/ErrorClassifier.hpp"
#include "common/UiText.hpp"
#include "dto/ApiDtos.hpp"
#include "models/AppState.hpp"
#include "services/ApiOrchestratorService.hpp"
#include "services/ServerSyncService.hpp"
#include "views/pages/SettingsPage.hpp"

#include <QLineEdit>
#include <QMetaObject>
#include <QPushButton>
#include <QSpinBox>

namespace currency::client::controllers {

namespace {

QString connectionStateText(const bool connected) {
    return connected ? QString::fromUtf8(u8"Подключено") : QString::fromUtf8(u8"Нет подключения");
}

QString settingsErrorTitleByType(const common::Error& error, const QString& fallback) {
    if (common::isTransportError(error)) {
        return QString::fromUtf8(u8"Ошибка соединения");
    }
    if (error.details == "provider_unavailable" || error.code == "validation_error") {
        return QString::fromUtf8(u8"Ошибка источника данных");
    }
    return fallback;
}

}

SettingsController::SettingsController(
    views::SettingsPage& page,
    models::AppState& appState,
    services::ApiOrchestratorService& orchestrator,
    services::ServerSyncService& serverSyncService,
    QObject& networkContext,
    QObject* parent)
    : QObject(parent),
      page_(page),
      appState_(appState),
      orchestrator_(orchestrator),
      serverSyncService_(serverSyncService),
      networkContext_(networkContext) {
    connect(page_.applyButton(), &QPushButton::clicked, this, &SettingsController::applySettings);
    connect(page_.connectButton(), &QPushButton::clicked, this, &SettingsController::testConnection);
    connect(&appState_, &models::AppState::connectionStatusChanged, this, &SettingsController::loadStateIntoView);
    loadStateIntoView();
}

void SettingsController::loadStateIntoView() {
    page_.hostEdit()->setText(appState_.serverHost());
    page_.portSpinBox()->setValue(appState_.serverPort());
    const auto status = appState_.connectionStatus();
    page_.setConnectionOverview(
        connectionStateText(status.connected),
        QString("%1:%2").arg(status.host).arg(status.port),
        status.message,
        status.connected);
}

void SettingsController::applySettings() {
    applySettingsInternal(true);
}

void SettingsController::testConnection() {
    refreshConnectionStatus(false);
}

void SettingsController::refreshConnectionStatus(const bool silent) {
    applySettingsInternal(false);
    if (connectionCheckInFlight_) {
        return;
    }

    connectionCheckInFlight_ = true;
    if (!silent) {
        connectionGuard_ = std::make_unique<common::BusyIndicatorGuard>(page_.connectButton(), common::UiText::busyChecking());
    }

    const auto host = appState_.serverHost();
    const auto port = appState_.serverPort();
    QMetaObject::invokeMethod(
        &networkContext_,
        [this, host, port, silent] {
            const auto connectResult = serverSyncService_.connect(host, port);
            auto currencies = common::Result<QList<models::CurrencyViewModel>>::failure(
                common::Errors::internalError("Сетевая проверка не была выполнена.", {}));
            QList<QPair<dto::ApiSource, QList<models::CurrencyViewModel>>> perSource;
            if (connectResult.ok()) {
                currencies = orchestrator_.fetchCurrencies();
                if (currencies.ok()) {
                    for (const auto source : dto::allApiSources()) {
                        const auto sourceCurrencies = orchestrator_.fetchCurrencies(source);
                        if (sourceCurrencies.ok()) {
                            perSource.push_back({source, sourceCurrencies.value()});
                        }
                    }
                }
            }

            QMetaObject::invokeMethod(
                this,
                [this, silent, connectResult, currencies, perSource] {
                    connectionGuard_.reset();
                    connectionCheckInFlight_ = false;

                    if (!connectResult.ok()) {
                        appState_.markServerDisconnected(connectResult.error().message);
                        if (!silent) {
                            emit appState_.errorRaised(
                                settingsErrorTitleByType(connectResult.error(), QString::fromUtf8(u8"Ошибка подключения")),
                                connectResult.error().message);
                        }
                        return;
                    }

                    if (!currencies.ok()) {
                        if (common::isTransportError(currencies.error())) {
                            appState_.markServerDisconnected(currencies.error().message);
                        }
                        if (!silent) {
                            emit appState_.errorRaised(
                                settingsErrorTitleByType(currencies.error(), QString::fromUtf8(u8"Проверка сервера не пройдена")),
                                currencies.error().message);
                        }
                        return;
                    }

                    appState_.setAvailableCurrencies(currencies.value());
                    for (const auto& entry : perSource) {
                        appState_.setCurrenciesForSource(entry.first, entry.second);
                    }

                    appState_.markServerConnected(QString::fromUtf8(u8"Сервер доступен. Загружено валют: %1.").arg(appState_.availableCurrencies().size()));
                    if (!silent) {
                        emit appState_.infoRaised(QString::fromUtf8(u8"Подключение"), QString::fromUtf8(u8"Соединение с сервером успешно установлено."));
                    }
                },
                Qt::QueuedConnection);
        },
        Qt::QueuedConnection);
}

void SettingsController::applySettingsInternal(const bool notifyUser) {
    std::unique_ptr<common::BusyIndicatorGuard> guard;
    if (notifyUser) {
        guard = std::make_unique<common::BusyIndicatorGuard>(page_.applyButton(), common::UiText::busySaving());
    }

    appState_.setServerHost(page_.hostEdit()->text());
    appState_.setServerPort(static_cast<quint16>(page_.portSpinBox()->value()));

    if (notifyUser) {
        emit appState_.infoRaised(common::UiText::settingsPageTitle(), QString::fromUtf8(u8"Настройки подключения сохранены."));
    }
}

}