#pragma once

#include "common/Errors.hpp"
#include "common/Result.hpp"
#include "dto/ApiDtos.hpp"
#include "dto/NormalizedDtos.hpp"
#include "dto/UiDtos.hpp"
#include "gateways/ServerGateway.hpp"
#include "models/viewmodels/CurrencyViewModel.hpp"

#include <QDate>

#include <optional>

namespace currency::client::services {

struct HistoryFetchResult {
    QList<dto::NormalizedHistoryPointDto> points;
    QList<common::Error> warnings;
};

class ApiOrchestratorService {
public:
    explicit ApiOrchestratorService(gateways::ServerGateway& gateway);

    common::Result<QList<models::CurrencyViewModel>> fetchCurrencies(std::optional<dto::ApiSource> source = std::nullopt);
    common::Result<HistoryFetchResult> fetchHistory(
        dto::ApiSource source,
        const QString& requestedBaseCurrency,
        const QString& requestedQuoteCurrency,
        const QDate& from,
        const QDate& to,
        const QString& step);
    common::Result<dto::ConversionResultDto> convert(
        dto::ApiSource source,
        const QString& fromCurrency,
        const QString& toCurrency,
        double amount);

private:
    common::Result<dto::ServerEnvelopeDto> ensureSuccess(
        common::Result<dto::ServerEnvelopeDto> response,
        const QString& action) const;
    QList<common::Error> extractWarnings(
        const dto::ServerEnvelopeDto& envelope,
        const QString& sourceName) const;

    gateways::ServerGateway& gateway_;
};

}
