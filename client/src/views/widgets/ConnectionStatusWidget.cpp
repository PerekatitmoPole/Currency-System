#include "views/widgets/ConnectionStatusWidget.hpp"

#include "common/UiText.hpp"

#include <QLabel>
#include <QStyle>
#include <QVBoxLayout>

namespace currency::client::views {

ConnectionStatusWidget::ConnectionStatusWidget(QWidget* parent)
    : QWidget(parent) {
    setObjectName("cardWidget");

    auto* layout = new QVBoxLayout(this);
    layout->setContentsMargins(20, 20, 20, 20);
    layout->setSpacing(10);

    auto* title = new QLabel(common::UiText::serverStatusTitle(), this);
    title->setObjectName("sectionTitleLabel");
    layout->addWidget(title);

    stateLabel_ = new QLabel(common::UiText::noConnection(), this);
    stateLabel_->setObjectName("connectionStateBadge");
    stateLabel_->setProperty("connected", false);
    layout->addWidget(stateLabel_, 0, Qt::AlignLeft);

    endpointLabel_ = new QLabel("127.0.0.1:5555", this);
    endpointLabel_->setObjectName("connectionEndpointLabel");
    layout->addWidget(endpointLabel_);

    detailsLabel_ = new QLabel(common::UiText::defaultDisconnectedState(), this);
    detailsLabel_->setObjectName("connectionDetailsLabel");
    detailsLabel_->setWordWrap(true);
    layout->addWidget(detailsLabel_);
}

void ConnectionStatusWidget::setStatus(const dto::ConnectionStatusDto& status) {
    stateLabel_->setText(status.connected ? common::UiText::connected() : common::UiText::noConnection());
    stateLabel_->setProperty("connected", status.connected);
    stateLabel_->style()->unpolish(stateLabel_);
    stateLabel_->style()->polish(stateLabel_);
    stateLabel_->update();
    endpointLabel_->setText(QString("%1:%2").arg(status.host).arg(status.port));
    detailsLabel_->setText(status.message);
}

}
