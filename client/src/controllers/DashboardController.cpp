#include "controllers/DashboardController.hpp"

#include "common/BusyIndicatorGuard.hpp"
#include "common/ErrorClassifier.hpp"
#include "common/UiText.hpp"
#include "dto/ApiDtos.hpp"
#include "models/AggregationTableModel.hpp"
#include "models/AppState.hpp"
#include "models/CurrencyTableModel.hpp"
#include "services/AggregationService.hpp"
#include "services/ApiOrchestratorService.hpp"
#include "services/ChartPreparationService.hpp"
#include "services/ExportService.hpp"
#include "views/pages/DashboardPage.hpp"
#include "views/widgets/AggregationSummaryWidget.hpp"
#include "views/widgets/HistoryChartWidget.hpp"
#include "views/widgets/RatesTableWidget.hpp"

#include <QComboBox>
#include <QDateEdit>
#include <QDoubleSpinBox>
#include <QFileDialog>
#include <QFileInfo>
#include <QLocale>
#include <QMetaObject>
#include <QPushButton>
#include <QSignalBlocker>
#include <QSet>

#include <algorithm>

namespace currency::client::controllers {

namespace {
QString currencyCode(const QComboBox* combo) {
    const auto value = combo->currentData().toString();
    return value.isEmpty() ? combo->currentText().section(' ', 0, 0).trimmed().toUpper() : value;
}

QString currencyText(const models::CurrencyViewModel& currency) {
    const auto name = currency.name.trimmed();
    return name.isEmpty() || name == currency.code ? currency.code : QString("%1 - %2").arg(currency.code, name);
}

QString withExportExtension(QString path, const QString& selectedFilter) {
    if (QFileInfo(path).suffix().isEmpty()) {
        if (selectedFilter.contains("*.csv")) {
            path += ".csv";
        } else if (selectedFilter.contains("*.xlsx")) {
            path += ".xlsx";
        } else {
            path += ".json";
        }
    }
    return path;
}

QString withPngExtension(QString path) {
    if (QFileInfo(path).suffix().isEmpty()) {
        path += ".png";
    }
    return path;
}

QString errorTitleByType(const common::Error& error, const QString& defaultTitle) {
    if (common::isTransportError(error)) {
        return QString::fromUtf8(u8"Ошибка соединения");
    }
    if (error.details == "provider_unavailable" || error.code == "validation_error") {
        return QString::fromUtf8(u8"Ошибка источника данных");
    }
    return defaultTitle;
}

QString formatAggregationPreview(
    const dto::AggregationSummaryDto& summary,
    const QDate& from,
    const QDate& to) {
    if (summary.sampleCount <= 0) {
        return QString::fromUtf8(u8"Нет данных для агрегации по выбранному диапазону дат.");
    }

    const auto rangeText = from == to
        ? QString::fromUtf8(u8"на %1").arg(from.toString("dd.MM.yyyy"))
        : QString::fromUtf8(u8"за период %1 - %2").arg(from.toString("dd.MM.yyyy"), to.toString("dd.MM.yyyy"));
    return QString::fromUtf8(u8"Агрегация %1: %2/%3, среднее %4, минимум %5, максимум %6.")
        .arg(rangeText, summary.baseCurrency, summary.quoteCurrency)
        .arg(QString::number(summary.averageRate, 'f', 6))
        .arg(QString::number(summary.minimumRate, 'f', 6))
        .arg(QString::number(summary.maximumRate, 'f', 6));
}

QString formatDayRatePreview(
    const QDate& day,
    const QString& providerName,
    const QString& baseCurrency,
    const QString& quoteCurrency,
    const double rate,
    const double amount) {
    const QLocale locale(QLocale::Russian);
    return QString::fromUtf8(u8"%1 (%2): %3/%4 = %5, %6 %3 = %7 %4")
        .arg(day.toString("dd.MM.yyyy"), providerName, baseCurrency, quoteCurrency)
        .arg(locale.toString(rate, 'f', 6))
        .arg(locale.toString(amount, 'f', 2))
        .arg(locale.toString(amount * rate, 'f', 2));
}

QList<dto::ApiSource> buildDayRateSourceOrder(
    const dto::ApiSource selectedSource,
    const QList<dto::ApiSource>& selectedSources) {
    QList<dto::ApiSource> ordered;
    QSet<dto::ApiSource> seen;

    const auto appendIfHistory = [&ordered, &seen](const dto::ApiSource source) {
        if (!dto::supportsHistory(source) || seen.contains(source)) {
            return;
        }
        seen.insert(source);
        ordered.push_back(source);
    };

    appendIfHistory(selectedSource);
    for (const auto source : selectedSources) {
        appendIfHistory(source);
    }
    for (const auto source : dto::sourcesWithHistory()) {
        appendIfHistory(source);
    }
    return ordered;
}

QList<dto::NormalizedQuoteDto> selectSummaryQuotes(
    const QList<dto::NormalizedQuoteDto>& quotes,
    const QString& quoteCurrency) {
    const auto normalizedQuote = quoteCurrency.trimmed().toUpper();
    if (normalizedQuote.isEmpty()) {
        return quotes;
    }

    QList<dto::NormalizedQuoteDto> filtered;
    for (const auto& quote : quotes) {
        if (quote.quoteCurrency == normalizedQuote) {
            filtered.push_back(quote);
        }
    }
    return filtered.isEmpty() ? quotes : filtered;
}

QList<models::RateViewModel> toPeriodRateRows(const QList<dto::NormalizedHistoryPointDto>& points) {
    QList<models::RateViewModel> rows;
    rows.reserve(points.size());
    for (const auto& point : points) {
        rows.push_back(models::RateViewModel{
            .sourceName = point.providerName,
            .baseCurrency = point.baseCurrency,
            .quoteCurrency = point.quoteCurrency,
            .rate = point.rate,
            .timestamp = point.timestamp,
            .note = {},
        });
    }
    std::sort(rows.begin(), rows.end(), [](const auto& left, const auto& right) {
        return left.timestamp < right.timestamp;
    });
    return rows;
}

QList<dto::NormalizedQuoteDto> toPeriodQuotes(const QList<dto::NormalizedHistoryPointDto>& points) {
    QList<dto::NormalizedQuoteDto> rows;
    rows.reserve(points.size());
    for (const auto& point : points) {
        rows.push_back(dto::NormalizedQuoteDto{
            .source = point.source,
            .providerName = point.providerName,
            .baseCurrency = point.baseCurrency,
            .quoteCurrency = point.quoteCurrency,
            .baseName = point.baseCurrency,
            .quoteName = point.quoteCurrency,
            .rate = point.rate,
            .timestamp = point.timestamp,
        });
    }
    return rows;
}

}

DashboardController::DashboardController(
    views::DashboardPage& page,
    models::AppState& appState,
    services::ApiOrchestratorService& orchestrator,
    services::AggregationService& aggregationService,
    services::ChartPreparationService& chartPreparationService,
    services::ExportService& exportService,
    models::CurrencyTableModel& currencyTableModel,
    models::AggregationTableModel& aggregationTableModel,
    QObject& networkContext,
    QObject* parent)
    : QObject(parent),
      page_(page),
      appState_(appState),
      orchestrator_(orchestrator),
      aggregationService_(aggregationService),
      chartPreparationService_(chartPreparationService),
      exportService_(exportService),
      currencyTableModel_(currencyTableModel),
      aggregationTableModel_(aggregationTableModel),
      networkContext_(networkContext) {
    page_.ratesTableWidget()->setModel(&currencyTableModel_);
    page_.aggregationSummaryWidget()->setModel(&aggregationTableModel_);

    connect(page_.refreshButton(), &QPushButton::clicked, this, &DashboardController::refreshQuotes);
    connect(page_.dayRateButton(), &QPushButton::clicked, this, &DashboardController::loadDayRate);
    connect(page_.convertButton(), &QPushButton::clicked, this, &DashboardController::runConversion);
    connect(page_.loadHistoryButton(), &QPushButton::clicked, this, &DashboardController::loadHistory);
    connect(page_.swapCurrenciesButton(), &QPushButton::clicked, this, &DashboardController::swapCurrencies);
    connect(page_.exportRatesButton(), &QPushButton::clicked, this, &DashboardController::exportRates);
    connect(page_.exportHistoryButton(), &QPushButton::clicked, this, &DashboardController::exportHistory);
    connect(page_.saveChartButton(), &QPushButton::clicked, this, &DashboardController::saveChart);
    connect(page_.sourceCombo(), QOverload<int>::of(&QComboBox::currentIndexChanged), this, &DashboardController::populateCurrencyCombos);
    connect(page_.baseCurrencyCombo(), QOverload<int>::of(&QComboBox::currentIndexChanged), this, [this] {
        repopulateQuoteCurrencies(currencyCode(page_.quoteCurrencyCombo()));
    });
    connect(&appState_, &models::AppState::providerCurrenciesChanged, this, &DashboardController::populateCurrencyCombos);
    connect(&appState_, &models::AppState::selectedSourcesChanged, this, &DashboardController::populateSources);
    connect(&appState_, &models::AppState::historyChanged, &page_, [this] {
        page_.historyChartWidget()->setPoints(
            chartPreparationService_.prepareChartPoints(appState_.normalizedHistory()),
            QString::fromUtf8(u8"История %1/%2").arg(currencyCode(page_.baseCurrencyCombo()), currencyCode(page_.quoteCurrencyCombo())));
    });
    connect(&appState_, &models::AppState::aggregationChanged, &page_, [this] {
        page_.aggregationSummaryWidget()->setSummary(appState_.aggregationSummary());
    });

    page_.fromDateEdit()->setDate(appState_.historyFromDate());
    page_.toDateEdit()->setDate(appState_.historyToDate());
    const auto stepIndex = page_.stepCombo()->findText(appState_.historyStep());
    if (stepIndex >= 0) {
        page_.stepCombo()->setCurrentIndex(stepIndex);
    }

    populateSources();
    page_.aggregationSummaryWidget()->setSummary(appState_.aggregationSummary());
    page_.aggregationSummaryWidget()->setDayRatePreview(QString::fromUtf8(u8"Не запрошено"));
}

void DashboardController::populateSources() {
    const auto previous = page_.sourceCombo()->count() > 0 ? page_.sourceCombo()->currentData() : QVariant{};
    const QSignalBlocker blocker(page_.sourceCombo());
    page_.sourceCombo()->clear();
    for (const auto source : dto::sourcesWithLatest()) {
        page_.sourceCombo()->addItem(dto::toDisplayName(source), static_cast<int>(source));
    }

    const auto selected = appState_.selectedSources().value(0, dto::ApiSource::Cbr);
    const auto index = previous.isValid()
        ? page_.sourceCombo()->findData(previous)
        : page_.sourceCombo()->findData(static_cast<int>(selected));
    if (index >= 0) {
        page_.sourceCombo()->setCurrentIndex(index);
    }
    populateCurrencyCombos();
}

void DashboardController::populateCurrencyCombos() {
    if (page_.sourceCombo()->count() == 0) {
        return;
    }

    const auto source = static_cast<dto::ApiSource>(page_.sourceCombo()->currentData().toInt());
    const auto currencies = appState_.currenciesForSource(source);
    const auto currentBase = currencyCode(page_.baseCurrencyCombo());
    const auto currentQuote = currencyCode(page_.quoteCurrencyCombo());

    {
        const QSignalBlocker baseBlocker(page_.baseCurrencyCombo());
        page_.baseCurrencyCombo()->clear();
        for (const auto& currency : currencies) {
            page_.baseCurrencyCombo()->addItem(currencyText(currency), currency.code);
        }

        const auto fallbackBase = source == dto::ApiSource::Cbr ? QString("RUB") : QString("EUR");
        const auto requestedBase = currentBase.isEmpty() ? fallbackBase : currentBase;
        const auto baseIndex = page_.baseCurrencyCombo()->findData(requestedBase);
        if (baseIndex >= 0) {
            page_.baseCurrencyCombo()->setCurrentIndex(baseIndex);
        } else if (page_.baseCurrencyCombo()->count() > 0) {
            page_.baseCurrencyCombo()->setCurrentIndex(0);
        }
    }

    repopulateQuoteCurrencies(currentQuote);
    page_.loadHistoryButton()->setEnabled(dto::supportsHistory(source));
}

void DashboardController::repopulateQuoteCurrencies(const QString& preferredQuote) {
    if (page_.sourceCombo()->count() == 0) {
        return;
    }

    const auto source = static_cast<dto::ApiSource>(page_.sourceCombo()->currentData().toInt());
    const auto currencies = appState_.currenciesForSource(source);
    const auto base = currencyCode(page_.baseCurrencyCombo());

    const QSignalBlocker blocker(page_.quoteCurrencyCombo());
    page_.quoteCurrencyCombo()->clear();
    for (const auto& currency : currencies) {
        if (currency.code != base) {
            page_.quoteCurrencyCombo()->addItem(currencyText(currency), currency.code);
        }
    }

    QString requestedQuote = preferredQuote;
    if (requestedQuote.isEmpty() || requestedQuote == base) {
        requestedQuote = page_.quoteCurrencyCombo()->findData("USD") >= 0 ? QString("USD") : QString();
    }

    const auto quoteIndex = page_.quoteCurrencyCombo()->findData(requestedQuote);
    if (quoteIndex >= 0) {
        page_.quoteCurrencyCombo()->setCurrentIndex(quoteIndex);
    } else if (page_.quoteCurrencyCombo()->count() > 0) {
        page_.quoteCurrencyCombo()->setCurrentIndex(0);
    }
}

void DashboardController::refreshQuotes() {
    loadPeriodRates(false);
}

void DashboardController::loadPeriodRates(const bool updateChart) {
    if (page_.sourceCombo()->count() == 0 || page_.baseCurrencyCombo()->count() == 0 || page_.quoteCurrencyCombo()->count() == 0) {
        emit appState_.errorRaised(common::UiText::dashboardPageTitle(), common::UiText::connectionRequiredMessage());
        return;
    }

    const auto source = static_cast<dto::ApiSource>(page_.sourceCombo()->currentData().toInt());
    appState_.setSelectedSources({source});
    const auto baseCurrency = currencyCode(page_.baseCurrencyCombo());
    const auto quoteCurrency = currencyCode(page_.quoteCurrencyCombo());

    const auto from = page_.fromDateEdit()->date();
    const auto to = page_.toDateEdit()->date();
    appState_.setHistoryRange(from, to);
    appState_.setHistoryStep(page_.stepCombo()->currentText());

    if (requestInFlight_) {
        return;
    }

    requestInFlight_ = true;
    busyGuard_ = std::make_unique<common::BusyIndicatorGuard>(
        updateChart ? page_.loadHistoryButton() : page_.refreshButton(),
        updateChart ? common::UiText::busyLoading() : common::UiText::busyRefreshing());
    const auto step = page_.stepCombo()->currentText();
    QMetaObject::invokeMethod(
        &networkContext_,
        [this, source, baseCurrency, quoteCurrency, from, to, step, updateChart] {
            const auto result = orchestrator_.fetchHistory(source, baseCurrency, quoteCurrency, from, to, step);
            QMetaObject::invokeMethod(
                this,
                [this, result, baseCurrency, quoteCurrency, from, to, updateChart] {
                    busyGuard_.reset();
                    requestInFlight_ = false;

                    if (!result.ok()) {
                        if (common::isTransportError(result.error())) {
                            appState_.markServerDisconnected(result.error().message);
                        }
                        emit appState_.errorRaised(
                            errorTitleByType(
                                result.error(),
                                updateChart ? QString::fromUtf8(u8"Не удалось загрузить историю") : QString::fromUtf8(u8"Не удалось загрузить курсы")),
                            result.error().message);
                        return;
                    }

                    appState_.setNormalizedHistory(result.value().points);
                    if (updateChart) {
                        appState_.setHistoryPoints(chartPreparationService_.toHistoryViewModels(result.value().points));
                    }
                    const auto periodRateRows = toPeriodRateRows(result.value().points);
                    const auto periodQuotes = toPeriodQuotes(result.value().points);
                    const auto aggregationRows = aggregationService_.aggregate(periodQuotes);
                    const auto summaryQuotes = selectSummaryQuotes(periodQuotes, quoteCurrency);
                    currencyTableModel_.setRates(periodRateRows);
                    page_.ratesTableWidget()->setTitle(from == to
                        ? QString::fromUtf8(u8"Курсы на дату %1").arg(from.toString("dd.MM.yyyy"))
                        : QString::fromUtf8(u8"Курсы за период %1 - %2").arg(from.toString("dd.MM.yyyy"), to.toString("dd.MM.yyyy")));
                    aggregationTableModel_.setResults(aggregationRows);
                    appState_.setLatestRates(periodRateRows);
                    appState_.setAggregationResults(aggregationRows);
                    const auto summary = aggregationService_.summarize(summaryQuotes);
                    appState_.setAggregationSummary(summary);
                    page_.setDateAggregationPreview(formatAggregationPreview(summary, from, to));

                    if (!periodRateRows.isEmpty()) {
                        const auto amount = page_.amountSpinBox()->value();
                        const auto converted = amount * periodRateRows.first().rate;
                        const QLocale locale(QLocale::Russian);
                        page_.setConversionPreview(QString::fromUtf8(u8"%1 %2 = %3 %4 по курсу %5")
                            .arg(locale.toString(amount, 'f', 2), baseCurrency)
                            .arg(locale.toString(converted, 'f', 2), quoteCurrency)
                            .arg(locale.toString(periodRateRows.first().rate, 'f', 6)));
                    } else {
                        page_.setConversionPreview(QString::fromUtf8(u8"Данные по выбранному диапазону не найдены."));
                    }

                    appState_.markServerConnected(updateChart ? common::UiText::dashboardHistoryLoadedMessage() : common::UiText::dashboardRatesLoadedMessage());
                },
                Qt::QueuedConnection);
        },
        Qt::QueuedConnection);
}

void DashboardController::loadHistory() {
    if (page_.sourceCombo()->count() == 0 || page_.baseCurrencyCombo()->count() == 0 || page_.quoteCurrencyCombo()->count() == 0) {
        emit appState_.errorRaised(common::UiText::historyPageTitle(), common::UiText::connectionRequiredMessage());
        return;
    }
    const auto source = static_cast<dto::ApiSource>(page_.sourceCombo()->currentData().toInt());
    if (!dto::supportsHistory(source)) {
        emit appState_.errorRaised(common::UiText::historyPageTitle(), QString::fromUtf8(u8"Выбранный API не поддерживает историю."));
        return;
    }
    loadPeriodRates(true);
}

void DashboardController::loadDayRate() {
    if (page_.sourceCombo()->count() == 0 || page_.baseCurrencyCombo()->count() == 0 || page_.quoteCurrencyCombo()->count() == 0) {
        emit appState_.errorRaised(common::UiText::dashboardPageTitle(), common::UiText::connectionRequiredMessage());
        return;
    }

    const auto selectedSource = static_cast<dto::ApiSource>(page_.sourceCombo()->currentData().toInt());

    if (requestInFlight_) {
        return;
    }

    const auto day = page_.dayRateDateEdit()->date();
    if (day > QDate::currentDate()) {
        page_.aggregationSummaryWidget()->setDayRatePreview(
            QString::fromUtf8(u8"%1: этот день ещё не настал").arg(day.toString("dd.MM.yyyy")));
        emit appState_.infoRaised(
            QString::fromUtf8(u8"Курс на дату"),
            QString::fromUtf8(u8"Запрос на будущую дату недоступен: этот день ещё не настал."));
        return;
    }

    const auto baseCurrency = currencyCode(page_.baseCurrencyCombo());
    const auto quoteCurrency = currencyCode(page_.quoteCurrencyCombo());
    const auto amount = page_.amountSpinBox()->value();
    const auto sourcesToTry = buildDayRateSourceOrder(selectedSource, appState_.selectedSources());
    requestInFlight_ = true;
    busyGuard_ = std::make_unique<common::BusyIndicatorGuard>(page_.dayRateButton(), common::UiText::busyLoading());

    QMetaObject::invokeMethod(
        &networkContext_,
        [this, sourcesToTry, baseCurrency, quoteCurrency, day, amount] {
            common::Error lastError{};
            bool hadTransportError = false;
            QString previewText;
            bool found = false;

            for (const auto source : sourcesToTry) {
                const auto result = orchestrator_.fetchHistory(
                    source,
                    baseCurrency,
                    quoteCurrency,
                    day,
                    day,
                    "1d");
                if (!result.ok()) {
                    lastError = result.error();
                    hadTransportError = hadTransportError || common::isTransportError(result.error());
                    continue;
                }
                if (result.value().points.isEmpty()) {
                    continue;
                }

                const auto latestPoint = std::max_element(
                    result.value().points.cbegin(),
                    result.value().points.cend(),
                    [](const auto& left, const auto& right) { return left.timestamp < right.timestamp; });
                previewText = formatDayRatePreview(day, dto::toDisplayName(source), baseCurrency, quoteCurrency, latestPoint->rate, amount);
                found = true;
                break;
            }

            QMetaObject::invokeMethod(
                this,
                [this, found, previewText, lastError, hadTransportError, day] {
                    busyGuard_.reset();
                    requestInFlight_ = false;

                    if (found) {
                        page_.aggregationSummaryWidget()->setDayRatePreview(previewText);
                        appState_.markServerConnected(common::UiText::historyLoadedMessage());
                        return;
                    }

                    if (hadTransportError) {
                        appState_.markServerDisconnected(lastError.message);
                    }
                    if (!lastError.code.isEmpty() && !hadTransportError && lastError.details == "provider_unavailable") {
                        emit appState_.infoRaised(QString::fromUtf8(u8"Источник данных недоступен"), lastError.message);
                    }
                    page_.aggregationSummaryWidget()->setDayRatePreview(
                        QString::fromUtf8(u8"%1: данных нет для выбранной пары").arg(day.toString("dd.MM.yyyy")));
                },
                Qt::QueuedConnection);
        },
        Qt::QueuedConnection);
}

void DashboardController::runConversion() {
    if (page_.sourceCombo()->count() == 0 || page_.baseCurrencyCombo()->count() == 0 || page_.quoteCurrencyCombo()->count() == 0) {
        emit appState_.errorRaised(common::UiText::conversionPageTitle(), common::UiText::connectionRequiredMessage());
        return;
    }

    if (requestInFlight_) {
        return;
    }

    const auto source = static_cast<dto::ApiSource>(page_.sourceCombo()->currentData().toInt());
    const auto baseCurrency = currencyCode(page_.baseCurrencyCombo());
    const auto quoteCurrency = currencyCode(page_.quoteCurrencyCombo());
    const auto amount = page_.amountSpinBox()->value();
    requestInFlight_ = true;
    busyGuard_ = std::make_unique<common::BusyIndicatorGuard>(page_.convertButton(), common::UiText::busyConverting());

    QMetaObject::invokeMethod(
        &networkContext_,
        [this, source, baseCurrency, quoteCurrency, amount] {
            const auto result = orchestrator_.convert(source, baseCurrency, quoteCurrency, amount);
            QMetaObject::invokeMethod(
                this,
                [this, result, amount, baseCurrency, quoteCurrency] {
                    busyGuard_.reset();
                    requestInFlight_ = false;

                    if (!result.ok()) {
                        if (common::isTransportError(result.error())) {
                            appState_.markServerDisconnected(result.error().message);
                        }
                        emit appState_.errorRaised(
                            errorTitleByType(result.error(), common::UiText::conversionPageTitle()),
                            result.error().message);
                        return;
                    }

                    appState_.setConversionResult(result.value());
                    appState_.markServerConnected(common::UiText::conversionLoadedMessage());
                    if (!result.value().warning.isEmpty()) {
                        emit appState_.infoRaised(QString::fromUtf8(u8"Предупреждение конвертации"), result.value().warning);
                    }

                    const QLocale locale(QLocale::Russian);
                    page_.setConversionPreview(QString::fromUtf8(u8"%1 %2 = %3 %4 (серверный расчёт)")
                        .arg(locale.toString(amount, 'f', 2), baseCurrency)
                        .arg(locale.toString(result.value().result, 'f', 2), quoteCurrency));
                },
                Qt::QueuedConnection);
        },
        Qt::QueuedConnection);
}

void DashboardController::swapCurrencies() {
    const auto base = currencyCode(page_.baseCurrencyCombo());
    const auto quote = currencyCode(page_.quoteCurrencyCombo());
    const auto baseIndex = page_.baseCurrencyCombo()->findData(quote);
    if (baseIndex >= 0) {
        page_.baseCurrencyCombo()->setCurrentIndex(baseIndex);
        repopulateQuoteCurrencies(base);
    }
}

void DashboardController::exportRates() {
    QString selectedFilter = "JSON (*.json)";
    auto path = QFileDialog::getSaveFileName(
        &page_,
        QString::fromUtf8(u8"Экспорт курсов"),
        "rates.json",
        "JSON (*.json);;CSV (*.csv);;Excel (*.xlsx)",
        &selectedFilter);
    if (path.isEmpty()) {
        return;
    }

    path = withExportExtension(path, selectedFilter);
    const common::BusyIndicatorGuard guard(page_.exportRatesButton(), common::UiText::busyExporting());
    const auto result = exportService_.exportRates(appState_.latestRates(), path);
    if (!result.ok()) {
        emit appState_.errorRaised(QString::fromUtf8(u8"Ошибка экспорта"), result.error().message);
        return;
    }

    emit appState_.infoRaised(QString::fromUtf8(u8"Экспорт"), QString::fromUtf8(u8"Курсы сохранены в файл %1.").arg(path));
}

void DashboardController::exportHistory() {
    QString selectedFilter = "JSON (*.json)";
    auto path = QFileDialog::getSaveFileName(
        &page_,
        QString::fromUtf8(u8"Экспорт истории"),
        "history.json",
        "JSON (*.json);;CSV (*.csv);;Excel (*.xlsx)",
        &selectedFilter);
    if (path.isEmpty()) {
        return;
    }

    path = withExportExtension(path, selectedFilter);
    const common::BusyIndicatorGuard guard(page_.exportHistoryButton(), common::UiText::busyExporting());
    const auto result = exportService_.exportHistory(appState_.historyPoints(), path);
    if (!result.ok()) {
        emit appState_.errorRaised(QString::fromUtf8(u8"Ошибка экспорта"), result.error().message);
        return;
    }

    emit appState_.infoRaised(QString::fromUtf8(u8"Экспорт"), QString::fromUtf8(u8"История сохранена в файл %1.").arg(path));
}

void DashboardController::saveChart() {
    if (appState_.historyPoints().isEmpty()) {
        emit appState_.errorRaised(QString::fromUtf8(u8"Экспорт графика"), QString::fromUtf8(u8"Сначала постройте график истории."));
        return;
    }

    auto path = QFileDialog::getSaveFileName(&page_, QString::fromUtf8(u8"Сохранить график"), "history-chart.png", "PNG (*.png)");
    if (path.isEmpty()) {
        return;
    }

    path = withPngExtension(path);
    const common::BusyIndicatorGuard guard(page_.saveChartButton(), common::UiText::busySaving());
    if (!page_.historyChartWidget()->saveAsPng(path)) {
        emit appState_.errorRaised(QString::fromUtf8(u8"Экспорт графика"), QString::fromUtf8(u8"Не удалось сохранить PNG-файл графика."));
        return;
    }

    emit appState_.infoRaised(QString::fromUtf8(u8"Экспорт графика"), QString::fromUtf8(u8"График сохранён в файл %1.").arg(path));
}

}
