#include "gateways/ExchangeRateHostClient.hpp"

#include "common/Errors.hpp"

namespace currency::client::gateways {

ExchangeRateHostClient::ExchangeRateHostClient(QNetworkAccessManager& networkManager, QObject* parent)
    : AbstractApiSourceClient(dto::ApiSource::ExchangeRateHost, networkManager, parent) {}

common::Result<dto::ExternalRateRawDto> ExchangeRateHostClient::fetchLatest(const QString&, const QStringList&) {
    return common::Result<dto::ExternalRateRawDto>::failure(common::Errors::unsupportedError(
        "exchangerate.host gateway is scaffolded but not implemented yet", "Add provider-specific request and mapper logic."));
}

common::Result<dto::ExternalHistoryRawDto> ExchangeRateHostClient::fetchHistory(const QString&, const QString&, const QDate&, const QDate&) {
    return common::Result<dto::ExternalHistoryRawDto>::failure(common::Errors::unsupportedError(
        "exchangerate.host history gateway is scaffolded but not implemented yet", "Add provider-specific request and mapper logic."));
}

}