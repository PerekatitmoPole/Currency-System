#include "views/widgets/AggregationSummaryWidget.hpp"

#include "common/DateTimeUtils.hpp"

#include <QAbstractItemModel>
#include <QGridLayout>
#include <QHeaderView>
#include <QLabel>
#include <QTableView>
#include <QVBoxLayout>

namespace currency::client::views {

namespace {

QWidget* createMetricCard(
    QWidget* parent,
    const QString& title,
    QLabel*& valueLabel) {
    auto* card = new QWidget(parent);
    card->setObjectName("metricCard");
    auto* layout = new QVBoxLayout(card);
    layout->setContentsMargins(14, 14, 14, 14);
    layout->setSpacing(6);

    auto* titleLabel = new QLabel(title, card);
    titleLabel->setObjectName("metricTitleLabel");
    layout->addWidget(titleLabel);

    valueLabel = new QLabel("Нет данных", card);
    valueLabel->setObjectName("metricValueLabel");
    valueLabel->setWordWrap(true);
    layout->addWidget(valueLabel);

    return card;
}

}

AggregationSummaryWidget::AggregationSummaryWidget(QWidget* parent)
    : QWidget(parent) {
    setObjectName("cardWidget");

    auto* layout = new QVBoxLayout(this);
    layout->setContentsMargins(20, 20, 20, 20);
    layout->setSpacing(14);

    auto* title = new QLabel("Сводка по агрегации", this);
    title->setObjectName("sectionTitleLabel");
    layout->addWidget(title);

    headlineLabel_ = new QLabel("Сводка пока не сформирована", this);
    headlineLabel_->setObjectName("summaryHeadlineLabel");
    layout->addWidget(headlineLabel_);

    detailsLabel_ = new QLabel("Загрузите котировки, чтобы сравнить значения между провайдерами.", this);
    detailsLabel_->setObjectName("mutedLabel");
    detailsLabel_->setWordWrap(true);
    layout->addWidget(detailsLabel_);

    auto* metricsLayout = new QGridLayout();
    metricsLayout->setHorizontalSpacing(12);
    metricsLayout->setVerticalSpacing(12);
    metricsLayout->addWidget(createMetricCard(this, "Источники", sourcesMetricValue_), 0, 0);
    metricsLayout->addWidget(createMetricCard(this, "Наблюдения", samplesMetricValue_), 0, 1);
    metricsLayout->addWidget(createMetricCard(this, "Последнее обновление", latestMetricValue_), 1, 0);
    metricsLayout->addWidget(createMetricCard(this, "Лучшая цена", bestMetricValue_), 1, 1);
    metricsLayout->addWidget(createMetricCard(this, "Курс на дату", dayRateMetricValue_), 2, 0, 1, 2);
    layout->addLayout(metricsLayout);
    dayRateMetricValue_->setText("Не запрошено");

    emptyLabel_ = new QLabel("Таблица агрегации пока пуста. После загрузки котировок здесь появится сравнение провайдеров по валютным парам.", this);
    emptyLabel_->setObjectName("mutedLabel");
    emptyLabel_->setWordWrap(true);
    layout->addWidget(emptyLabel_);

    tableView_ = new QTableView(this);
    tableView_->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    tableView_->horizontalHeader()->setStretchLastSection(true);
    tableView_->horizontalHeader()->setMinimumSectionSize(110);
    tableView_->verticalHeader()->setVisible(false);
    tableView_->verticalHeader()->setDefaultSectionSize(40);
    tableView_->setEditTriggers(QAbstractItemView::NoEditTriggers);
    tableView_->setSelectionBehavior(QAbstractItemView::SelectRows);
    tableView_->setAlternatingRowColors(true);
    tableView_->setSortingEnabled(true);
    tableView_->setWordWrap(false);
    tableView_->setShowGrid(false);
    tableView_->setMinimumHeight(320);
    layout->addWidget(tableView_);
}

void AggregationSummaryWidget::setSummary(const dto::AggregationSummaryDto& summary) {
    if (summary.sampleCount <= 0) {
        headlineLabel_->setText("Сводка пока не сформирована");
        detailsLabel_->setText("Загрузите котировки, чтобы сравнить значения между провайдерами.");
        sourcesMetricValue_->setText("0");
        samplesMetricValue_->setText("0");
        latestMetricValue_->setText("Нет данных");
        bestMetricValue_->setText("Нет данных");
        return;
    }

    headlineLabel_->setText(QString("%1/%2: среднее значение %3")
        .arg(summary.baseCurrency, summary.quoteCurrency, QString::number(summary.averageRate, 'f', 6)));
    detailsLabel_->setText(QString("Минимум: %1 | Максимум: %2")
        .arg(QString::number(summary.minimumRate, 'f', 6))
        .arg(QString::number(summary.maximumRate, 'f', 6)));
    sourcesMetricValue_->setText(QString::number(summary.providerCount));
    samplesMetricValue_->setText(QString::number(summary.sampleCount));
    latestMetricValue_->setText(common::DateTimeUtils::toDisplayString(summary.latestTimestamp));
    bestMetricValue_->setText(QString::number(summary.bestRate, 'f', 6));
}

void AggregationSummaryWidget::setDayRatePreview(const QString& text) {
    dayRateMetricValue_->setText(text.trimmed().isEmpty() ? QString("Не запрошено") : text);
}

void AggregationSummaryWidget::setModel(QAbstractItemModel* model) {
    model_ = model;
    tableView_->setModel(model_);
    if (model_ != nullptr) {
        connect(model_, &QAbstractItemModel::modelReset, this, &AggregationSummaryWidget::refreshEmptyState);
        connect(model_, &QAbstractItemModel::rowsInserted, this, &AggregationSummaryWidget::refreshEmptyState);
        connect(model_, &QAbstractItemModel::rowsRemoved, this, &AggregationSummaryWidget::refreshEmptyState);
    }
    refreshEmptyState();
}

void AggregationSummaryWidget::refreshEmptyState() {
    const auto hasRows = model_ != nullptr && model_->rowCount() > 0;
    emptyLabel_->setVisible(!hasRows);
    tableView_->setVisible(hasRows);
}

}
