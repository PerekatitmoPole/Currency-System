#include "views/widgets/BurgerMenuWidget.hpp"

#include "common/UiText.hpp"
#include "dto/ApiDtos.hpp"

#include <QLabel>
#include <QPushButton>
#include <QSize>
#include <QStyle>
#include <QVBoxLayout>

namespace currency::client::views {

BurgerMenuWidget::BurgerMenuWidget(QWidget* parent)
    : QWidget(parent) {
    setObjectName("burgerMenuWidget");
    setAttribute(Qt::WA_StyledBackground, true);

    auto* rootLayout = new QVBoxLayout(this);
    rootLayout->setContentsMargins(20, 20, 20, 20);
    rootLayout->setSpacing(16);

    auto* brandCard = new QWidget(this);
    brandCard->setObjectName("menuBrandCard");
    auto* brandLayout = new QVBoxLayout(brandCard);
    brandLayout->setContentsMargins(20, 20, 20, 20);
    brandLayout->setSpacing(8);

    auto* title = new QLabel("Currency", brandCard);
    title->setObjectName("menuTitleLabel");
    brandLayout->addWidget(title);

    auto* subtitle = new QLabel(QString::fromUtf8(u8"Приложение для курсов валют, построения графиков"), brandCard);
    subtitle->setObjectName("menuSubtitleLabel");
    subtitle->setWordWrap(true);
    brandLayout->addWidget(subtitle);
    rootLayout->addWidget(brandCard);

    auto* statusCard = new QWidget(this);
    statusCard->setObjectName("menuStatusCard");
    auto* statusLayout = new QVBoxLayout(statusCard);
    statusLayout->setContentsMargins(20, 20, 20, 20);
    statusLayout->setSpacing(8);

    auto* statusTitle = new QLabel(common::UiText::connectionStatusTitle(), statusCard);
    statusTitle->setObjectName("menuSectionLabel");
    statusLayout->addWidget(statusTitle);

    statusBadge_ = new QLabel(common::UiText::noConnection(), statusCard);
    statusBadge_->setObjectName("menuStatusBadge");
    statusBadge_->setProperty("connected", false);
    statusLayout->addWidget(statusBadge_, 0, Qt::AlignLeft);

    endpointLabel_ = new QLabel("127.0.0.1:5555", statusCard);
    endpointLabel_->setObjectName("menuEndpointLabel");
    statusLayout->addWidget(endpointLabel_);

    detailsLabel_ = new QLabel(common::UiText::connectionMissingDetails(), statusCard);
    detailsLabel_->setObjectName("menuDetailsLabel");
    detailsLabel_->setWordWrap(true);
    statusLayout->addWidget(detailsLabel_);
    rootLayout->addWidget(statusCard);

    auto* navigationTitle = new QLabel(common::UiText::sectionsTitle(), this);
    navigationTitle->setObjectName("menuSectionLabel");
    rootLayout->addWidget(navigationTitle);

    addButton(common::UiText::sourcePageTitle(), Page::Start, QStyle::SP_DirHomeIcon);
    addButton(common::UiText::dashboardPageTitle(), Page::Dashboard, QStyle::SP_DesktopIcon);
    addButton(common::UiText::settingsPageTitle(), Page::Settings, QStyle::SP_FileDialogInfoView);

    auto* providersCard = new QWidget(this);
    providersCard->setObjectName("menuProvidersCard");
    auto* providersLayout = new QVBoxLayout(providersCard);
    providersLayout->setContentsMargins(20, 20, 20, 20);
    providersLayout->setSpacing(8);

    auto* providersTitle = new QLabel(common::UiText::activeSourcesTitle(), providersCard);
    providersTitle->setObjectName("menuSectionLabel");
    providersLayout->addWidget(providersTitle);

    providersLabel_ = new QLabel(QString::fromUtf8(u8"ЦБ РФ / ECB / Frankfurter"), providersCard);
    providersLabel_->setObjectName("menuProvidersLabel");
    providersLabel_->setWordWrap(true);
    providersLayout->addWidget(providersLabel_);
    rootLayout->addWidget(providersCard);

    rootLayout->addStretch();
    setCurrentPage(Page::Start);
}

void BurgerMenuWidget::setCurrentPage(const Page page) {
    for (auto iterator = buttons_.begin(); iterator != buttons_.end(); ++iterator) {
        iterator.value()->setProperty("selected", iterator.key() == page);
        iterator.value()->style()->unpolish(iterator.value());
        iterator.value()->style()->polish(iterator.value());
        iterator.value()->update();
    }
}

void BurgerMenuWidget::setConnectionStatus(const dto::ConnectionStatusDto& status) {
    statusBadge_->setText(status.connected ? common::UiText::connectionActive() : common::UiText::noConnection());
    statusBadge_->setProperty("connected", status.connected);
    statusBadge_->style()->unpolish(statusBadge_);
    statusBadge_->style()->polish(statusBadge_);
    statusBadge_->update();

    endpointLabel_->setText(QString("%1:%2").arg(status.host).arg(status.port));
    detailsLabel_->setText(status.message);
}

void BurgerMenuWidget::setSelectedSources(const QList<dto::ApiSource>& sources) {
    if (sources.isEmpty()) {
        providersLabel_->setText(common::UiText::noSourcesSelected());
        return;
    }

    QStringList displayNames;
    displayNames.reserve(sources.size());
    for (const auto source : sources) {
        displayNames.push_back(dto::toDisplayName(source));
    }

    providersLabel_->setText(displayNames.join(" / "));
}

void BurgerMenuWidget::addButton(const QString& title, const Page page, const QStyle::StandardPixmap iconType) {
    auto* button = new QPushButton(title, this);
    button->setObjectName("menuButton");
    button->setProperty("selected", false);
    button->setIcon(style()->standardIcon(iconType));
    button->setIconSize(QSize(18, 18));
    layout()->addWidget(button);
    buttons_.insert(page, button);

    connect(button, &QPushButton::clicked, this, [this, page] {
        emit pageRequested(page);
    });
}

}
