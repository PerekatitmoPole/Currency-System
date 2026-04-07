#include "gateways/AlphaVantageClient.hpp"

#include "common/Errors.hpp"

namespace currency::client::gateways {

AlphaVantageClient::AlphaVantageClient(QNetworkAccessManager& networkManager, QObject* parent)
    : AbstractApiSourceClient(dto::ApiSource::AlphaVantage, networkManager, parent) {}

common::Result<dto::ExternalRateRawDto> AlphaVantageClient::fetchLatest(const QString&, const QStringList&) {
    return common::Result<dto::ExternalRateRawDto>::failure(common::Errors::unsupportedError(
        "Alpha Vantage gateway is scaffolded but not implemented yet", "Add provider-specific request and mapper logic."));
}

common::Result<dto::ExternalHistoryRawDto> AlphaVantageClient::fetchHistory(const QString&, const QString&, const QDate&, const QDate&) {
    return common::Result<dto::ExternalHistoryRawDto>::failure(common::Errors::unsupportedError(
        "Alpha Vantage history gateway is scaffolded but not implemented yet", "Add provider-specific request and mapper logic."));
}

}