#include "gateways/OpenExchangeRatesClient.hpp"

#include "common/Errors.hpp"

namespace currency::client::gateways {

OpenExchangeRatesClient::OpenExchangeRatesClient(QNetworkAccessManager& networkManager, QObject* parent)
    : AbstractApiSourceClient(dto::ApiSource::OpenExchangeRates, networkManager, parent) {}

common::Result<dto::ExternalRateRawDto> OpenExchangeRatesClient::fetchLatest(const QString&, const QStringList&) {
    return common::Result<dto::ExternalRateRawDto>::failure(common::Errors::unsupportedError(
        "Open Exchange Rates gateway is scaffolded but not implemented yet", "Add provider-specific request and mapper logic."));
}

common::Result<dto::ExternalHistoryRawDto> OpenExchangeRatesClient::fetchHistory(const QString&, const QString&, const QDate&, const QDate&) {
    return common::Result<dto::ExternalHistoryRawDto>::failure(common::Errors::unsupportedError(
        "Open Exchange Rates history gateway is scaffolded but not implemented yet", "Add provider-specific request and mapper logic."));
}

}