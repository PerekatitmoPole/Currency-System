#include "controllers/StartPageController.hpp"

#include "app/MainWindow.hpp"
#include "common/DateTimeUtils.hpp"
#include "dto/ApiDtos.hpp"
#include "models/AppState.hpp"
#include "models/ProviderCatalogTableModel.hpp"
#include "views/pages/StartPage.hpp"
#include "views/widgets/ConnectionStatusWidget.hpp"

#include <QItemSelectionModel>
#include <QPushButton>
#include <QTableView>

#include <algorithm>

namespace currency::client::controllers {

namespace {

QString formatCurrency(const models::CurrencyViewModel& currency) {
    const auto name = currency.name.trimmed();
    return name.isEmpty() || name == currency.code ? currency.code : QString("%1 - %2").arg(currency.code, name);
}

QString defaultBaseFor(const dto::ApiSource source, const QList<models::CurrencyViewModel>& currencies) {
    const auto preferred = source == dto::ApiSource::Cbr ? QString("RUB") : QString("EUR");
    for (const auto& currency : currencies) {
        if (currency.code == preferred) {
            return formatCurrency(currency);
        }
    }
    return currencies.isEmpty() ? QString::fromUtf8(u8"нет данных") : formatCurrency(currencies.first());
}

QString conversionsFor(const QList<models::CurrencyViewModel>& currencies, const QString& baseCode) {
    QStringList values;
    for (const auto& currency : currencies) {
        if (currency.code != baseCode) {
            values.push_back(formatCurrency(currency) + ",");
        }
    }
    return values.isEmpty() ? QString::fromUtf8(u8"нет данных") : values.join("\n");
}

}

StartPageController::StartPageController(
    views::StartPage& page,
    app::MainWindow& mainWindow,
    models::AppState& appState,
    QObject* parent)
    : QObject(parent),
      page_(page),
      mainWindow_(mainWindow),
      appState_(appState) {
    connect(page_.dashboardButton(), &QPushButton::clicked, &mainWindow_, [this] {
        mainWindow_.showPage(views::BurgerMenuWidget::Page::Dashboard);
    });
    connect(page_.settingsButton(), &QPushButton::clicked, &mainWindow_, [this] {
        mainWindow_.showPage(views::BurgerMenuWidget::Page::Settings);
    });
    connect(page_.providerTableView()->selectionModel(), &QItemSelectionModel::currentRowChanged, this, [this] {
        updateSelectedProviderDetails();
    });
    connect(&appState_, &models::AppState::selectedSourcesChanged, this, &StartPageController::updateOverviewMetrics);
    connect(&appState_, &models::AppState::latestRatesChanged, this, &StartPageController::updateOverviewMetrics);
    connect(&appState_, &models::AppState::connectionStatusChanged, &page_, [this] {
        page_.connectionStatusWidget()->setStatus(appState_.connectionStatus());
    });
    connect(&appState_, &models::AppState::availableCurrenciesChanged, this, &StartPageController::updateOverviewMetrics);
    connect(&appState_, &models::AppState::providerCurrenciesChanged, this, &StartPageController::updateSelectedProviderDetails);

    page_.connectionStatusWidget()->setStatus(appState_.connectionStatus());
    page_.providerTableView()->selectRow(0);
    updateSelectedProviderDetails();
    updateOverviewMetrics();
}

void StartPageController::updateSelectedProviderDetails() {
    auto* tableModel = qobject_cast<models::ProviderCatalogTableModel*>(page_.providerTableView()->model());
    if (tableModel == nullptr) {
        return;
    }

    const auto current = page_.providerTableView()->currentIndex();
    const auto row = current.isValid() ? current.row() : 0;
    const auto descriptor = tableModel->descriptorAt(row);
    const auto currencies = appState_.currenciesForSource(descriptor.source);

    if (descriptor.availableViaServer) {
        appState_.setSelectedSources({descriptor.source});
    }

    const auto baseCode = descriptor.source == dto::ApiSource::Cbr ? QString("RUB") : QString("EUR");
    const auto status = descriptor.availableViaServer
        ? QString::fromUtf8(u8"доступен через сервер")
        : (descriptor.requiresApiKey ? QString::fromUtf8(u8"требуется API-ключ на сервере") : QString::fromUtf8(u8"ожидает серверную интеграцию"));

    page_.setProviderDetails(
        descriptor.displayName,
        defaultBaseFor(descriptor.source, currencies),
        descriptor.availableViaServer ? conversionsFor(currencies, baseCode) : QString::fromUtf8(u8"недоступно до подключения источника"),
        status);
}

void StartPageController::updateOverviewMetrics() {
    auto lastSyncText = QString::fromUtf8(u8"Нет успешной синхронизации");
    const auto rates = appState_.latestRates();
    const auto latestRate = std::max_element(rates.cbegin(), rates.cend(), [](const auto& left, const auto& right) {
        return left.timestamp < right.timestamp;
    });
    if (latestRate != rates.cend()) {
        lastSyncText = common::DateTimeUtils::toDisplayString(latestRate->timestamp);
    }

    page_.setSystemMetrics(lastSyncText, appState_.selectedSources().size(), appState_.availableCurrencies().size());
}

}
