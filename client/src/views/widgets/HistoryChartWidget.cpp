#include "views/widgets/HistoryChartWidget.hpp"

#include <QtCharts/QChart>
#include <QtCharts/QLegend>

#include <QBrush>
#include <QColor>
#include <QCursor>
#include <QDateTime>
#include <QLabel>
#include <QPainter>
#include <QPen>
#include <QToolTip>
#include <QVBoxLayout>

#include <algorithm>

namespace currency::client::views {

HistoryChartWidget::HistoryChartWidget(QWidget* parent)
    : QWidget(parent) {
    setObjectName("cardWidget");

    auto* layout = new QVBoxLayout(this);
    layout->setContentsMargins(20, 20, 20, 20);
    layout->setSpacing(12);

    auto* title = new QLabel("График истории", this);
    title->setObjectName("sectionTitleLabel");
    layout->addWidget(title);

    emptyStateLabel_ = new QLabel("Выберите источник, валютную пару и период, затем загрузите историю, чтобы построить график.", this);
    emptyStateLabel_->setObjectName("mutedLabel");
    emptyStateLabel_->setWordWrap(true);
    layout->addWidget(emptyStateLabel_);

    auto* chart = new QChart();
    chart->legend()->hide();
    chart->setBackgroundVisible(false);
    chart->setMargins(QMargins(0, 0, 0, 0));
    chart->setTitleBrush(QBrush(QColor("#16324f")));

    chartView_ = new QChartView(chart, this);
    chartView_->setRenderHint(QPainter::Antialiasing);
    chartView_->setStyleSheet("background: transparent;");
    chartView_->setMinimumHeight(360);
    chartView_->setVisible(false);
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

    emptyStateLabel_->setVisible(points.isEmpty());
    chartView_->setVisible(!points.isEmpty());
    if (points.isEmpty()) {
        return;
    }

    auto* series = new QLineSeries(chart);
    QPen linePen(QColor("#0f7b6c"));
    linePen.setWidth(3);
    series->setPen(linePen);

    for (const auto& point : points) {
        series->append(point.timestamp.toMSecsSinceEpoch(), point.value);
    }

    const auto [minIt, maxIt] = std::minmax_element(points.cbegin(), points.cend(), [](const auto& left, const auto& right) {
        return left.value < right.value;
    });
    const auto [minTimeIt, maxTimeIt] = std::minmax_element(points.cbegin(), points.cend(), [](const auto& left, const auto& right) {
        return left.timestamp < right.timestamp;
    });

    connect(series, &QLineSeries::hovered, this, [](const QPointF& point, const bool state) {
        if (!state) {
            QToolTip::hideText();
            return;
        }

        const auto timestamp = QDateTime::fromMSecsSinceEpoch(static_cast<qint64>(point.x()));
        QToolTip::showText(
            QCursor::pos(),
            QString("%1\nКурс: %2")
                .arg(timestamp.toLocalTime().toString("dd.MM.yyyy HH:mm"))
                .arg(QString::number(point.y(), 'f', 6)));
    });

    chart->addSeries(series);

    auto* axisX = new QDateTimeAxis(chart);
    axisX->setFormat(points.size() > 12 ? "dd.MM\nyy" : "dd.MM HH:mm");
    axisX->setTitleText("Время");
    axisX->setLabelsColor(QColor("#40566e"));
    axisX->setGridLineColor(QColor("#dfe8f2"));
    axisX->setTickCount(std::clamp(points.size(), qsizetype(3), qsizetype(8)));
    if (minTimeIt->timestamp == maxTimeIt->timestamp) {
        axisX->setRange(minTimeIt->timestamp.addSecs(-3600), maxTimeIt->timestamp.addSecs(3600));
    } else {
        axisX->setRange(minTimeIt->timestamp, maxTimeIt->timestamp);
    }

    auto* axisY = new QValueAxis(chart);
    axisY->setTitleText("Курс");
    axisY->setLabelFormat("%.4f");
    axisY->setLabelsColor(QColor("#40566e"));
    axisY->setGridLineColor(QColor("#dfe8f2"));
    const auto minValue = minIt->value;
    const auto maxValue = maxIt->value;
    const auto padding = std::max((maxValue - minValue) * 0.12, std::max(maxValue, 1.0) * 0.01);
    axisY->setRange(minValue - padding, maxValue + padding);
    axisY->applyNiceNumbers();

    chart->addAxis(axisX, Qt::AlignBottom);
    chart->addAxis(axisY, Qt::AlignLeft);
    series->attachAxis(axisX);
    series->attachAxis(axisY);
}

bool HistoryChartWidget::saveAsPng(const QString& path) const {
    return chartView_->grab().save(path, "PNG");
}

}
