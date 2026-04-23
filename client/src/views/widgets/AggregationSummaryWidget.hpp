#pragma once

#include "dto/UiDtos.hpp"

#include <QWidget>

class QAbstractItemModel;
class QLabel;
class QTableView;

namespace currency::client::views {

class AggregationSummaryWidget : public QWidget {
    Q_OBJECT

public:
    AggregationSummaryWidget(QWidget* parent = nullptr);

    void setSummary(const dto::AggregationSummaryDto& summary);
    void setDayRatePreview(const QString& text);
    void setModel(QAbstractItemModel* model);

private:
    void refreshEmptyState();

    QLabel* headlineLabel_{nullptr};
    QLabel* detailsLabel_{nullptr};
    QLabel* sourcesMetricValue_{nullptr};
    QLabel* samplesMetricValue_{nullptr};
    QLabel* latestMetricValue_{nullptr};
    QLabel* bestMetricValue_{nullptr};
    QLabel* dayRateMetricValue_{nullptr};
    QLabel* emptyLabel_{nullptr};
    QTableView* tableView_{nullptr};
    QAbstractItemModel* model_{nullptr};
};

}
