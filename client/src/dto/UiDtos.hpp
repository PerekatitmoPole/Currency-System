#pragma once

#include <QDateTime>
#include <QString>
#include <QtGlobal>

namespace currency::client::dto {

struct ConnectionStatusDto {
    QString host;
    quint16 port{0};
    bool connected{false};
    QString message;
};

struct ChartPointDto {
    QDateTime timestamp;
    double value{0.0};
};

struct AggregationSummaryDto {
    QString baseCurrency;
    QString quoteCurrency;
    int sampleCount{0};
    double averageRate{0.0};
    double minimumRate{0.0};
    double maximumRate{0.0};
};

}