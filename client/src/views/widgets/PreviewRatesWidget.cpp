#include "views/widgets/PreviewRatesWidget.hpp"

#include "common/DateTimeUtils.hpp"

#include <algorithm>
#include <QFrame>
#include <QGridLayout>
#include <QLabel>
#include <QVBoxLayout>

namespace currency::client::views {

PreviewRatesWidget::PreviewRatesWidget(QWidget* parent)
    : QWidget(parent) {
    setObjectName("cardWidget");

    auto* rootLayout = new QVBoxLayout(this);
    rootLayout->setContentsMargins(20, 20, 20, 20);
    rootLayout->setSpacing(12);

    auto* title = new QLabel("Latest preview", this);
    title->setObjectName("sectionTitleLabel");
    rootLayout->addWidget(title);

    gridLayout_ = new QGridLayout();
    gridLayout_->setHorizontalSpacing(12);
    gridLayout_->setVerticalSpacing(12);
    rootLayout->addLayout(gridLayout_);
}

void PreviewRatesWidget::setRates(const QList<models::RateViewModel>& rates) {
    QLayoutItem* child = nullptr;
    while ((child = gridLayout_->takeAt(0)) != nullptr) {
        delete child->widget();
        delete child;
    }

    const int maxCards = std::min(4, rates.size());
    for (int index = 0; index < maxCards; ++index) {
        const auto& rate = rates.at(index);
        auto* card = new QFrame(this);
        card->setObjectName("previewCard");
        auto* layout = new QVBoxLayout(card);
        layout->setContentsMargins(14, 14, 14, 14);
        layout->setSpacing(4);

        auto* pairLabel = new QLabel(rate.baseCurrency + "/" + rate.quoteCurrency, card);
        pairLabel->setObjectName("previewPairLabel");
        layout->addWidget(pairLabel);

        auto* valueLabel = new QLabel(QString::number(rate.rate, 'f', 6), card);
        valueLabel->setObjectName("previewValueLabel");
        layout->addWidget(valueLabel);

        auto* sourceLabel = new QLabel(rate.sourceName + " • " + common::DateTimeUtils::toDisplayString(rate.timestamp), card);
        sourceLabel->setObjectName("mutedLabel");
        layout->addWidget(sourceLabel);

        gridLayout_->addWidget(card, index / 2, index % 2);
    }
}

}