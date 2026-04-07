#pragma once

#include "gateways/AbstractApiSourceClient.hpp"

namespace currency::client::gateways {

class OpenExchangeRatesClient : public AbstractApiSourceClient {
public:
    explicit OpenExchangeRatesClient(QNetworkAccessManager& networkManager, QObject* parent = nullptr);

    common::Result<dto::ExternalRateRawDto> fetchLatest(const QString&, const QStringList&) override;
    common::Result<dto::ExternalHistoryRawDto> fetchHistory(const QString&, const QString&, const QDate&, const QDate&) override;
};

}