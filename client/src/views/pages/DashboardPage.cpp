#include "views/pages/DashboardPage.hpp"

#include "views/widgets/AggregationSummaryWidget.hpp"
#include "views/widgets/HistoryChartWidget.hpp"
#include "views/widgets/RatesTableWidget.hpp"

#include <QComboBox>
#include <QDate>
#include <QDateEdit>
#include <QDoubleSpinBox>
#include <QGridLayout>
#include <QLabel>
#include <QPushButton>
#include <QVBoxLayout>

namespace currency::client::views {

namespace {

void tuneCurrencyCombo(QComboBox* combo) {
    combo->setMinimumContentsLength(14);
    combo->setMaximumWidth(270);
    combo->setMaxVisibleItems(12);
    combo->setSizeAdjustPolicy(QComboBox::AdjustToMinimumContentsLengthWithIcon);
}

}

DashboardPage::DashboardPage(QWidget* parent)
    : QWidget(parent) {
    auto* rootLayout = new QVBoxLayout(this);
    rootLayout->setContentsMargins(30, 26, 30, 26);
    rootLayout->setSpacing(18);

    auto* title = new QLabel(QString::fromUtf8(u8"Панель управления"), this);
    title->setObjectName("pageTitleLabel");
    rootLayout->addWidget(title);

    auto* subtitle = new QLabel(
        QString::fromUtf8(u8"Выберите API-источник, валютную пару, период и сумму. Здесь же загружаются курсы, строится график истории и выполняется экспорт."),
        this);
    subtitle->setObjectName("pageSubtitleLabel");
    subtitle->setWordWrap(true);
    rootLayout->addWidget(subtitle);

    auto* filtersCard = new QWidget(this);
    filtersCard->setObjectName("cardWidget");
    auto* filtersLayout = new QGridLayout(filtersCard);
    filtersLayout->setContentsMargins(22, 20, 22, 20);
    filtersLayout->setHorizontalSpacing(12);
    filtersLayout->setVerticalSpacing(10);

    sourceCombo_ = new QComboBox(filtersCard);
    baseCurrencyCombo_ = new QComboBox(filtersCard);
    quoteCurrencyCombo_ = new QComboBox(filtersCard);
    tuneCurrencyCombo(baseCurrencyCombo_);
    tuneCurrencyCombo(quoteCurrencyCombo_);

    swapCurrenciesButton_ = new QPushButton(QString::fromUtf8(u8"⇄"), filtersCard);
    swapCurrenciesButton_->setObjectName("iconButton");
    swapCurrenciesButton_->setToolTip(QString::fromUtf8(u8"Поменять валюты местами"));

    amountSpinBox_ = new QDoubleSpinBox(filtersCard);
    amountSpinBox_->setRange(0.01, 1'000'000'000.0);
    amountSpinBox_->setDecimals(2);
    amountSpinBox_->setValue(1.0);
    amountSpinBox_->setMinimumWidth(104);
    amountSpinBox_->setMaximumWidth(118);

    fromDateEdit_ = new QDateEdit(QDate::currentDate().addDays(-30), filtersCard);
    toDateEdit_ = new QDateEdit(QDate::currentDate(), filtersCard);
    dayRateDateEdit_ = new QDateEdit(QDate::currentDate(), filtersCard);
    fromDateEdit_->setCalendarPopup(true);
    toDateEdit_->setCalendarPopup(true);
    dayRateDateEdit_->setCalendarPopup(true);
    fromDateEdit_->setDisplayFormat("dd.MM.yyyy");
    toDateEdit_->setDisplayFormat("dd.MM.yyyy");
    dayRateDateEdit_->setDisplayFormat("dd.MM.yyyy");
    fromDateEdit_->setMinimumWidth(178);
    toDateEdit_->setMinimumWidth(178);
    fromDateEdit_->setMaximumWidth(190);
    toDateEdit_->setMaximumWidth(190);

    stepCombo_ = new QComboBox(filtersCard);
    stepCombo_->addItems({"1d", "5d", "1m", "1y"});
    stepCombo_->setMinimumWidth(86);
    stepCombo_->setMaximumWidth(96);

    refreshButton_ = new QPushButton(QString::fromUtf8(u8"Загрузить курсы"), filtersCard);
    refreshButton_->setObjectName("primaryButton");
    dayRateButton_ = new QPushButton(QString::fromUtf8(u8"Курс на дату"), filtersCard);
    convertButton_ = new QPushButton(QString::fromUtf8(u8"Конвертировать"), filtersCard);
    loadHistoryButton_ = new QPushButton(QString::fromUtf8(u8"Построить график"), filtersCard);
    exportRatesButton_ = new QPushButton(QString::fromUtf8(u8"Экспорт курсов"), filtersCard);
    exportHistoryButton_ = new QPushButton(QString::fromUtf8(u8"Экспорт истории"), filtersCard);
    saveChartButton_ = new QPushButton(QString::fromUtf8(u8"PNG графика"), filtersCard);

    conversionPreviewLabel_ = new QLabel(QString::fromUtf8(u8"Расчёт появится после загрузки курса."), filtersCard);
    conversionPreviewLabel_->setObjectName("conversionPreviewLabel");
    conversionPreviewLabel_->setWordWrap(true);
    dateAggregationLabel_ = new QLabel(QString::fromUtf8(u8"Агрегация по выбранной дате появится после загрузки данных."), filtersCard);
    dateAggregationLabel_->setObjectName("mutedLabel");
    dateAggregationLabel_->setWordWrap(true);

    filtersLayout->addWidget(new QLabel(QString::fromUtf8(u8"API-источник"), filtersCard), 0, 0);
    filtersLayout->addWidget(new QLabel(QString::fromUtf8(u8"Базовая валюта"), filtersCard), 0, 1);
    filtersLayout->addWidget(new QLabel(QString::fromUtf8(u8"Котируемая валюта"), filtersCard), 0, 3);
    filtersLayout->addWidget(new QLabel(QString::fromUtf8(u8"Сумма"), filtersCard), 0, 4);
    filtersLayout->addWidget(new QLabel(QString::fromUtf8(u8"С"), filtersCard), 2, 0);
    filtersLayout->addWidget(new QLabel(QString::fromUtf8(u8"По"), filtersCard), 2, 1);
    filtersLayout->addWidget(new QLabel(QString::fromUtf8(u8"Шаг"), filtersCard), 2, 2);
    filtersLayout->addWidget(new QLabel(QString::fromUtf8(u8"Дата курса"), filtersCard), 2, 3);

    filtersLayout->addWidget(sourceCombo_, 1, 0);
    filtersLayout->addWidget(baseCurrencyCombo_, 1, 1);
    filtersLayout->addWidget(swapCurrenciesButton_, 1, 2);
    filtersLayout->addWidget(quoteCurrencyCombo_, 1, 3);
    filtersLayout->addWidget(amountSpinBox_, 1, 4);
    filtersLayout->addWidget(fromDateEdit_, 3, 0);
    filtersLayout->addWidget(toDateEdit_, 3, 1);
    filtersLayout->addWidget(stepCombo_, 3, 2);
    filtersLayout->addWidget(dayRateDateEdit_, 3, 3);

    filtersLayout->addWidget(refreshButton_, 3, 4);
    filtersLayout->addWidget(dayRateButton_, 3, 5);
    filtersLayout->addWidget(convertButton_, 3, 6);
    filtersLayout->addWidget(loadHistoryButton_, 3, 7);
    filtersLayout->addWidget(exportRatesButton_, 3, 8);
    filtersLayout->addWidget(exportHistoryButton_, 3, 9);
    filtersLayout->addWidget(saveChartButton_, 3, 10);
    filtersLayout->addWidget(conversionPreviewLabel_, 4, 0, 1, 11);
    filtersLayout->addWidget(dateAggregationLabel_, 5, 0, 1, 11);

    filtersLayout->setColumnStretch(0, 2);
    filtersLayout->setColumnStretch(1, 2);
    filtersLayout->setColumnStretch(2, 0);
    filtersLayout->setColumnStretch(3, 2);
    filtersLayout->setColumnStretch(4, 1);
    filtersLayout->setColumnStretch(5, 1);
    filtersLayout->setColumnStretch(6, 1);
    filtersLayout->setColumnStretch(7, 1);
    filtersLayout->setColumnStretch(8, 1);
    filtersLayout->setColumnStretch(9, 1);
    filtersLayout->setColumnStretch(10, 1);

    rootLayout->addWidget(filtersCard);

    auto* contentLayout = new QGridLayout();
    contentLayout->setHorizontalSpacing(18);
    contentLayout->setVerticalSpacing(18);

    ratesTableWidget_ = new RatesTableWidget(this);
    historyChartWidget_ = new HistoryChartWidget(this);
    aggregationSummaryWidget_ = new AggregationSummaryWidget(this);

    contentLayout->addWidget(ratesTableWidget_, 0, 0, 1, 2);
    contentLayout->addWidget(historyChartWidget_, 1, 0);
    contentLayout->addWidget(aggregationSummaryWidget_, 1, 1);
    contentLayout->setColumnStretch(0, 3);
    contentLayout->setColumnStretch(1, 2);
    contentLayout->setRowStretch(0, 2);
    contentLayout->setRowStretch(1, 3);

    rootLayout->addLayout(contentLayout, 1);
}

QComboBox* DashboardPage::sourceCombo() const { return sourceCombo_; }
QComboBox* DashboardPage::baseCurrencyCombo() const { return baseCurrencyCombo_; }
QComboBox* DashboardPage::quoteCurrencyCombo() const { return quoteCurrencyCombo_; }
QComboBox* DashboardPage::stepCombo() const { return stepCombo_; }
QDateEdit* DashboardPage::fromDateEdit() const { return fromDateEdit_; }
QDateEdit* DashboardPage::toDateEdit() const { return toDateEdit_; }
QDateEdit* DashboardPage::dayRateDateEdit() const { return dayRateDateEdit_; }
QDoubleSpinBox* DashboardPage::amountSpinBox() const { return amountSpinBox_; }
QPushButton* DashboardPage::refreshButton() const { return refreshButton_; }
QPushButton* DashboardPage::loadHistoryButton() const { return loadHistoryButton_; }
QPushButton* DashboardPage::dayRateButton() const { return dayRateButton_; }
QPushButton* DashboardPage::convertButton() const { return convertButton_; }
QPushButton* DashboardPage::swapCurrenciesButton() const { return swapCurrenciesButton_; }
QPushButton* DashboardPage::exportRatesButton() const { return exportRatesButton_; }
QPushButton* DashboardPage::exportHistoryButton() const { return exportHistoryButton_; }
QPushButton* DashboardPage::saveChartButton() const { return saveChartButton_; }
RatesTableWidget* DashboardPage::ratesTableWidget() const { return ratesTableWidget_; }
HistoryChartWidget* DashboardPage::historyChartWidget() const { return historyChartWidget_; }
AggregationSummaryWidget* DashboardPage::aggregationSummaryWidget() const { return aggregationSummaryWidget_; }

void DashboardPage::setConversionPreview(const QString& text) {
    conversionPreviewLabel_->setText(text);
}

void DashboardPage::setDateAggregationPreview(const QString& text) {
    dateAggregationLabel_->setText(text);
}

}
