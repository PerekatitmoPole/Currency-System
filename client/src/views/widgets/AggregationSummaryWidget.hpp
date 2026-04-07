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
    void setModel(QAbstractItemModel* model);

private:
    QLabel* headlineLabel_{nullptr};
    QLabel* detailsLabel_{nullptr};
    QTableView* tableView_{nullptr};
};

}