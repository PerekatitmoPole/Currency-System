#pragma once

#include <QWidget>

class QPushButton;

namespace currency::client::views {

class BurgerMenuWidget : public QWidget {
    Q_OBJECT

public:
    enum class Page {
        Start,
        Dashboard,
        Rates,
        History,
        Aggregation,
        Settings,
    };

    BurgerMenuWidget(QWidget* parent = nullptr);

signals:
    void pageRequested(Page page);

private:
    void addButton(const QString& title, Page page);
};

}