#pragma once

#include "dto/UiDtos.hpp"

#include <QWidget>

namespace QtCharts {
class QChartView;
class QLineSeries;
class QDateTimeAxis;
class QValueAxis;
}

namespace currency::client::views {

class HistoryChartWidget : public QWidget {
    Q_OBJECT

public:
    HistoryChartWidget(QWidget* parent = nullptr);

    void setPoints(const QList<dto::ChartPointDto>& points, const QString& title);

private:
    QtCharts::QChartView* chartView_{nullptr};
};

}