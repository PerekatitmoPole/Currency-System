#include "views/pages/HistoryPage.hpp"

#include "views/widgets/HistoryChartWidget.hpp"

#include <QComboBox>
#include <QDateEdit>
#include <QGridLayout>
#include <QHeaderView>
#include <QLabel>
#include <QPushButton>
#include <QTableView>
#include <QVBoxLayout>

namespace currency::client::views {

HistoryPage::HistoryPage(QWidget* parent)
    : QWidget(parent) {
    auto* rootLayout = new QVBoxLayout(this);
    rootLayout->setContentsMargins(28, 28, 28, 28);
    rootLayout->setSpacing(20);

    auto* title = new QLabel("History", this);
    title->setObjectName("pageTitleLabel");
    rootLayout->addWidget(title);

    auto* filtersCard = new QWidget(this);
    filtersCard->setObjectName("cardWidget");
    auto* filtersLayout = new QGridLayout(filtersCard);
    filtersLayout->setContentsMargins(20, 20, 20, 20);
    filtersLayout->setHorizontalSpacing(16);
    filtersLayout->setVerticalSpacing(12);

    sourceCombo_ = new QComboBox(filtersCard);
    baseCurrencyCombo_ = new QComboBox(filtersCard);
    quoteCurrencyCombo_ = new QComboBox(filtersCard);
    fromDateEdit_ = new QDateEdit(QDate::currentDate().addDays(-30), filtersCard);
    toDateEdit_ = new QDateEdit(QDate::currentDate(), filtersCard);
    fromDateEdit_->setCalendarPopup(true);
    toDateEdit_->setCalendarPopup(true);
    loadButton_ = new QPushButton("Load history", filtersCard);
    loadButton_->setObjectName("primaryButton");

    filtersLayout->addWidget(new QLabel("Source", filtersCard), 0, 0);
    filtersLayout->addWidget(sourceCombo_, 1, 0);
    filtersLayout->addWidget(new QLabel("Base", filtersCard), 0, 1);
    filtersLayout->addWidget(baseCurrencyCombo_, 1, 1);
    filtersLayout->addWidget(new QLabel("Quote", filtersCard), 0, 2);
    filtersLayout->addWidget(quoteCurrencyCombo_, 1, 2);
    filtersLayout->addWidget(new QLabel("From", filtersCard), 0, 3);
    filtersLayout->addWidget(fromDateEdit_, 1, 3);
    filtersLayout->addWidget(new QLabel("To", filtersCard), 0, 4);
    filtersLayout->addWidget(toDateEdit_, 1, 4);
    filtersLayout->addWidget(loadButton_, 1, 5);

    rootLayout->addWidget(filtersCard);

    chartWidget_ = new HistoryChartWidget(this);
    rootLayout->addWidget(chartWidget_, 1);

    auto* tableCard = new QWidget(this);
    tableCard->setObjectName("cardWidget");
    auto* tableLayout = new QVBoxLayout(tableCard);
    tableLayout->setContentsMargins(20, 20, 20, 20);
    tableLayout->setSpacing(12);
    auto* tableTitle = new QLabel("History table", tableCard);
    tableTitle->setObjectName("sectionTitleLabel");
    tableLayout->addWidget(tableTitle);

    tableView_ = new QTableView(tableCard);
    tableView_->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    tableView_->setEditTriggers(QAbstractItemView::NoEditTriggers);
    tableLayout->addWidget(tableView_);

    rootLayout->addWidget(tableCard, 1);
}

QComboBox* HistoryPage::sourceCombo() const { return sourceCombo_; }
QComboBox* HistoryPage::baseCurrencyCombo() const { return baseCurrencyCombo_; }
QComboBox* HistoryPage::quoteCurrencyCombo() const { return quoteCurrencyCombo_; }
QDateEdit* HistoryPage::fromDateEdit() const { return fromDateEdit_; }
QDateEdit* HistoryPage::toDateEdit() const { return toDateEdit_; }
QPushButton* HistoryPage::loadButton() const { return loadButton_; }
HistoryChartWidget* HistoryPage::chartWidget() const { return chartWidget_; }
QTableView* HistoryPage::tableView() const { return tableView_; }

}