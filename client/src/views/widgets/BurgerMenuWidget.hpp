#pragma once

#include "dto/ApiDtos.hpp"
#include "dto/UiDtos.hpp"

#include <QHash>
#include <QStyle>
#include <QWidget>

class QPushButton;
class QLabel;

namespace currency::client::views {

class BurgerMenuWidget : public QWidget {
    Q_OBJECT

public:
    enum class Page {
        Start,
        Dashboard,
        Settings,
    };

    BurgerMenuWidget(QWidget* parent = nullptr);
    void setCurrentPage(Page page);
    void setConnectionStatus(const dto::ConnectionStatusDto& status);
    void setSelectedSources(const QList<dto::ApiSource>& sources);

signals:
    void pageRequested(Page page);

private:
    void addButton(const QString& title, Page page, QStyle::StandardPixmap iconType);

    QHash<Page, QPushButton*> buttons_;
    QLabel* statusBadge_{nullptr};
    QLabel* endpointLabel_{nullptr};
    QLabel* detailsLabel_{nullptr};
    QLabel* providersLabel_{nullptr};
};

}
