#include "views/pages/StartPage.hpp"

#include "views/widgets/ApiSelectorWidget.hpp"
#include "views/widgets/ConnectionStatusWidget.hpp"
#include "views/widgets/PreviewRatesWidget.hpp"

#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QVBoxLayout>

namespace currency::client::views {

StartPage::StartPage(QWidget* parent)
    : QWidget(parent) {
    auto* rootLayout = new QVBoxLayout(this);
    rootLayout->setContentsMargins(28, 28, 28, 28);
    rootLayout->setSpacing(20);

    auto* heroCard = new QWidget(this);
    heroCard->setObjectName("heroCard");
    auto* heroLayout = new QVBoxLayout(heroCard);
    heroLayout->setContentsMargins(28, 28, 28, 28);
    heroLayout->setSpacing(10);

    auto* headline = new QLabel("Currency System", heroCard);
    headline->setObjectName("heroTitleLabel");
    heroLayout->addWidget(headline);

    auto* subtitle = new QLabel(
        "Aggregate currency quotes from several providers, compare them visually and sync normalized data to the server.",
        heroCard);
    subtitle->setObjectName("heroSubtitleLabel");
    subtitle->setWordWrap(true);
    heroLayout->addWidget(subtitle);

    aggregateButton_ = new QPushButton("Start aggregation", heroCard);
    aggregateButton_->setObjectName("primaryButton");
    heroLayout->addWidget(aggregateButton_, 0, Qt::AlignLeft);

    rootLayout->addWidget(heroCard);

    auto* contentLayout = new QHBoxLayout();
    contentLayout->setSpacing(20);

    apiSelector_ = new ApiSelectorWidget(this);
    contentLayout->addWidget(apiSelector_, 2);

    auto* rightColumn = new QVBoxLayout();
    rightColumn->setSpacing(20);
    connectionStatusWidget_ = new ConnectionStatusWidget(this);
    previewRatesWidget_ = new PreviewRatesWidget(this);
    rightColumn->addWidget(connectionStatusWidget_);
    rightColumn->addWidget(previewRatesWidget_);
    rightColumn->addStretch();

    contentLayout->addLayout(rightColumn, 1);
    rootLayout->addLayout(contentLayout, 1);
}

ApiSelectorWidget* StartPage::apiSelector() const {
    return apiSelector_;
}

ConnectionStatusWidget* StartPage::connectionStatusWidget() const {
    return connectionStatusWidget_;
}

PreviewRatesWidget* StartPage::previewRatesWidget() const {
    return previewRatesWidget_;
}

QPushButton* StartPage::aggregateButton() const {
    return aggregateButton_;
}

}