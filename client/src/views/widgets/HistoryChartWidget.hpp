#pragma once

#include "dto/UiDtos.hpp"

#include <QtCharts/QChartView>
#include <QtCharts/QDateTimeAxis>
#include <QtCharts/QLineSeries>
#include <QtCharts/QValueAxis>
#include <QLabel>
#include <QWidget>

namespace currency::client::views {

class HistoryChartWidget : public QWidget {
    Q_OBJECT

public:
    HistoryChartWidget(QWidget* parent = nullptr);

    void setPoints(const QList<dto::ChartPointDto>& points, const QString& title);
    bool saveAsPng(const QString& path) const;

private:
    QChartView* chartView_{nullptr};
    QLabel* emptyStateLabel_{nullptr};
};

}
