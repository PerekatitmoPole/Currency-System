#include "common/DateTimeUtils.hpp"

#include <QTimeZone>

namespace currency::client::common {

QDateTime DateTimeUtils::parseIsoUtc(const QString& value) {
    auto parsed = QDateTime::fromString(value, Qt::ISODate);
    if (parsed.isValid()) {
        return parsed.toUTC();
    }

    parsed = QDateTime::fromString(value, "yyyy-MM-dd");
    if (parsed.isValid()) {
        parsed.setTimeZone(QTimeZone::utc());
        return parsed;
    }

    return {};
}

QString DateTimeUtils::toIsoUtc(const QDateTime& value) {
    return value.toUTC().toString(Qt::ISODate);
}

QString DateTimeUtils::toDisplayString(const QDateTime& value) {
    if (!value.isValid()) {
        return "Нет данных";
    }

    return value.toLocalTime().toString("dd.MM.yyyy HH:mm");
}

QString DateTimeUtils::toCbrDate(const QDate& value) {
    return value.toString("dd/MM/yyyy");
}

}
