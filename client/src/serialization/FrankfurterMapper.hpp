#pragma once

#include "common/Result.hpp"
#include "dto/ExternalDtos.hpp"
#include "serialization/JsonPayloadParser.hpp"

namespace currency::client::serialization {


class FrankfurterMapper {
public:
    FrankfurterMapper(JsonPayloadParser& parser);

    common::Result<dto::ExternalRateRawDto> mapLatest(
        const QByteArray& payload,
        const QString& requestedBaseCurrency,
        const QStringList& requestedQuoteCurrencies) const;

    common::Result<dto::ExternalHistoryRawDto> mapHistory(
        const QByteArray& payload,
        const QString& requestedBaseCurrency,
        const QString& requestedQuoteCurrency) const;

private:
    JsonPayloadParser& parser_;
};

}