#include "serialization/CbrMapper.hpp"

#include "common/DateTimeUtils.hpp"

#include <QDate>

namespace currency::client::serialization {

namespace {

double parseLocalizedDouble(QString value) {
    value.replace(',', '.');
    return value.toDouble();
}

QDateTime parseCbrRootDate(const QString& value) {
    const auto date = QDate::fromString(value, "dd.MM.yyyy");
    return common::DateTimeUtils::parseIsoUtc(date.toString("yyyy-MM-dd") + "T00:00:00Z");
}

}

CbrMapper::CbrMapper(XmlPayloadParser& parser)
    : parser_(parser) {}

common::Result<dto::ExternalRateRawDto> CbrMapper::mapLatest(
    const QByteArray& payload,
    const QString& requestedBaseCurrency,
    const QStringList& requestedQuoteCurrencies) const {
    const auto parsed = parser_.parse(payload);
    if (!parsed.ok()) {
        return common::Result<dto::ExternalRateRawDto>::failure(parsed.error());
    }

    dto::ExternalRateRawDto dto;
    dto.source = dto::ApiSource::Cbr;
    dto.requestedBaseCurrency = requestedBaseCurrency;
    dto.requestedQuoteCurrencies = requestedQuoteCurrencies;
    dto.payloadFormat = "xml";
    dto.rawPayload = payload;

    const auto root = parsed.value().documentElement();
    const auto timestamp = parseCbrRootDate(root.attribute("Date"));

    auto node = root.firstChildElement("Valute");
    while (!node.isNull()) {
        const auto nominal = node.firstChildElement("Nominal").text().toDouble();
        const auto quoteCode = node.firstChildElement("CharCode").text();
        const auto id = node.attribute("ID");
        const auto value = parseLocalizedDouble(node.firstChildElement("Value").text());

        dto::ExternalQuoteEntryRawDto entry;
        entry.baseCurrency = quoteCode;
        entry.quoteCurrency = "RUB";
        entry.rate = nominal > 0.0 ? value / nominal : value;
        entry.timestamp = timestamp;
        entry.nominal = nominal > 0.0 ? nominal : 1.0;
        entry.metadata.insert("value_id", id);
        dto.entries.push_back(entry);
        node = node.nextSiblingElement("Valute");
    }

    dto.entries.push_back(dto::ExternalQuoteEntryRawDto{
        .baseCurrency = "RUB",
        .quoteCurrency = "RUB",
        .rate = 1.0,
        .timestamp = timestamp,
        .nominal = 1.0,
        .metadata = {},
    });

    return common::Result<dto::ExternalRateRawDto>::success(dto);
}

common::Result<dto::ExternalHistoryRawDto> CbrMapper::mapHistory(
    const QByteArray& payload,
    const QString& requestedBaseCurrency,
    const QString& requestedQuoteCurrency,
    const QString& trackedCurrencyCode) const {
    const auto parsed = parser_.parse(payload);
    if (!parsed.ok()) {
        return common::Result<dto::ExternalHistoryRawDto>::failure(parsed.error());
    }

    dto::ExternalHistoryRawDto dto;
    dto.source = dto::ApiSource::Cbr;
    dto.requestedBaseCurrency = requestedBaseCurrency;
    dto.requestedQuoteCurrency = requestedQuoteCurrency;
    dto.payloadFormat = "xml";
    dto.rawPayload = payload;

    const auto root = parsed.value().documentElement();
    auto node = root.firstChildElement("Record");
    while (!node.isNull()) {
        const auto dateValue = node.attribute("Date");
        const auto timestamp = common::DateTimeUtils::parseIsoUtc(QDate::fromString(dateValue, "dd.MM.yyyy").toString("yyyy-MM-dd") + "T00:00:00Z");
        const auto nominal = node.firstChildElement("Nominal").text().toDouble();
        const auto value = parseLocalizedDouble(node.firstChildElement("Value").text());

        dto.points.push_back(dto::ExternalHistoryPointRawDto{
            .baseCurrency = trackedCurrencyCode,
            .quoteCurrency = "RUB",
            .rate = nominal > 0.0 ? value / nominal : value,
            .timestamp = timestamp,
            .metadata = {},
        });
        node = node.nextSiblingElement("Record");
    }

    return common::Result<dto::ExternalHistoryRawDto>::success(dto);
}

}