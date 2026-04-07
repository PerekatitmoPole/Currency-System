#include "gateways/CurrencyApiClient.hpp"

#include "common/Errors.hpp"

namespace currency::client::gateways {

CurrencyApiClient::CurrencyApiClient(QNetworkAccessManager& networkManager, QObject* parent)
    : AbstractApiSourceClient(dto::ApiSource::CurrencyApi, networkManager, parent) {}

common::Result<dto::ExternalRateRawDto> CurrencyApiClient::fetchLatest(const QString&, const QStringList&) {
    return common::Result<dto::ExternalRateRawDto>::failure(common::Errors::unsupportedError(
        "CurrencyAPI gateway is scaffolded but not implemented yet", "Add provider-specific request and mapper logic."));
}

common::Result<dto::ExternalHistoryRawDto> CurrencyApiClient::fetchHistory(const QString&, const QString&, const QDate&, const QDate&) {
    return common::Result<dto::ExternalHistoryRawDto>::failure(common::Errors::unsupportedError(
        "CurrencyAPI history gateway is scaffolded but not implemented yet", "Add provider-specific request and mapper logic."));
}

}