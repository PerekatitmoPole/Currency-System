#include "serialization/EcbMapper.hpp"

#include "common/DateTimeUtils.hpp"

#include <QDomElement>

namespace currency::client::serialization {

namespace {

QDateTime timeFromCube(const QDomElement& element) {
    auto parent = element.parentNode().toElement();
    while (!parent.isNull()) {
        if (parent.hasAttribute("time")) {
            return common::DateTimeUtils::parseIsoUtc(parent.attribute("time") + "T00:00:00Z");
        }
        parent = parent.parentNode().toElement();
    }
    return {};
}

}

EcbMapper::EcbMapper(XmlPayloadParser& parser)
    : parser_(parser) {}

common::Result<dto::ExternalRateRawDto> EcbMapper::mapLatest(
    const QByteArray& payload,
    const QString& requestedBaseCurrency,
    const QStringList& requestedQuoteCurrencies) const {
    const auto parsed = parser_.parse(payload);
    if (!parsed.ok()) {
        return common::Result<dto::ExternalRateRawDto>::failure(parsed.error());
    }

    dto::ExternalRateRawDto dto;
    dto.source = dto::ApiSource::Ecb;
    dto.requestedBaseCurrency = requestedBaseCurrency;
    dto.requestedQuoteCurrencies = requestedQuoteCurrencies;
    dto.payloadFormat = "xml";
    dto.rawPayload = payload;

    const auto nodes = parsed.value().elementsByTagName("Cube");
    for (int index = 0; index < nodes.size(); ++index) {
        const auto element = nodes.at(index).toElement();
        if (!element.hasAttribute("currency") || !element.hasAttribute("rate")) {
            continue;
        }

        dto.entries.push_back(dto::ExternalQuoteEntryRawDto{
            .baseCurrency = "EUR",
            .quoteCurrency = element.attribute("currency"),
            .rate = element.attribute("rate").toDouble(),
            .timestamp = timeFromCube(element),
            .nominal = 1.0,
            .metadata = {},
        });
    }

    return common::Result<dto::ExternalRateRawDto>::success(dto);
}

common::Result<dto::ExternalHistoryRawDto> EcbMapper::mapHistory(
    const QByteArray& payload,
    const QString& requestedBaseCurrency,
    const QString& requestedQuoteCurrency) const {
    const auto parsed = parser_.parse(payload);
    if (!parsed.ok()) {
        return common::Result<dto::ExternalHistoryRawDto>::failure(parsed.error());
    }

    dto::ExternalHistoryRawDto dto;
    dto.source = dto::ApiSource::Ecb;
    dto.requestedBaseCurrency = requestedBaseCurrency;
    dto.requestedQuoteCurrency = requestedQuoteCurrency;
    dto.payloadFormat = "xml";
    dto.rawPayload = payload;

    const auto nodes = parsed.value().elementsByTagName("Cube");
    for (int index = 0; index < nodes.size(); ++index) {
        const auto element = nodes.at(index).toElement();
        if (!element.hasAttribute("currency") || !element.hasAttribute("rate")) {
            continue;
        }

        dto.points.push_back(dto::ExternalHistoryPointRawDto{
            .baseCurrency = "EUR",
            .quoteCurrency = element.attribute("currency"),
            .rate = element.attribute("rate").toDouble(),
            .timestamp = timeFromCube(element),
            .metadata = {},
        });
    }

    return common::Result<dto::ExternalHistoryRawDto>::success(dto);
}

}