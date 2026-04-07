#pragma once

#include <QObject>

namespace currency::client::app {
class MainWindow;
}

namespace currency::client::views {
class BurgerMenuWidget;
}

namespace currency::client::controllers {

class NavigationController : public QObject {
    Q_OBJECT

public:
    NavigationController(views::BurgerMenuWidget& menu, app::MainWindow& mainWindow, QObject* parent = nullptr);

private:
    app::MainWindow& mainWindow_;
};

}