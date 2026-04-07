#include "controllers/SettingsController.hpp"

#include "dto/ApiDtos.hpp"
#include "models/AppState.hpp"
#include "services/ApiOrchestratorService.hpp"
#include "services/ServerSyncService.hpp"
#include "views/pages/SettingsPage.hpp"

#include <QLineEdit>
#include <QPushButton>
#include <QSpinBox>

namespace currency::client::controllers {

namespace {

QStringList parseQuotes(const QString& rawValue) {
    QStringList result;
    for (auto token : rawValue.split(',', Qt::SkipEmptyParts)) {
        token = token.trimmed().toUpper();
        if (!token.isEmpty()) {
            result.push_back(token);
        }
    }
    return result;
}

}

SettingsController::SettingsController(
    views::SettingsPage& page,
    models::AppState& appState,
    services::ApiOrchestratorService& orchestrator,
    services::ServerSyncService& serverSyncService,
    QObject* parent)
    : QObject(parent),
      page_(page),
      appState_(appState),
      orchestrator_(orchestrator),
      serverSyncService_(serverSyncService) {
    connect(page_.applyButton(), &QPushButton::clicked, this, &SettingsController::applySettings);
    connect(page_.connectButton(), &QPushButton::clicked, this, &SettingsController::testConnection);
    loadStateIntoView();
}

void SettingsController::loadStateIntoView() {
    page_.hostEdit()->setText(appState_.serverHost());
    page_.portSpinBox()->setValue(appState_.serverPort());
    page_.defaultBaseEdit()->setText(appState_.defaultBaseCurrency());
    page_.defaultQuotesEdit()->setText(appState_.defaultQuoteCurrencies().join(", "));

    for (const auto source : dto::allApiSources()) {
        page_.apiKeyEdit(source)->setText(appState_.apiKey(source));
    }
}

void SettingsController::applySettings() {
    applySettingsInternal(true);
}

void SettingsController::testConnection() {
    applySettingsInternal(false);
    const auto result = serverSyncService_.connect(appState_.serverHost(), appState_.serverPort());
    if (!result.ok()) {
        appState_.setConnectionStatus(dto::ConnectionStatusDto{
            .host = appState_.serverHost(),
            .port = appState_.serverPort(),
            .connected = false,
            .message = result.error().message,
        });
        emit appState_.errorRaised("Connection failed", result.error().message);
        return;
    }

    appState_.setConnectionStatus(dto::ConnectionStatusDto{
        .host = appState_.serverHost(),
        .port = appState_.serverPort(),
        .connected = true,
        .message = "TCP connection to the server is healthy.",
    });
    emit appState_.infoRaised("Connection", "Server connection established successfully.");
}

void SettingsController::applySettingsInternal(const bool notifyUser) {
    appState_.setServerHost(page_.hostEdit()->text());
    appState_.setServerPort(static_cast<quint16>(page_.portSpinBox()->value()));
    appState_.setDefaultBaseCurrency(page_.defaultBaseEdit()->text());
    appState_.setDefaultQuoteCurrencies(parseQuotes(page_.defaultQuotesEdit()->text()));

    for (const auto source : dto::allApiSources()) {
        const auto value = page_.apiKeyEdit(source)->text();
        appState_.setApiKey(source, value);
        orchestrator_.setApiKey(source, value);
    }

    if (notifyUser) {
        emit appState_.infoRaised("Settings", "Client settings were applied.");
    }
}

}