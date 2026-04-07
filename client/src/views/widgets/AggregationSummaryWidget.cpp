#include "views/widgets/AggregationSummaryWidget.hpp"

#include <QHeaderView>
#include <QLabel>
#include <QTableView>
#include <QVBoxLayout>

namespace currency::client::views {

AggregationSummaryWidget::AggregationSummaryWidget(QWidget* parent)
    : QWidget(parent) {
    setObjectName("cardWidget");

    auto* layout = new QVBoxLayout(this);
    layout->setContentsMargins(20, 20, 20, 20);
    layout->setSpacing(10);

    auto* title = new QLabel("Aggregation summary", this);
    title->setObjectName("sectionTitleLabel");
    layout->addWidget(title);

    headlineLabel_ = new QLabel("No aggregation yet", this);
    headlineLabel_->setObjectName("summaryHeadlineLabel");
    layout->addWidget(headlineLabel_);

    detailsLabel_ = new QLabel("Run aggregation to compare providers.", this);
    detailsLabel_->setObjectName("mutedLabel");
    detailsLabel_->setWordWrap(true);
    layout->addWidget(detailsLabel_);

    tableView_ = new QTableView(this);
    tableView_->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    tableView_->horizontalHeader()->setStretchLastSection(true);
    tableView_->setEditTriggers(QAbstractItemView::NoEditTriggers);
    tableView_->setSelectionBehavior(QAbstractItemView::SelectRows);
    layout->addWidget(tableView_);
}

void AggregationSummaryWidget::setSummary(const dto::AggregationSummaryDto& summary) {
    if (summary.sampleCount <= 0) {
        headlineLabel_->setText("No aggregation yet");
        detailsLabel_->setText("Run aggregation to compare providers.");
        return;
    }

    headlineLabel_->setText(QString("%1/%2 average: %3")
        .arg(summary.baseCurrency, summary.quoteCurrency, QString::number(summary.averageRate, 'f', 6)));
    detailsLabel_->setText(QString("Samples: %1 | Min: %2 | Max: %3")
        .arg(summary.sampleCount)
        .arg(QString::number(summary.minimumRate, 'f', 6))
        .arg(QString::number(summary.maximumRate, 'f', 6)));
}

void AggregationSummaryWidget::setModel(QAbstractItemModel* model) {
    tableView_->setModel(model);
}

}