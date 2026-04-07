#pragma once

#include "common/Result.hpp"
#include "dto/ExternalDtos.hpp"

#include <QDate>
#include <QString>
#include <QStringList>

namespace currency::client::gateways {

class IApiSourceClient {
public:
    virtual ~IApiSourceClient() = default;

    virtual dto::ApiSource source() const = 0;
    virtual QString displayName() const = 0;
    virtual bool supportsHistory() const = 0;
    virtual void setApiKey(const QString& apiKey) = 0;

    virtual common::Result<dto::ExternalRateRawDto> fetchLatest(
        const QString& requestedBaseCurrency,
        const QStringList& requestedQuoteCurrencies) = 0;

    virtual common::Result<dto::ExternalHistoryRawDto> fetchHistory(
        const QString& requestedBaseCurrency,
        const QString& requestedQuoteCurrency,
        const QDate& from,
        const QDate& to) = 0;
};

}