#pragma once

#include <QDateTime>
#include <QMap>
#include <QString>

namespace currency::client::dto {

using FieldMap = QMap<QString, QString>;

struct GetRatesRequestDto {
    QString provider;
    QString baseCode;
    QStringList quoteCodes;
};

struct GetHistoryRequestDto {
    QString provider;
    QString baseCode;
    QString quoteCode;
    QDateTime from;
    QDateTime to;
    QString step;
};

struct ConvertRequestDto {
    QString provider;
    QString fromCurrency;
    QString toCurrency;
    double amount{0.0};
};

struct ServerEnvelopeDto {
    QString status;
    FieldMap fields;
    QString errorCode;
    QString errorMessage;
};

}