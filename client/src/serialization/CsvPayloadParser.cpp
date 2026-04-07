#include "serialization/CsvPayloadParser.hpp"

namespace currency::client::serialization {

namespace {

QStringList splitCsvLine(const QString& line) {
    QStringList fields;
    QString current;
    bool insideQuotes = false;

    for (const QChar character : line) {
        if (character == '"') {
            insideQuotes = !insideQuotes;
            continue;
        }

        if (character == ',' && !insideQuotes) {
            fields.push_back(current.trimmed());
            current.clear();
            continue;
        }

        current.append(character);
    }

    fields.push_back(current.trimmed());
    return fields;
}

}

common::Result<QList<QStringList>> CsvPayloadParser::parse(const QByteArray& payload) const {
    QList<QStringList> rows;
    const auto lines = QString::fromUtf8(payload).split('\n', Qt::SkipEmptyParts);
    for (const auto& rawLine : lines) {
        rows.push_back(splitCsvLine(rawLine.trimmed()));
    }

    return common::Result<QList<QStringList>>::success(rows);
}

}