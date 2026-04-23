#pragma once

#include <QWidget>

class QLineEdit;
class QLabel;
class QPushButton;
class QSpinBox;
class QString;

namespace currency::client::views {

class SettingsPage : public QWidget {
    Q_OBJECT

public:
    SettingsPage(QWidget* parent = nullptr);

    QLineEdit* hostEdit() const;
    QSpinBox* portSpinBox() const;
    QPushButton* connectButton() const;
    QPushButton* applyButton() const;
    void setConnectionOverview(const QString& state, const QString& endpoint, const QString& details, bool connected);

private:
    QLineEdit* hostEdit_{nullptr};
    QSpinBox* portSpinBox_{nullptr};
    QPushButton* connectButton_{nullptr};
    QPushButton* applyButton_{nullptr};
    QLabel* stateLabel_{nullptr};
    QLabel* endpointLabel_{nullptr};
    QLabel* detailsLabel_{nullptr};
};

}
