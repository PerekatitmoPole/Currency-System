#include "gateways/FixerClient.hpp"

#include "common/Errors.hpp"

namespace currency::client::gateways {

FixerClient::FixerClient(QNetworkAccessManager& networkManager, QObject* parent)
    : AbstractApiSourceClient(dto::ApiSource::Fixer, networkManager, parent) {}

common::Result<dto::ExternalRateRawDto> FixerClient::fetchLatest(const QString&, const QStringList&) {
    return common::Result<dto::ExternalRateRawDto>::failure(common::Errors::unsupportedError(
        "Fixer gateway is scaffolded but not implemented yet", "Add provider-specific request and mapper logic."));
}

common::Result<dto::ExternalHistoryRawDto> FixerClient::fetchHistory(const QString&, const QString&, const QDate&, const QDate&) {
    return common::Result<dto::ExternalHistoryRawDto>::failure(common::Errors::unsupportedError(
        "Fixer history gateway is scaffolded but not implemented yet", "Add provider-specific request and mapper logic."));
}

}