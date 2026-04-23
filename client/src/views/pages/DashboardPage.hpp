#pragma once

#include <QWidget>

class QComboBox;
class QDateEdit;
class QDoubleSpinBox;
class QLineEdit;
class QPushButton;
class QLabel;
class QString;

namespace currency::client::views {

class RatesTableWidget;
class HistoryChartWidget;
class AggregationSummaryWidget;

class DashboardPage : public QWidget {
    Q_OBJECT

public:
    DashboardPage(QWidget* parent = nullptr);

    QComboBox* sourceCombo() const;
    QComboBox* baseCurrencyCombo() const;
    QComboBox* quoteCurrencyCombo() const;
    QComboBox* stepCombo() const;
    QDateEdit* fromDateEdit() const;
    QDateEdit* toDateEdit() const;
    QDateEdit* dayRateDateEdit() const;
    QDoubleSpinBox* amountSpinBox() const;
    QPushButton* refreshButton() const;
    QPushButton* loadHistoryButton() const;
    QPushButton* dayRateButton() const;
    QPushButton* convertButton() const;
    QPushButton* swapCurrenciesButton() const;
    QPushButton* exportRatesButton() const;
    QPushButton* exportHistoryButton() const;
    QPushButton* saveChartButton() const;
    void setConversionPreview(const QString& text);
    void setDateAggregationPreview(const QString& text);
    RatesTableWidget* ratesTableWidget() const;
    HistoryChartWidget* historyChartWidget() const;
    AggregationSummaryWidget* aggregationSummaryWidget() const;

private:
    QComboBox* sourceCombo_{nullptr};
    QComboBox* baseCurrencyCombo_{nullptr};
    QComboBox* quoteCurrencyCombo_{nullptr};
    QComboBox* stepCombo_{nullptr};
    QDateEdit* fromDateEdit_{nullptr};
    QDateEdit* toDateEdit_{nullptr};
    QDateEdit* dayRateDateEdit_{nullptr};
    QDoubleSpinBox* amountSpinBox_{nullptr};
    QPushButton* refreshButton_{nullptr};
    QPushButton* loadHistoryButton_{nullptr};
    QPushButton* dayRateButton_{nullptr};
    QPushButton* convertButton_{nullptr};
    QPushButton* swapCurrenciesButton_{nullptr};
    QPushButton* exportRatesButton_{nullptr};
    QPushButton* exportHistoryButton_{nullptr};
    QPushButton* saveChartButton_{nullptr};
    QLabel* conversionPreviewLabel_{nullptr};
    QLabel* dateAggregationLabel_{nullptr};
    RatesTableWidget* ratesTableWidget_{nullptr};
    HistoryChartWidget* historyChartWidget_{nullptr};
    AggregationSummaryWidget* aggregationSummaryWidget_{nullptr};
};

}
