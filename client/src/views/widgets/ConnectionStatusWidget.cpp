#include "views/widgets/ConnectionStatusWidget.hpp"

#include <QLabel>
#include <QVBoxLayout>

namespace currency::client::views {

ConnectionStatusWidget::ConnectionStatusWidget(QWidget* parent)
    : QWidget(parent) {
    setObjectName("cardWidget");

    auto* layout = new QVBoxLayout(this);
    layout->setContentsMargins(20, 20, 20, 20);
    layout->setSpacing(8);

    auto* title = new QLabel("Server connection", this);
    title->setObjectName("sectionTitleLabel");
    layout->addWidget(title);

    stateLabel_ = new QLabel("Disconnected", this);
    stateLabel_->setObjectName("statusLabel");
    layout->addWidget(stateLabel_);

    endpointLabel_ = new QLabel("127.0.0.1:5555", this);
    endpointLabel_->setObjectName("mutedLabel");
    layout->addWidget(endpointLabel_);

    detailsLabel_ = new QLabel("No active TCP connection.", this);
    detailsLabel_->setObjectName("mutedLabel");
    detailsLabel_->setWordWrap(true);
    layout->addWidget(detailsLabel_);
}

void ConnectionStatusWidget::setStatus(const dto::ConnectionStatusDto& status) {
    stateLabel_->setText(status.connected ? "Connected" : "Disconnected");
    endpointLabel_->setText(QString("%1:%2").arg(status.host).arg(status.port));
    detailsLabel_->setText(status.message);
}

}