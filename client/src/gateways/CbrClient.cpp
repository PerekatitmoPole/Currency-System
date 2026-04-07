#include "gateways/CbrClient.hpp"

#include "common/DateTimeUtils.hpp"
#include "common/Errors.hpp"

#include <QUrlQuery>

namespace currency::client::gateways {

CbrClient::CbrClient(
    QNetworkAccessManager& networkManager,
    serialization::CbrMapper& mapper,
    QObject* parent)
    : AbstractApiSourceClient(dto::ApiSource::Cbr, networkManager, parent),
      mapper_(mapper) {}

common::Result<dto::ExternalRateRawDto> CbrClient::fetchLatest(
    const QString& requestedBaseCurrency,
    const QStringList& requestedQuoteCurrencies) {
    const auto response = performGet(QUrl("https://www.cbr.ru/scripts/XML_daily.asp"));
    if (!response.ok()) {
        return common::Result<dto::ExternalRateRawDto>::failure(response.error());
    }

    return mapper_.mapLatest(response.value(), requestedBaseCurrency, requestedQuoteCurrencies);
}

common::Result<dto::ExternalHistoryRawDto> CbrClient::fetchHistory(
    const QString& requestedBaseCurrency,
    const QString& requestedQuoteCurrency,
    const QDate& from,
    const QDate& to) {
    if (requestedBaseCurrency != "RUB" && requestedQuoteCurrency != "RUB") {
        return common::Result<dto::ExternalHistoryRawDto>::failure(common::Errors::unsupportedError(
            "ЦБ РФ history is currently supported only for pairs with RUB",
            "Select RUB as base or quote currency for CBR history requests"));
    }

    const QString trackedCurrency = requestedBaseCurrency == "RUB" ? requestedQuoteCurrency : requestedBaseCurrency;
    const auto idResult = resolveTrackedCurrencyId(trackedCurrency);
    if (!idResult.ok()) {
        return common::Result<dto::ExternalHistoryRawDto>::failure(idResult.error());
    }

    QUrl url("https://www.cbr.ru/scripts/XML_dynamic.asp");
    QUrlQuery query;
    query.addQueryItem("date_req1", common::DateTimeUtils::toCbrDate(from));
    query.addQueryItem("date_req2", common::DateTimeUtils::toCbrDate(to));
    query.addQueryItem("VAL_NM_RQ", idResult.value());
    url.setQuery(query);

    const auto response = performGet(url);
    if (!response.ok()) {
        return common::Result<dto::ExternalHistoryRawDto>::failure(response.error());
    }

    return mapper_.mapHistory(response.value(), requestedBaseCurrency, requestedQuoteCurrency, trackedCurrency);
}

common::Result<QString> CbrClient::resolveTrackedCurrencyId(const QString& trackedCurrencyCode) {
    const auto response = performGet(QUrl("https://www.cbr.ru/scripts/XML_daily.asp"));
    if (!response.ok()) {
        return common::Result<QString>::failure(response.error());
    }

    const auto mapped = mapper_.mapLatest(response.value(), "RUB", {trackedCurrencyCode});
    if (!mapped.ok()) {
        return common::Result<QString>::failure(mapped.error());
    }

    for (const auto& entry : mapped.value().entries) {
        if (entry.baseCurrency == trackedCurrencyCode) {
            const auto id = entry.metadata.value("value_id");
            if (!id.isEmpty()) {
                return common::Result<QString>::success(id);
            }
        }
    }

    return common::Result<QString>::failure(common::Errors::validationError(
        QString("Currency %1 is not available in ЦБ РФ catalogue").arg(trackedCurrencyCode), {}));
}

}