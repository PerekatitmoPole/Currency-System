#include "controllers/NavigationController.hpp"

#include "app/MainWindow.hpp"
#include "views/widgets/BurgerMenuWidget.hpp"

namespace currency::client::controllers {

NavigationController::NavigationController(views::BurgerMenuWidget& menu, app::MainWindow& mainWindow, QObject* parent)
    : QObject(parent),
      mainWindow_(mainWindow) {
    connect(&menu, &views::BurgerMenuWidget::pageRequested, &mainWindow_, [this](const views::BurgerMenuWidget::Page page) {
        mainWindow_.showPage(page);
    });
}

}