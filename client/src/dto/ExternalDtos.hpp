#pragma once

#include "dto/ApiDtos.hpp"

#include <QByteArray>
#include <QDateTime>
#include <QList>
#include <QMap>
#include <QString>

namespace currency::client::dto {

struct ExternalQuoteEntryRawDto {
    QString baseCurrency;
    QString quoteCurrency;
    double rate{0.0};
    QDateTime timestamp;
    double nominal{1.0};
    QMap<QString, QString> metadata;
};

struct ExternalRateRawDto {
    ApiSource source{ApiSource::Frankfurter};
    QString requestedBaseCurrency;
    QStringList requestedQuoteCurrencies;
    QString payloadFormat;
    QByteArray rawPayload;
    QList<ExternalQuoteEntryRawDto> entries;
};

struct ExternalHistoryPointRawDto {
    QString baseCurrency;
    QString quoteCurrency;
    double rate{0.0};
    QDateTime timestamp;
    QMap<QString, QString> metadata;
};

struct ExternalHistoryRawDto {
    ApiSource source{ApiSource::Frankfurter};
    QString requestedBaseCurrency;
    QString requestedQuoteCurrency;
    QString payloadFormat;
    QByteArray rawPayload;
    QList<ExternalHistoryPointRawDto> points;
};

}