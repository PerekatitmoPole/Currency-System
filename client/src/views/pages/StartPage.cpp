#include "views/pages/StartPage.hpp"

#include "views/widgets/ConnectionStatusWidget.hpp"

#include <QGridLayout>
#include <QFrame>
#include <QHeaderView>
#include <QHBoxLayout>
#include <QLabel>
#include <QPlainTextEdit>
#include <QPushButton>
#include <QTableView>
#include <QVBoxLayout>

namespace currency::client::views {

namespace {

QWidget* createMetricCard(QWidget* parent, const QString& title, QLabel*& valueLabel) {
    auto* card = new QWidget(parent);
    card->setObjectName("metricCard");
    auto* layout = new QVBoxLayout(card);
    layout->setContentsMargins(14, 12, 14, 12);
    layout->setSpacing(4);

    auto* titleLabel = new QLabel(title, card);
    titleLabel->setObjectName("metricTitleLabel");
    layout->addWidget(titleLabel);

    valueLabel = new QLabel(QString::fromUtf8(u8"Нет данных"), card);
    valueLabel->setObjectName("metricValueLabel");
    valueLabel->setWordWrap(true);
    layout->addWidget(valueLabel);

    return card;
}

}

StartPage::StartPage(QWidget* parent)
    : QWidget(parent) {
    auto* rootLayout = new QVBoxLayout(this);
    rootLayout->setContentsMargins(28, 24, 28, 24);
    rootLayout->setSpacing(16);

    auto* topLayout = new QGridLayout();
    topLayout->setHorizontalSpacing(18);
    topLayout->setVerticalSpacing(12);

    auto* introCard = new QWidget(this);
    introCard->setObjectName("heroCardCompact");
    auto* introLayout = new QVBoxLayout(introCard);
    introLayout->setContentsMargins(24, 20, 24, 20);
    introLayout->setSpacing(8);

    auto* title = new QLabel(QString::fromUtf8(u8"Currency System"), introCard);
    title->setObjectName("heroTitleLabel");
    introLayout->addWidget(title);

    auto* subtitle = new QLabel(
        QString::fromUtf8(u8"Клиент-серверное приложение для получения актуальных курсов валют из публичных API, хранения истории котировок, построения диаграмм и экспорта данных в файлы. Рассчитано на параллельную работу нескольких пользователей."),
        introCard);
    subtitle->setObjectName("heroSubtitleLabel");
    subtitle->setWordWrap(true);
    introLayout->addWidget(subtitle);

    auto* actionsLayout = new QHBoxLayout();
    actionsLayout->setSpacing(10);
    dashboardButton_ = new QPushButton(QString::fromUtf8(u8"Открыть панель управления"), introCard);
    dashboardButton_->setObjectName("primaryButton");
    settingsButton_ = new QPushButton(QString::fromUtf8(u8"Настройки сервера"), introCard);
    settingsButton_->setObjectName("secondaryButton");
    actionsLayout->addWidget(dashboardButton_);
    actionsLayout->addWidget(settingsButton_);
    actionsLayout->addStretch();
    introLayout->addLayout(actionsLayout);

    topLayout->addWidget(introCard, 0, 0, 2, 2);

    connectionStatusWidget_ = new ConnectionStatusWidget(this);
    topLayout->addWidget(connectionStatusWidget_, 0, 2);

    auto* metricsCard = new QWidget(this);
    metricsCard->setObjectName("cardWidget");
    auto* metricsLayout = new QGridLayout(metricsCard);
    metricsLayout->setContentsMargins(16, 16, 16, 16);
    metricsLayout->setHorizontalSpacing(10);
    metricsLayout->setVerticalSpacing(10);
    metricsLayout->addWidget(createMetricCard(metricsCard, QString::fromUtf8(u8"Синхронизация"), lastSyncValueLabel_), 0, 0);
    metricsLayout->addWidget(createMetricCard(metricsCard, QString::fromUtf8(u8"Источники"), activeSourcesValueLabel_), 0, 1);
    metricsLayout->addWidget(createMetricCard(metricsCard, QString::fromUtf8(u8"Валюты"), currenciesValueLabel_), 0, 2);
    topLayout->addWidget(metricsCard, 1, 2);

    topLayout->setColumnStretch(0, 2);
    topLayout->setColumnStretch(1, 2);
    topLayout->setColumnStretch(2, 3);
    rootLayout->addLayout(topLayout);

    auto* contentLayout = new QHBoxLayout();
    contentLayout->setSpacing(16);

    auto* providersCard = new QWidget(this);
    providersCard->setObjectName("cardWidget");
    auto* providersLayout = new QVBoxLayout(providersCard);
    providersLayout->setContentsMargins(20, 18, 20, 18);
    providersLayout->setSpacing(10);

    auto* providersTitle = new QLabel(QString::fromUtf8(u8"Доступные API"), providersCard);
    providersTitle->setObjectName("sectionTitleLabel");
    providersLayout->addWidget(providersTitle);

    providerTableView_ = new QTableView(providersCard);
    providerTableView_->setModel(&providerCatalogModel_);
    providerTableView_->hideColumn(2);
    providerTableView_->hideColumn(3);
    providerTableView_->hideColumn(4);
    providerTableView_->horizontalHeader()->setSectionResizeMode(0, QHeaderView::ResizeToContents);
    providerTableView_->horizontalHeader()->setSectionResizeMode(1, QHeaderView::Stretch);
    providerTableView_->verticalHeader()->setVisible(false);
    providerTableView_->verticalHeader()->setDefaultSectionSize(42);
    providerTableView_->setEditTriggers(QAbstractItemView::NoEditTriggers);
    providerTableView_->setSelectionBehavior(QAbstractItemView::SelectRows);
    providerTableView_->setSelectionMode(QAbstractItemView::SingleSelection);
    providerTableView_->setAlternatingRowColors(true);
    providerTableView_->setShowGrid(false);
    providerTableView_->setMinimumHeight(330);
    providersLayout->addWidget(providerTableView_, 1);

    contentLayout->addWidget(providersCard, 3);

    auto* detailsCard = new QWidget(this);
    detailsCard->setObjectName("cardWidget");
    auto* detailsLayout = new QVBoxLayout(detailsCard);
    detailsLayout->setContentsMargins(20, 18, 20, 18);
    detailsLayout->setSpacing(12);

    providerTitleLabel_ = new QLabel(QString::fromUtf8(u8"Выберите API"), detailsCard);
    providerTitleLabel_->setObjectName("sectionTitleLabel");
    detailsLayout->addWidget(providerTitleLabel_);

    providerBaseValueLabel_ = new QLabel(QString::fromUtf8(u8"Базовая валюта: нет данных"), detailsCard);
    providerBaseValueLabel_->setObjectName("metricValueLabel");
    providerBaseValueLabel_->setWordWrap(true);
    detailsLayout->addWidget(providerBaseValueLabel_);

    auto* conversionsTitle = new QLabel(QString::fromUtf8(u8"Конвертация в"), detailsCard);
    conversionsTitle->setObjectName("metricTitleLabel");
    detailsLayout->addWidget(conversionsTitle);

    providerConversionsTextEdit_ = new QPlainTextEdit(detailsCard);
    providerConversionsTextEdit_->setObjectName("providerCurrencyList");
    providerConversionsTextEdit_->setReadOnly(true);
    providerConversionsTextEdit_->setFrameShape(QFrame::NoFrame);
    providerConversionsTextEdit_->setPlainText(QString::fromUtf8(u8"подключитесь к серверу"));
    providerConversionsTextEdit_->setMinimumHeight(250);
    detailsLayout->addWidget(providerConversionsTextEdit_, 1);

    providerStatusValueLabel_ = new QLabel(QString::fromUtf8(u8"Статус: ожидает выбора"), detailsCard);
    providerStatusValueLabel_->setObjectName("mutedLabel");
    providerStatusValueLabel_->setWordWrap(true);
    detailsLayout->addWidget(providerStatusValueLabel_);
    detailsLayout->addStretch();

    contentLayout->addWidget(detailsCard, 2);
    rootLayout->addLayout(contentLayout, 1);

    setSystemMetrics(QString::fromUtf8(u8"Нет успешной синхронизации"), 0, 0);
}

ConnectionStatusWidget* StartPage::connectionStatusWidget() const {
    return connectionStatusWidget_;
}

QTableView* StartPage::providerTableView() const {
    return providerTableView_;
}

QPushButton* StartPage::dashboardButton() const {
    return dashboardButton_;
}

QPushButton* StartPage::settingsButton() const {
    return settingsButton_;
}

void StartPage::setSystemMetrics(const QString& lastSyncText, const int activeSourceCount, const int currencyCount) {
    lastSyncValueLabel_->setText(lastSyncText);
    activeSourcesValueLabel_->setText(QString::number(activeSourceCount));
    currenciesValueLabel_->setText(QString::number(currencyCount));
}

void StartPage::setProviderDetails(
    const QString& title,
    const QString& baseCurrency,
    const QString& conversions,
    const QString& statusText) {
    providerTitleLabel_->setText(title);
    providerBaseValueLabel_->setText(QString::fromUtf8(u8"Базовая валюта: %1").arg(baseCurrency));
    providerConversionsTextEdit_->setPlainText(conversions);
    providerStatusValueLabel_->setText(QString::fromUtf8(u8"Статус: %1").arg(statusText));
}

}
