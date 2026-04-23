#pragma once

#include "dto/ApiDtos.hpp"
#include "dto/UiDtos.hpp"
#include "views/widgets/BurgerMenuWidget.hpp"

#include <QMainWindow>

namespace currency::client::views {
class StartPage;
class DashboardPage;
class SettingsPage;
}

class QStackedWidget;

namespace currency::client::app {

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    MainWindow(QWidget* parent = nullptr);

    void showPage(views::BurgerMenuWidget::Page page);
    void setConnectionStatus(const dto::ConnectionStatusDto& status);
    void setSelectedSources(const QList<dto::ApiSource>& sources);
    void showFeedbackMessage(const QString& title, const QString& message, bool error);

    views::BurgerMenuWidget* burgerMenu() const;
    views::StartPage* startPage() const;
    views::DashboardPage* dashboardPage() const;
    views::SettingsPage* settingsPage() const;

private:
    views::BurgerMenuWidget* burgerMenu_{nullptr};
    QStackedWidget* pageStack_{nullptr};
    views::StartPage* startPage_{nullptr};
    views::DashboardPage* dashboardPage_{nullptr};
    views::SettingsPage* settingsPage_{nullptr};
};

}
