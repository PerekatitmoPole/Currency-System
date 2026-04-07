#include "views/widgets/HistoryChartWidget.hpp"

#include <QtCharts/QChart>
#include <QtCharts/QChartView>
#include <QtCharts/QDateTimeAxis>
#include <QtCharts/QLegend>
#include <QtCharts/QLineSeries>
#include <QtCharts/QValueAxis>

#include <QCursor>
#include <QDateTime>
#include <QLabel>
#include <QPainter>
#include <QToolTip>
#include <QVBoxLayout>

namespace currency::client::views {

HistoryChartWidget::HistoryChartWidget(QWidget* parent)
    : QWidget(parent) {
    setObjectName("cardWidget");

    auto* layout = new QVBoxLayout(this);
    layout->setContentsMargins(20, 20, 20, 20);
    layout->setSpacing(12);

    auto* title = new QLabel("History chart", this);
    title->setObjectName("sectionTitleLabel");
    layout->addWidget(title);

    auto* chart = new QtCharts::QChart();
    chart->legend()->hide();
    chart->setBackgroundVisible(false);
    chart->setMargins(QMargins(0, 0, 0, 0));

    chartView_ = new QtCharts::QChartView(chart, this);
    chartView_->setRenderHint(QPainter::Antialiasing);
    layout->addWidget(chartView_, 1);
}

void HistoryChartWidget::setPoints(const QList<dto::ChartPointDto>& points, const QString& title) {
    auto* chart = chartView_->chart();
    chart->removeAllSeries();
    const auto axes = chart->axes();
    for (auto* axis : axes) {
        chart->removeAxis(axis);
    }
    chart->setTitle(title);

    auto* series = new QtCharts::QLineSeries(chart);
    for (const auto& point : points) {
        series->append(point.timestamp.toMSecsSinceEpoch(), point.value);
    }

    connect(series, &QtCharts::QLineSeries::hovered, this, [](const QPointF& point, const bool state) {
        if (!state) {
            QToolTip::hideText();
            return;
        }

        const auto timestamp = QDateTime::fromMSecsSinceEpoch(static_cast<qint64>(point.x()));
        QToolTip::showText(QCursor::pos(), QString("%1\nRate: %2")
            .arg(timestamp.toLocalTime().toString("dd.MM.yyyy HH:mm"))
            .arg(QString::number(point.y(), 'f', 6)));
    });

    chart->addSeries(series);

    auto* axisX = new QtCharts::QDateTimeAxis(chart);
    axisX->setFormat("dd.MM\nHH:mm");
    axisX->setTitleText("Time");

    auto* axisY = new QtCharts::QValueAxis(chart);
    axisY->setTitleText("Rate");
    axisY->setLabelFormat("%.4f");

    chart->addAxis(axisX, Qt::AlignBottom);
    chart->addAxis(axisY, Qt::AlignLeft);
    series->attachAxis(axisX);
    series->attachAxis(axisY);
}

}