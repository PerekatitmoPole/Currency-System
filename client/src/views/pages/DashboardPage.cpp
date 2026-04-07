#include "views/pages/DashboardPage.hpp"

#include "views/widgets/AggregationSummaryWidget.hpp"
#include "views/widgets/HistoryChartWidget.hpp"
#include "views/widgets/RatesTableWidget.hpp"

#include <QComboBox>
#include <QGridLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QVBoxLayout>

namespace currency::client::views {

DashboardPage::DashboardPage(QWidget* parent)
    : QWidget(parent) {
    auto* rootLayout = new QVBoxLayout(this);
    rootLayout->setContentsMargins(28, 28, 28, 28);
    rootLayout->setSpacing(20);

    auto* title = new QLabel("Dashboard", this);
    title->setObjectName("pageTitleLabel");
    rootLayout->addWidget(title);

    auto* filtersCard = new QWidget(this);
    filtersCard->setObjectName("cardWidget");
    auto* filtersLayout = new QGridLayout(filtersCard);
    filtersLayout->setContentsMargins(20, 20, 20, 20);
    filtersLayout->setHorizontalSpacing(16);
    filtersLayout->setVerticalSpacing(12);

    baseCurrencyCombo_ = new QComboBox(filtersCard);
    quoteCurrencyCombo_ = new QComboBox(filtersCard);
    refreshButton_ = new QPushButton("Refresh quotes", filtersCard);
    refreshButton_->setObjectName("primaryButton");
    loadHistoryButton_ = new QPushButton("Load history", filtersCard);
    syncButton_ = new QPushButton("Sync to server", filtersCard);

    filtersLayout->addWidget(new QLabel("Base currency", filtersCard), 0, 0);
    filtersLayout->addWidget(baseCurrencyCombo_, 1, 0);
    filtersLayout->addWidget(new QLabel("Quote currency", filtersCard), 0, 1);
    filtersLayout->addWidget(quoteCurrencyCombo_, 1, 1);
    filtersLayout->addWidget(refreshButton_, 1, 2);
    filtersLayout->addWidget(loadHistoryButton_, 1, 3);
    filtersLayout->addWidget(syncButton_, 1, 4);

    rootLayout->addWidget(filtersCard);

    auto* contentLayout = new QGridLayout();
    contentLayout->setHorizontalSpacing(20);
    contentLayout->setVerticalSpacing(20);

    ratesTableWidget_ = new RatesTableWidget(this);
    historyChartWidget_ = new HistoryChartWidget(this);
    aggregationSummaryWidget_ = new AggregationSummaryWidget(this);

    contentLayout->addWidget(ratesTableWidget_, 0, 0, 1, 2);
    contentLayout->addWidget(historyChartWidget_, 1, 0);
    contentLayout->addWidget(aggregationSummaryWidget_, 1, 1);

    rootLayout->addLayout(contentLayout, 1);
}

QComboBox* DashboardPage::baseCurrencyCombo() const { return baseCurrencyCombo_; }
QComboBox* DashboardPage::quoteCurrencyCombo() const { return quoteCurrencyCombo_; }
QPushButton* DashboardPage::refreshButton() const { return refreshButton_; }
QPushButton* DashboardPage::loadHistoryButton() const { return loadHistoryButton_; }
QPushButton* DashboardPage::syncButton() const { return syncButton_; }
RatesTableWidget* DashboardPage::ratesTableWidget() const { return ratesTableWidget_; }
HistoryChartWidget* DashboardPage::historyChartWidget() const { return historyChartWidget_; }
AggregationSummaryWidget* DashboardPage::aggregationSummaryWidget() const { return aggregationSummaryWidget_; }

}