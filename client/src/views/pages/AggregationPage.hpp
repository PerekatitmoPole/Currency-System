#pragma once

#include <QWidget>

class QPushButton;

namespace currency::client::views {

class AggregationSummaryWidget;

class AggregationPage : public QWidget {
    Q_OBJECT

public:
    AggregationPage(QWidget* parent = nullptr);

    AggregationSummaryWidget* summaryWidget() const;
    QPushButton* syncButton() const;

private:
    AggregationSummaryWidget* summaryWidget_{nullptr};
    QPushButton* syncButton_{nullptr};
};

}