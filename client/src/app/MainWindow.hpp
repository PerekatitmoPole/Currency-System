#pragma once

#include "views/widgets/BurgerMenuWidget.hpp"

#include <QMainWindow>

namespace currency::client::views {
class StartPage;
class DashboardPage;
class RatesPage;
class HistoryPage;
class AggregationPage;
class SettingsPage;
}

class QStackedWidget;

namespace currency::client::app {

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    MainWindow(QWidget* parent = nullptr);

    void showPage(views::BurgerMenuWidget::Page page);

    views::BurgerMenuWidget* burgerMenu() const;
    views::StartPage* startPage() const;
    views::DashboardPage* dashboardPage() const;
    views::RatesPage* ratesPage() const;
    views::HistoryPage* historyPage() const;
    views::AggregationPage* aggregationPage() const;
    views::SettingsPage* settingsPage() const;

private:
    views::BurgerMenuWidget* burgerMenu_{nullptr};
    QStackedWidget* pageStack_{nullptr};
    views::StartPage* startPage_{nullptr};
    views::DashboardPage* dashboardPage_{nullptr};
    views::RatesPage* ratesPage_{nullptr};
    views::HistoryPage* historyPage_{nullptr};
    views::AggregationPage* aggregationPage_{nullptr};
    views::SettingsPage* settingsPage_{nullptr};
};

}