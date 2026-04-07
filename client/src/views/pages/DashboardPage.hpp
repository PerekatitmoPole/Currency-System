#pragma once

#include <QWidget>

class QComboBox;
class QLineEdit;
class QPushButton;

namespace currency::client::views {

class RatesTableWidget;
class HistoryChartWidget;
class AggregationSummaryWidget;

class DashboardPage : public QWidget {
    Q_OBJECT

public:
    DashboardPage(QWidget* parent = nullptr);

    QComboBox* baseCurrencyCombo() const;
    QComboBox* quoteCurrencyCombo() const;
    QPushButton* refreshButton() const;
    QPushButton* loadHistoryButton() const;
    QPushButton* syncButton() const;
    RatesTableWidget* ratesTableWidget() const;
    HistoryChartWidget* historyChartWidget() const;
    AggregationSummaryWidget* aggregationSummaryWidget() const;

private:
    QComboBox* baseCurrencyCombo_{nullptr};
    QComboBox* quoteCurrencyCombo_{nullptr};
    QPushButton* refreshButton_{nullptr};
    QPushButton* loadHistoryButton_{nullptr};
    QPushButton* syncButton_{nullptr};
    RatesTableWidget* ratesTableWidget_{nullptr};
    HistoryChartWidget* historyChartWidget_{nullptr};
    AggregationSummaryWidget* aggregationSummaryWidget_{nullptr};
};

}