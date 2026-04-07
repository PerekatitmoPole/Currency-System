#include "views/pages/SettingsPage.hpp"

#include <QFormLayout>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QScrollArea>
#include <QSpinBox>
#include <QVBoxLayout>

namespace currency::client::views {

SettingsPage::SettingsPage(QWidget* parent)
    : QWidget(parent) {
    auto* rootLayout = new QVBoxLayout(this);
    rootLayout->setContentsMargins(28, 28, 28, 28);
    rootLayout->setSpacing(20);

    auto* title = new QLabel("Settings", this);
    title->setObjectName("pageTitleLabel");
    rootLayout->addWidget(title);

    auto* connectionGroup = new QGroupBox("Server connection", this);
    connectionGroup->setObjectName("cardWidget");
    auto* connectionForm = new QFormLayout(connectionGroup);
    connectionForm->setContentsMargins(20, 20, 20, 20);
    connectionForm->setSpacing(12);

    hostEdit_ = new QLineEdit(connectionGroup);
    portSpinBox_ = new QSpinBox(connectionGroup);
    portSpinBox_->setRange(1, 65535);
    portSpinBox_->setValue(5555);
    defaultBaseEdit_ = new QLineEdit(connectionGroup);
    defaultQuotesEdit_ = new QLineEdit(connectionGroup);
    defaultQuotesEdit_->setPlaceholderText("USD, GBP, RUB, JPY, CNY");

    connectionForm->addRow("Host", hostEdit_);
    connectionForm->addRow("Port", portSpinBox_);
    connectionForm->addRow("Default base", defaultBaseEdit_);
    connectionForm->addRow("Default quotes", defaultQuotesEdit_);

    auto* actionsLayout = new QHBoxLayout();
    connectButton_ = new QPushButton("Test connection", connectionGroup);
    connectButton_->setObjectName("secondaryButton");
    applyButton_ = new QPushButton("Apply settings", connectionGroup);
    applyButton_->setObjectName("primaryButton");
    actionsLayout->addWidget(connectButton_);
    actionsLayout->addWidget(applyButton_);
    connectionForm->addRow(actionsLayout);

    rootLayout->addWidget(connectionGroup);

    auto* apiKeysGroup = new QGroupBox("API keys", this);
    apiKeysGroup->setObjectName("cardWidget");
    auto* apiKeysForm = new QFormLayout(apiKeysGroup);
    apiKeysForm->setContentsMargins(20, 20, 20, 20);
    apiKeysForm->setSpacing(12);

    for (const auto source : dto::allApiSources()) {
        auto* edit = new QLineEdit(apiKeysGroup);
        edit->setEchoMode(QLineEdit::PasswordEchoOnEdit);
        edit->setPlaceholderText(dto::requiresApiKey(source) ? "API key required for this source" : "Optional / not required");
        apiKeysForm->addRow(dto::toDisplayName(source), edit);
        apiKeyEdits_.insert(source, edit);
    }

    rootLayout->addWidget(apiKeysGroup);
    rootLayout->addStretch();
}

QLineEdit* SettingsPage::hostEdit() const { return hostEdit_; }
QSpinBox* SettingsPage::portSpinBox() const { return portSpinBox_; }
QLineEdit* SettingsPage::defaultBaseEdit() const { return defaultBaseEdit_; }
QLineEdit* SettingsPage::defaultQuotesEdit() const { return defaultQuotesEdit_; }
QLineEdit* SettingsPage::apiKeyEdit(const dto::ApiSource source) const { return apiKeyEdits_.value(source); }
QPushButton* SettingsPage::connectButton() const { return connectButton_; }
QPushButton* SettingsPage::applyButton() const { return applyButton_; }

}