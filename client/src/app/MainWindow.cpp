#include "app/MainWindow.hpp"

#include "common/UiText.hpp"
#include "views/pages/DashboardPage.hpp"
#include "views/pages/SettingsPage.hpp"
#include "views/pages/StartPage.hpp"
#include "views/widgets/BurgerMenuWidget.hpp"

#include <QFrame>
#include <QHBoxLayout>
#include <QScrollArea>
#include <QStackedWidget>
#include <QStatusBar>
#include <QStyle>
#include <QVBoxLayout>
#include <QWidget>

namespace currency::client::app {

MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent) {
    setWindowTitle(common::UiText::appWindowTitle());
    resize(1540, 980);

    auto* central = new QWidget(this);
    auto* layout = new QHBoxLayout(central);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);

    burgerMenu_ = new views::BurgerMenuWidget(central);
    burgerMenu_->setFixedWidth(270);
    layout->addWidget(burgerMenu_);

    auto* contentShell = new QWidget(central);
    contentShell->setObjectName("contentShell");
    auto* contentLayout = new QVBoxLayout(contentShell);
    contentLayout->setContentsMargins(24, 24, 24, 14);
    contentLayout->setSpacing(0);

    pageStack_ = new QStackedWidget(contentShell);
    pageStack_->setObjectName("pageStack");
    startPage_ = new views::StartPage(pageStack_);
    dashboardPage_ = new views::DashboardPage(pageStack_);
    settingsPage_ = new views::SettingsPage(pageStack_);

    auto wrapPage = [this](QWidget* page) {
        auto* area = new QScrollArea(pageStack_);
        area->setWidgetResizable(true);
        area->setFrameShape(QFrame::NoFrame);
        area->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
        area->setWidget(page);
        return area;
    };

    pageStack_->addWidget(wrapPage(startPage_));
    pageStack_->addWidget(wrapPage(dashboardPage_));
    pageStack_->addWidget(wrapPage(settingsPage_));

    contentLayout->addWidget(pageStack_, 1);
    layout->addWidget(contentShell, 1);

    setCentralWidget(central);
    statusBar()->setSizeGripEnabled(false);
    statusBar()->showMessage(common::UiText::readyMessage());
    showPage(views::BurgerMenuWidget::Page::Start);
}

void MainWindow::showPage(const views::BurgerMenuWidget::Page page) {
    switch (page) {
    case views::BurgerMenuWidget::Page::Start:
        pageStack_->setCurrentIndex(0);
        break;
    case views::BurgerMenuWidget::Page::Dashboard:
        pageStack_->setCurrentIndex(1);
        break;
    case views::BurgerMenuWidget::Page::Settings:
        pageStack_->setCurrentIndex(2);
        break;
    }

    burgerMenu_->setCurrentPage(page);
}

void MainWindow::setConnectionStatus(const dto::ConnectionStatusDto& status) {
    burgerMenu_->setConnectionStatus(status);
}

void MainWindow::setSelectedSources(const QList<dto::ApiSource>& sources) {
    burgerMenu_->setSelectedSources(sources);
}

void MainWindow::showFeedbackMessage(const QString& title, const QString& message, const bool error) {
    statusBar()->setProperty("error", error);
    statusBar()->style()->unpolish(statusBar());
    statusBar()->style()->polish(statusBar());
    statusBar()->showMessage(QString("%1: %2").arg(title, message), 9000);
}

views::BurgerMenuWidget* MainWindow::burgerMenu() const { return burgerMenu_; }
views::StartPage* MainWindow::startPage() const { return startPage_; }
views::DashboardPage* MainWindow::dashboardPage() const { return dashboardPage_; }
views::SettingsPage* MainWindow::settingsPage() const { return settingsPage_; }

}
