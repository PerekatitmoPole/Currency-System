#include "gateways/FrankfurterClient.hpp"

#include <QUrlQuery>

namespace currency::client::gateways {

FrankfurterClient::FrankfurterClient(
    QNetworkAccessManager& networkManager,
    serialization::FrankfurterMapper& mapper,
    QObject* parent)
    : AbstractApiSourceClient(dto::ApiSource::Frankfurter, networkManager, parent),
      mapper_(mapper) {}

common::Result<dto::ExternalRateRawDto> FrankfurterClient::fetchLatest(
    const QString& requestedBaseCurrency,
    const QStringList& requestedQuoteCurrencies) {
    QUrl url("https://api.frankfurter.dev/v1/latest");
    QUrlQuery query;
    query.addQueryItem("base", requestedBaseCurrency);
    if (!requestedQuoteCurrencies.isEmpty()) {
        query.addQueryItem("symbols", requestedQuoteCurrencies.join(','));
    }
    url.setQuery(query);

    const auto response = performGet(url);
    if (!response.ok()) {
        return common::Result<dto::ExternalRateRawDto>::failure(response.error());
    }

    return mapper_.mapLatest(response.value(), requestedBaseCurrency, requestedQuoteCurrencies);
}

common::Result<dto::ExternalHistoryRawDto> FrankfurterClient::fetchHistory(
    const QString& requestedBaseCurrency,
    const QString& requestedQuoteCurrency,
    const QDate& from,
    const QDate& to) {
    QUrl url(QString("https://api.frankfurter.dev/v1/%1..%2").arg(from.toString("yyyy-MM-dd"), to.toString("yyyy-MM-dd")));
    QUrlQuery query;
    query.addQueryItem("base", requestedBaseCurrency);
    query.addQueryItem("symbols", requestedQuoteCurrency);
    url.setQuery(query);

    const auto response = performGet(url);
    if (!response.ok()) {
        return common::Result<dto::ExternalHistoryRawDto>::failure(response.error());
    }

    return mapper_.mapHistory(response.value(), requestedBaseCurrency, requestedQuoteCurrency);
}

}