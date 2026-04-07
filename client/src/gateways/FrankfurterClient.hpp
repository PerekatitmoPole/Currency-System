#pragma once

#include "gateways/AbstractApiSourceClient.hpp"
#include "serialization/FrankfurterMapper.hpp"

namespace currency::client::gateways {

class FrankfurterClient : public AbstractApiSourceClient {
public:
    FrankfurterClient(QNetworkAccessManager& networkManager, serialization::FrankfurterMapper& mapper, QObject* parent = nullptr);

    common::Result<dto::ExternalRateRawDto> fetchLatest(
        const QString& requestedBaseCurrency,
        const QStringList& requestedQuoteCurrencies) override;

    common::Result<dto::ExternalHistoryRawDto> fetchHistory(
        const QString& requestedBaseCurrency,
        const QString& requestedQuoteCurrency,
        const QDate& from,
        const QDate& to) override;

private:
    serialization::FrankfurterMapper& mapper_;
};

}