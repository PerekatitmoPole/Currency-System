#pragma once

#include <QWidget>

class QComboBox;
class QDateEdit;
class QPushButton;
class QTableView;

namespace currency::client::views {

class HistoryChartWidget;

class HistoryPage : public QWidget {
    Q_OBJECT

public:
    HistoryPage(QWidget* parent = nullptr);

    QComboBox* sourceCombo() const;
    QComboBox* baseCurrencyCombo() const;
    QComboBox* quoteCurrencyCombo() const;
    QDateEdit* fromDateEdit() const;
    QDateEdit* toDateEdit() const;
    QPushButton* loadButton() const;
    HistoryChartWidget* chartWidget() const;
    QTableView* tableView() const;

private:
    QComboBox* sourceCombo_{nullptr};
    QComboBox* baseCurrencyCombo_{nullptr};
    QComboBox* quoteCurrencyCombo_{nullptr};
    QDateEdit* fromDateEdit_{nullptr};
    QDateEdit* toDateEdit_{nullptr};
    QPushButton* loadButton_{nullptr};
    HistoryChartWidget* chartWidget_{nullptr};
    QTableView* tableView_{nullptr};
};

}