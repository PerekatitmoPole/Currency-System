#include "gateways/CurrencyFreaksClient.hpp"

#include "common/Errors.hpp"

namespace currency::client::gateways {

CurrencyFreaksClient::CurrencyFreaksClient(QNetworkAccessManager& networkManager, QObject* parent)
    : AbstractApiSourceClient(dto::ApiSource::CurrencyFreaks, networkManager, parent) {}

common::Result<dto::ExternalRateRawDto> CurrencyFreaksClient::fetchLatest(const QString&, const QStringList&) {
    return common::Result<dto::ExternalRateRawDto>::failure(common::Errors::unsupportedError(
        "CurrencyFreaks gateway is scaffolded but not implemented yet", "Add provider-specific request and mapper logic."));
}

common::Result<dto::ExternalHistoryRawDto> CurrencyFreaksClient::fetchHistory(const QString&, const QString&, const QDate&, const QDate&) {
    return common::Result<dto::ExternalHistoryRawDto>::failure(common::Errors::unsupportedError(
        "CurrencyFreaks history gateway is scaffolded but not implemented yet", "Add provider-specific request and mapper logic."));
}

}