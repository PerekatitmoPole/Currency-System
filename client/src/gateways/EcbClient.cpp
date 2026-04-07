#include "gateways/EcbClient.hpp"

namespace currency::client::gateways {

EcbClient::EcbClient(
    QNetworkAccessManager& networkManager,
    serialization::EcbMapper& mapper,
    QObject* parent)
    : AbstractApiSourceClient(dto::ApiSource::Ecb, networkManager, parent),
      mapper_(mapper) {}

common::Result<dto::ExternalRateRawDto> EcbClient::fetchLatest(
    const QString& requestedBaseCurrency,
    const QStringList& requestedQuoteCurrencies) {
    const auto response = performGet(QUrl("https://www.ecb.europa.eu/stats/eurofxref/eurofxref-daily.xml"));
    if (!response.ok()) {
        return common::Result<dto::ExternalRateRawDto>::failure(response.error());
    }

    return mapper_.mapLatest(response.value(), requestedBaseCurrency, requestedQuoteCurrencies);
}

common::Result<dto::ExternalHistoryRawDto> EcbClient::fetchHistory(
    const QString& requestedBaseCurrency,
    const QString& requestedQuoteCurrency,
    const QDate& from,
    const QDate& to) {
    Q_UNUSED(from)
    Q_UNUSED(to)

    const auto response = performGet(QUrl("https://www.ecb.europa.eu/stats/eurofxref/eurofxref-hist.xml"));
    if (!response.ok()) {
        return common::Result<dto::ExternalHistoryRawDto>::failure(response.error());
    }

    return mapper_.mapHistory(response.value(), requestedBaseCurrency, requestedQuoteCurrency);
}

}