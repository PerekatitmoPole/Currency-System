#include "views/pages/RatesPage.hpp"

#include "views/widgets/RatesTableWidget.hpp"

#include <QComboBox>
#include <QGridLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QVBoxLayout>

namespace currency::client::views {

RatesPage::RatesPage(QWidget* parent)
    : QWidget(parent) {
    auto* rootLayout = new QVBoxLayout(this);
    rootLayout->setContentsMargins(28, 28, 28, 28);
    rootLayout->setSpacing(20);

    auto* title = new QLabel("Rates", this);
    title->setObjectName("pageTitleLabel");
    rootLayout->addWidget(title);

    auto* filtersCard = new QWidget(this);
    filtersCard->setObjectName("cardWidget");
    auto* filtersLayout = new QGridLayout(filtersCard);
    filtersLayout->setContentsMargins(20, 20, 20, 20);
    filtersLayout->setHorizontalSpacing(16);
    filtersLayout->setVerticalSpacing(12);

    baseCurrencyCombo_ = new QComboBox(filtersCard);
    quoteCurrenciesEdit_ = new QLineEdit(filtersCard);
    quoteCurrenciesEdit_->setPlaceholderText("USD, GBP, RUB, JPY");
    refreshButton_ = new QPushButton("Load rates", filtersCard);
    refreshButton_->setObjectName("primaryButton");

    filtersLayout->addWidget(new QLabel("Base currency", filtersCard), 0, 0);
    filtersLayout->addWidget(baseCurrencyCombo_, 1, 0);
    filtersLayout->addWidget(new QLabel("Quote currencies", filtersCard), 0, 1);
    filtersLayout->addWidget(quoteCurrenciesEdit_, 1, 1);
    filtersLayout->addWidget(refreshButton_, 1, 2);

    rootLayout->addWidget(filtersCard);

    ratesTableWidget_ = new RatesTableWidget(this);
    rootLayout->addWidget(ratesTableWidget_, 1);
}

QComboBox* RatesPage::baseCurrencyCombo() const { return baseCurrencyCombo_; }
QLineEdit* RatesPage::quoteCurrenciesEdit() const { return quoteCurrenciesEdit_; }
QPushButton* RatesPage::refreshButton() const { return refreshButton_; }
RatesTableWidget* RatesPage::ratesTableWidget() const { return ratesTableWidget_; }

}