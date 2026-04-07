#pragma once

#include "dto/ApiDtos.hpp"

#include <QDateTime>
#include <QString>

namespace currency::client::dto {

struct NormalizedQuoteDto {
    ApiSource source{ApiSource::Frankfurter};
    QString providerName;
    QString baseCurrency;
    QString quoteCurrency;
    QString baseName;
    QString quoteName;
    double rate{0.0};
    QDateTime timestamp;
};

struct NormalizedHistoryPointDto {
    ApiSource source{ApiSource::Frankfurter};
    QString providerName;
    QString baseCurrency;
    QString quoteCurrency;
    double rate{0.0};
    QDateTime timestamp;
};

}