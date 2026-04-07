#pragma once

#include <QDate>
#include <QDateTime>
#include <QString>

namespace currency::client::common {

class DateTimeUtils {
public:
    static QDateTime parseIsoUtc(const QString& value);
    static QString toIsoUtc(const QDateTime& value);
    static QString toDisplayString(const QDateTime& value);
    static QString toCbrDate(const QDate& value);
};

}