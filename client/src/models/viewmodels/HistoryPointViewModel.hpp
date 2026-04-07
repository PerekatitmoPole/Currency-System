#pragma once

#include <QDateTime>
#include <QString>

namespace currency::client::models {

struct HistoryPointViewModel {
    QString sourceName;
    QString baseCurrency;
    QString quoteCurrency;
    double rate{0.0};
    QDateTime timestamp;
};

}