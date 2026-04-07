#include "views/pages/AggregationPage.hpp"

#include "views/widgets/AggregationSummaryWidget.hpp"

#include <QLabel>
#include <QPushButton>
#include <QVBoxLayout>

namespace currency::client::views {

AggregationPage::AggregationPage(QWidget* parent)
    : QWidget(parent) {
    auto* rootLayout = new QVBoxLayout(this);
    rootLayout->setContentsMargins(28, 28, 28, 28);
    rootLayout->setSpacing(20);

    auto* title = new QLabel("Aggregation", this);
    title->setObjectName("pageTitleLabel");
    rootLayout->addWidget(title);

    summaryWidget_ = new AggregationSummaryWidget(this);
    rootLayout->addWidget(summaryWidget_, 1);

    syncButton_ = new QPushButton("Sync normalized quotes to server", this);
    syncButton_->setObjectName("primaryButton");
    rootLayout->addWidget(syncButton_, 0, Qt::AlignRight);
}

AggregationSummaryWidget* AggregationPage::summaryWidget() const { return summaryWidget_; }
QPushButton* AggregationPage::syncButton() const { return syncButton_; }

}