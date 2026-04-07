#pragma once

#include "gateways/AbstractApiSourceClient.hpp"
#include "serialization/EcbMapper.hpp"

namespace currency::client::gateways {

class EcbClient : public AbstractApiSourceClient {
public:
    EcbClient(QNetworkAccessManager& networkManager, serialization::EcbMapper& mapper, QObject* parent = nullptr);

    common::Result<dto::ExternalRateRawDto> fetchLatest(
        const QString& requestedBaseCurrency,
        const QStringList& requestedQuoteCurrencies) override;

    common::Result<dto::ExternalHistoryRawDto> fetchHistory(
        const QString& requestedBaseCurrency,
        const QString& requestedQuoteCurrency,
        const QDate& from,
        const QDate& to) override;

private:
    serialization::EcbMapper& mapper_;
};

}