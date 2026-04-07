#include "app/MainWindow.hpp"

#include "views/pages/AggregationPage.hpp"
#include "views/pages/DashboardPage.hpp"
#include "views/pages/HistoryPage.hpp"
#include "views/pages/RatesPage.hpp"
#include "views/pages/SettingsPage.hpp"
#include "views/pages/StartPage.hpp"
#include "views/widgets/BurgerMenuWidget.hpp"

#include <QHBoxLayout>
#include <QStackedWidget>
#include <QWidget>

namespace currency::client::app {

MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent) {
    setWindowTitle("Currency System");
    resize(1480, 960);

    auto* central = new QWidget(this);
    auto* layout = new QHBoxLayout(central);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);

    burgerMenu_ = new views::BurgerMenuWidget(central);
    burgerMenu_->setFixedWidth(250);
    layout->addWidget(burgerMenu_);

    pageStack_ = new QStackedWidget(central);
    startPage_ = new views::StartPage(pageStack_);
    dashboardPage_ = new views::DashboardPage(pageStack_);
    ratesPage_ = new views::RatesPage(pageStack_);
    historyPage_ = new views::HistoryPage(pageStack_);
    aggregationPage_ = new views::AggregationPage(pageStack_);
    settingsPage_ = new views::SettingsPage(pageStack_);

    pageStack_->addWidget(startPage_);
    pageStack_->addWidget(dashboardPage_);
    pageStack_->addWidget(ratesPage_);
    pageStack_->addWidget(historyPage_);
    pageStack_->addWidget(aggregationPage_);
    pageStack_->addWidget(settingsPage_);
    layout->addWidget(pageStack_, 1);

    setCentralWidget(central);
    showPage(views::BurgerMenuWidget::Page::Start);
}

void MainWindow::showPage(const views::BurgerMenuWidget::Page page) {
    switch (page) {
    case views::BurgerMenuWidget::Page::Start:
        pageStack_->setCurrentWidget(startPage_);
        break;
    case views::BurgerMenuWidget::Page::Dashboard:
        pageStack_->setCurrentWidget(dashboardPage_);
        break;
    case views::BurgerMenuWidget::Page::Rates:
        pageStack_->setCurrentWidget(ratesPage_);
        break;
    case views::BurgerMenuWidget::Page::History:
        pageStack_->setCurrentWidget(historyPage_);
        break;
    case views::BurgerMenuWidget::Page::Aggregation:
        pageStack_->setCurrentWidget(aggregationPage_);
        break;
    case views::BurgerMenuWidget::Page::Settings:
        pageStack_->setCurrentWidget(settingsPage_);
        break;
    }
}

views::BurgerMenuWidget* MainWindow::burgerMenu() const { return burgerMenu_; }
views::StartPage* MainWindow::startPage() const { return startPage_; }
views::DashboardPage* MainWindow::dashboardPage() const { return dashboardPage_; }
views::RatesPage* MainWindow::ratesPage() const { return ratesPage_; }
views::HistoryPage* MainWindow::historyPage() const { return historyPage_; }
views::AggregationPage* MainWindow::aggregationPage() const { return aggregationPage_; }
views::SettingsPage* MainWindow::settingsPage() const { return settingsPage_; }

}