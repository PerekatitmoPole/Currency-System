#pragma once

#include "common/Result.hpp"
#include "dto/ExternalDtos.hpp"
#include "serialization/XmlPayloadParser.hpp"

namespace currency::client::serialization {

class CbrMapper {
public:
    CbrMapper(XmlPayloadParser& parser);

    common::Result<dto::ExternalRateRawDto> mapLatest(
        const QByteArray& payload,
        const QString& requestedBaseCurrency,
        const QStringList& requestedQuoteCurrencies) const;

    common::Result<dto::ExternalHistoryRawDto> mapHistory(
        const QByteArray& payload,
        const QString& requestedBaseCurrency,
        const QString& requestedQuoteCurrency,
        const QString& trackedCurrencyCode) const;

private:
    XmlPayloadParser& parser_;
};

}