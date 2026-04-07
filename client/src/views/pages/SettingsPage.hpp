#pragma once

#include "dto/ApiDtos.hpp"

#include <QHash>
#include <QWidget>

class QLineEdit;
class QPushButton;
class QSpinBox;

namespace currency::client::views {

class SettingsPage : public QWidget {
    Q_OBJECT

public:
    SettingsPage(QWidget* parent = nullptr);

    QLineEdit* hostEdit() const;
    QSpinBox* portSpinBox() const;
    QLineEdit* defaultBaseEdit() const;
    QLineEdit* defaultQuotesEdit() const;
    QLineEdit* apiKeyEdit(dto::ApiSource source) const;
    QPushButton* connectButton() const;
    QPushButton* applyButton() const;

private:
    QLineEdit* hostEdit_{nullptr};
    QSpinBox* portSpinBox_{nullptr};
    QLineEdit* defaultBaseEdit_{nullptr};
    QLineEdit* defaultQuotesEdit_{nullptr};
    QPushButton* connectButton_{nullptr};
    QPushButton* applyButton_{nullptr};
    QHash<dto::ApiSource, QLineEdit*> apiKeyEdits_;
};

}