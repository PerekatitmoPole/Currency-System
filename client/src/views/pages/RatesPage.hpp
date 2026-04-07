#pragma once

#include <QWidget>

class QComboBox;
class QLineEdit;
class QPushButton;

namespace currency::client::views {

class RatesTableWidget;

class RatesPage : public QWidget {
    Q_OBJECT

public:
    RatesPage(QWidget* parent = nullptr);

    QComboBox* baseCurrencyCombo() const;
    QLineEdit* quoteCurrenciesEdit() const;
    QPushButton* refreshButton() const;
    RatesTableWidget* ratesTableWidget() const;

private:
    QComboBox* baseCurrencyCombo_{nullptr};
    QLineEdit* quoteCurrenciesEdit_{nullptr};
    QPushButton* refreshButton_{nullptr};
    RatesTableWidget* ratesTableWidget_{nullptr};
};

}