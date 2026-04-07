#pragma once

#include "gateways/AbstractApiSourceClient.hpp"
#include "serialization/CbrMapper.hpp"

namespace currency::client::gateways {

class CbrClient : public AbstractApiSourceClient {
public:
    CbrClient(QNetworkAccessManager& networkManager, serialization::CbrMapper& mapper, QObject* parent = nullptr);

    common::Result<dto::ExternalRateRawDto> fetchLatest(
        const QString& requestedBaseCurrency,
        const QStringList& requestedQuoteCurrencies) override;

    common::Result<dto::ExternalHistoryRawDto> fetchHistory(
        const QString& requestedBaseCurrency,
        const QString& requestedQuoteCurrency,
        const QDate& from,
        const QDate& to) override;

private:
    common::Result<QString> resolveTrackedCurrencyId(const QString& trackedCurrencyCode);

    serialization::CbrMapper& mapper_;
};

}