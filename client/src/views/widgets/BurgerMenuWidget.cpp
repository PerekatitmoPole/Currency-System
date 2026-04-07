#include "views/widgets/BurgerMenuWidget.hpp"

#include <QLabel>
#include <QPushButton>
#include <QVBoxLayout>

namespace currency::client::views {

BurgerMenuWidget::BurgerMenuWidget(QWidget* parent)
    : QWidget(parent) {
    setObjectName("burgerMenuWidget");

    auto* layout = new QVBoxLayout(this);
    layout->setContentsMargins(20, 24, 20, 24);
    layout->setSpacing(10);

    auto* title = new QLabel("Currency\nSystem", this);
    title->setObjectName("menuTitleLabel");
    layout->addWidget(title);
    layout->addSpacing(24);

    addButton("Start", Page::Start);
    addButton("Dashboard", Page::Dashboard);
    addButton("Rates", Page::Rates);
    addButton("History", Page::History);
    addButton("Aggregation", Page::Aggregation);
    addButton("Settings", Page::Settings);
    layout->addStretch();
}

void BurgerMenuWidget::addButton(const QString& title, const Page page) {
    auto* button = new QPushButton(title, this);
    button->setObjectName("menuButton");
    layout()->addWidget(button);
    connect(button, &QPushButton::clicked, this, [this, page] {
        emit pageRequested(page);
    });
}

}