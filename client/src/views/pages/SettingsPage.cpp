#include "views/pages/SettingsPage.hpp"

#include <QFormLayout>
#include <QAbstractSpinBox>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QSpinBox>
#include <QStyle>
#include <QVBoxLayout>
#include <QGridLayout>
#include <QWidget>

namespace currency::client::views {

SettingsPage::SettingsPage(QWidget* parent)
    : QWidget(parent) {
    setObjectName("settingsPage");
    auto* rootLayout = new QVBoxLayout(this);
    rootLayout->setContentsMargins(36, 24, 36, 24);
    rootLayout->setSpacing(12);

    auto* title = new QLabel(QString::fromUtf8(u8"Настройки"), this);
    title->setObjectName("pageTitleLabel");
    rootLayout->addWidget(title);

    auto* subtitle = new QLabel(
        QString::fromUtf8(u8"Здесь настраивается подключение к серверу. Выбор источников и валют выполняется в рабочих вкладках."),
        this);
    subtitle->setObjectName("pageSubtitleLabel");
    subtitle->setWordWrap(true);
    rootLayout->addWidget(subtitle);

    auto* contentLayout = new QGridLayout();
    contentLayout->setHorizontalSpacing(18);
    contentLayout->setVerticalSpacing(18);

    auto* connectionGroup = new QGroupBox(QString::fromUtf8(u8"Подключение к серверу"), this);
    connectionGroup->setObjectName("cardWidget");

    auto* connectionForm = new QFormLayout(connectionGroup);
    connectionForm->setContentsMargins(24, 24, 24, 20);
    connectionForm->setHorizontalSpacing(18);
    connectionForm->setVerticalSpacing(12);
    connectionForm->setLabelAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    connectionForm->setFormAlignment(Qt::AlignLeft | Qt::AlignTop);
    connectionForm->setFieldGrowthPolicy(QFormLayout::AllNonFixedFieldsGrow);
    connectionForm->setRowWrapPolicy(QFormLayout::DontWrapRows);

    hostEdit_ = new QLineEdit(connectionGroup);
    hostEdit_->setMinimumWidth(280);

    portSpinBox_ = new QSpinBox(connectionGroup);
    portSpinBox_->setRange(1, 65535);
    portSpinBox_->setValue(5555);
    portSpinBox_->setButtonSymbols(QAbstractSpinBox::NoButtons);
    portSpinBox_->setAlignment(Qt::AlignLeft);

    connectionForm->addRow(QString::fromUtf8(u8"Хост"), hostEdit_);
    connectionForm->addRow(QString::fromUtf8(u8"Порт"), portSpinBox_);

    connectButton_ = new QPushButton(QString::fromUtf8(u8"Проверить подключение"), connectionGroup);
    connectButton_->setObjectName("secondaryButton");
    connectButton_->setMinimumWidth(220);

    applyButton_ = new QPushButton(QString::fromUtf8(u8"Применить настройки"), connectionGroup);
    applyButton_->setObjectName("primaryButton");
    applyButton_->setMinimumWidth(220);

    auto* actionsWidget = new QWidget(connectionGroup);
    auto* actionsLayout = new QHBoxLayout(actionsWidget);
    actionsLayout->setContentsMargins(0, 4, 0, 0);
    actionsLayout->setSpacing(12);
    actionsLayout->addWidget(connectButton_);
    actionsLayout->addWidget(applyButton_);
    actionsLayout->addStretch();

    auto* emptyLabel = new QLabel(QString(), connectionGroup);
    connectionForm->addRow(emptyLabel, actionsWidget);

    auto* statusGroup = new QGroupBox(QString::fromUtf8(u8"Состояние соединения"), this);
    statusGroup->setObjectName("cardWidget");
    auto* statusLayout = new QVBoxLayout(statusGroup);
    statusLayout->setContentsMargins(24, 24, 24, 20);
    statusLayout->setSpacing(10);

    stateLabel_ = new QLabel(QString::fromUtf8(u8"Нет подключения"), statusGroup);
    stateLabel_->setObjectName("connectionStateBadge");
    stateLabel_->setProperty("connected", false);
    statusLayout->addWidget(stateLabel_, 0, Qt::AlignLeft);

    endpointLabel_ = new QLabel("127.0.0.1:5555", statusGroup);
    endpointLabel_->setObjectName("connectionEndpointLabel");
    statusLayout->addWidget(endpointLabel_);

    detailsLabel_ = new QLabel(QString::fromUtf8(u8"Проверьте адрес сервера и подключение."), statusGroup);
    detailsLabel_->setObjectName("connectionDetailsLabel");
    detailsLabel_->setWordWrap(true);
    statusLayout->addWidget(detailsLabel_);
    statusLayout->addStretch();

    contentLayout->addWidget(connectionGroup, 0, 0);
    contentLayout->addWidget(statusGroup, 1, 0);
    contentLayout->setColumnStretch(0, 1);
    contentLayout->setRowStretch(0, 1);
    contentLayout->setRowStretch(1, 1);
    rootLayout->addLayout(contentLayout, 1);
}

QLineEdit* SettingsPage::hostEdit() const { return hostEdit_; }
QSpinBox* SettingsPage::portSpinBox() const { return portSpinBox_; }
QPushButton* SettingsPage::connectButton() const { return connectButton_; }
QPushButton* SettingsPage::applyButton() const { return applyButton_; }

void SettingsPage::setConnectionOverview(
    const QString& state,
    const QString& endpoint,
    const QString& details,
    const bool connected) {
    stateLabel_->setText(state);
    stateLabel_->setProperty("connected", connected);
    style()->unpolish(stateLabel_);
    style()->polish(stateLabel_);
    stateLabel_->update();
    endpointLabel_->setText(endpoint);
    detailsLabel_->setText(details);
}

}