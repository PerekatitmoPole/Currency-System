#pragma once

#include "dto/UiDtos.hpp"

#include <QWidget>

class QLabel;

namespace currency::client::views {

class ConnectionStatusWidget : public QWidget {
    Q_OBJECT

public:
    ConnectionStatusWidget(QWidget* parent = nullptr);

    void setStatus(const dto::ConnectionStatusDto& status);

private:
    QLabel* stateLabel_{nullptr};
    QLabel* endpointLabel_{nullptr};
    QLabel* detailsLabel_{nullptr};
};

}