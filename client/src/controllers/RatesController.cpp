#include "controllers/RatesController.hpp"

#include "models/AggregationTableModel.hpp"
#include "models/AppState.hpp"
#include "models/CurrencyTableModel.hpp"
#include "services/AggregationService.hpp"
#include "services/ApiOrchestratorService.hpp"
#include "views/pages/RatesPage.hpp"
#include "views/widgets/RatesTableWidget.hpp"

#include <QComboBox>
#include <QLineEdit>
#include <QPushButton>

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

RatesController::RatesController(
    views::RatesPage& page,
    models::AppState& appState,
    services::ApiOrchestratorService& orchestrator,
    services::AggregationService& aggregationService,
    models::CurrencyTableModel& currencyTableModel,
    models::AggregationTableModel& aggregationTableModel,
    QObject* parent)
    : QObject(parent),
      page_(page),
      appState_(appState),
      orchestrator_(orchestrator),
      aggregationService_(aggregationService),
      currencyTableModel_(currencyTableModel),
      aggregationTableModel_(aggregationTableModel) {
    Q_UNUSED(aggregationTableModel_)
    page_.ratesTableWidget()->setModel(&currencyTableModel_);
    page_.quoteCurrenciesEdit()->setText(appState_.defaultQuoteCurrencies().join(", "));
    connect(page_.refreshButton(), &QPushButton::clicked, this, &RatesController::refreshRates);
    connect(&appState_, &models::AppState::availableCurrenciesChanged, this, &RatesController::populateBaseCurrencies);
    populateBaseCurrencies();
}

void RatesController::populateBaseCurrencies() {
    const auto current = page_.baseCurrencyCombo()->currentText();
    page_.baseCurrencyCombo()->clear();
    for (const auto& currency : appState_.availableCurrencies()) {
        page_.baseCurrencyCombo()->addItem(currency.code);
    }
    const auto index = page_.baseCurrencyCombo()->findText(current.isEmpty() ? appState_.defaultBaseCurrency() : current);
    if (index >= 0) {
        page_.baseCurrencyCombo()->setCurrentIndex(index);
    }
}

void RatesController::refreshRates() {
    const auto quotes = parseQuotes(page_.quoteCurrenciesEdit()->text());
    const auto result = orchestrator_.fetchLatest(appState_.selectedSources(), page_.baseCurrencyCombo()->currentText(), quotes);
    if (!result.ok()) {
        emit appState_.errorRaised("Rates", result.error().message);
        return;
    }

    appState_.setDefaultBaseCurrency(page_.baseCurrencyCombo()->currentText());
    appState_.setDefaultQuoteCurrencies(quotes);
    appState_.setNormalizedQuotes(result.value().quotes);
    const auto rateRows = aggregationService_.toRateViewModels(result.value().quotes);
    currencyTableModel_.setRates(rateRows);
    appState_.setLatestRates(rateRows);
}

}