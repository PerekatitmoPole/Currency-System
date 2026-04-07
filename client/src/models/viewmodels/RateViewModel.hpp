#pragma once

#include <QDateTime>
#include <QString>

namespace currency::client::models {

struct RateViewModel {
    QString sourceName;
    QString baseCurrency;
    QString quoteCurrency;
    double rate{0.0};
    QDateTime timestamp;
    QString note;
};

}