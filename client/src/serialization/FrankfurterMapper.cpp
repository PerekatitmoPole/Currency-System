#include "serialization/FrankfurterMapper.hpp"

#include "common/DateTimeUtils.hpp"
#include "common/Errors.hpp"
#include "dto/ApiDtos.hpp"

namespace currency::client::serialization {

FrankfurterMapper::FrankfurterMapper(JsonPayloadParser& parser)
    : parser_(parser) {}

common::Result<dto::ExternalRateRawDto> FrankfurterMapper::mapLatest(
    const QByteArray& payload,
    const QString& requestedBaseCurrency,
    const QStringList& requestedQuoteCurrencies) const {
    const auto parsed = parser_.parse(payload);
    if (!parsed.ok()) {
        return common::Result<dto::ExternalRateRawDto>::failure(parsed.error());
    }

    const auto object = parsed.value().object();
    const auto base = object.value("base").toString(requestedBaseCurrency);
    const auto timestamp = common::DateTimeUtils::parseIsoUtc(object.value("date").toString() + "T00:00:00Z");

    dto::ExternalRateRawDto dto;
    dto.source = dto::ApiSource::Frankfurter;
    dto.requestedBaseCurrency = requestedBaseCurrency;
    dto.requestedQuoteCurrencies = requestedQuoteCurrencies;
    dto.payloadFormat = "json";
    dto.rawPayload = payload;

    const auto rates = object.value("rates").toObject();
    for (auto iterator = rates.begin(); iterator != rates.end(); ++iterator) {
        dto.entries.push_back(dto::ExternalQuoteEntryRawDto{
            .baseCurrency = base,
            .quoteCurrency = iterator.key(),
            .rate = iterator.value().toDouble(),
            .timestamp = timestamp,
            .nominal = 1.0,
            .metadata = {},
        });
    }

    return common::Result<dto::ExternalRateRawDto>::success(dto);
}

common::Result<dto::ExternalHistoryRawDto> FrankfurterMapper::mapHistory(
    const QByteArray& payload,
    const QString& requestedBaseCurrency,
    const QString& requestedQuoteCurrency) const {
    const auto parsed = parser_.parse(payload);
    if (!parsed.ok()) {
        return common::Result<dto::ExternalHistoryRawDto>::failure(parsed.error());
    }

    const auto object = parsed.value().object();
    const auto base = object.value("base").toString(requestedBaseCurrency);

    dto::ExternalHistoryRawDto dto;
    dto.source = dto::ApiSource::Frankfurter;
    dto.requestedBaseCurrency = requestedBaseCurrency;
    dto.requestedQuoteCurrency = requestedQuoteCurrency;
    dto.payloadFormat = "json";
    dto.rawPayload = payload;

    const auto ratesByDate = object.value("rates").toObject();
    for (auto iterator = ratesByDate.begin(); iterator != ratesByDate.end(); ++iterator) {
        const auto date = iterator.key();
        const auto timestamp = common::DateTimeUtils::parseIsoUtc(date + "T00:00:00Z");
        const auto values = iterator.value().toObject();
        for (auto rateIterator = values.begin(); rateIterator != values.end(); ++rateIterator) {
            dto.points.push_back(dto::ExternalHistoryPointRawDto{
                .baseCurrency = base,
                .quoteCurrency = rateIterator.key(),
                .rate = rateIterator.value().toDouble(),
                .timestamp = timestamp,
                .metadata = {},
            });
        }
    }

    return common::Result<dto::ExternalHistoryRawDto>::success(dto);
}

}