#include "services/ApiOrchestratorService.hpp"

#include "common/Errors.hpp"

namespace currency::client::services {

ApiOrchestratorService::ApiOrchestratorService(NormalizationService& normalizationService)
    : normalizationService_(normalizationService) {}

void ApiOrchestratorService::registerClient(gateways::IApiSourceClient& client) {
    clients_[client.source()] = &client;
}

void ApiOrchestratorService::setApiKey(const dto::ApiSource source, const QString& apiKey) {
    if (auto* client = findClient(source)) {
        client->setApiKey(apiKey);
    }
}

common::Result<LatestFetchResult> ApiOrchestratorService::fetchLatest(
    const QList<dto::ApiSource>& selectedSources,
    const QString& requestedBaseCurrency,
    const QStringList& requestedQuoteCurrencies) {
    LatestFetchResult result;

    for (const auto source : selectedSources) {
        auto* client = findClient(source);
        if (client == nullptr) {
            result.warnings.push_back(common::Errors::unsupportedError(
                "Requested API source is not registered", dto::toDisplayName(source)));
            continue;
        }

        const auto raw = client->fetchLatest(requestedBaseCurrency, requestedQuoteCurrencies);
        if (!raw.ok()) {
            result.warnings.push_back(raw.error());
            continue;
        }

        const auto normalized = normalizationService_.normalizeRates(raw.value());
        if (!normalized.ok()) {
            result.warnings.push_back(normalized.error());
            continue;
        }

        result.quotes.append(normalized.value());
    }

    if (result.quotes.isEmpty()) {
        if (!result.warnings.isEmpty()) {
            return common::Result<LatestFetchResult>::failure(result.warnings.first());
        }

        return common::Result<LatestFetchResult>::failure(common::Errors::validationError(
            "No API sources were selected for aggregation", {}));
    }

    return common::Result<LatestFetchResult>::success(result);
}

common::Result<QList<dto::NormalizedHistoryPointDto>> ApiOrchestratorService::fetchHistory(
    const dto::ApiSource source,
    const QString& requestedBaseCurrency,
    const QString& requestedQuoteCurrency,
    const QDate& from,
    const QDate& to) {
    auto* client = findClient(source);
    if (client == nullptr) {
        return common::Result<QList<dto::NormalizedHistoryPointDto>>::failure(common::Errors::unsupportedError(
            "Requested history source is not registered", dto::toDisplayName(source)));
    }

    const auto raw = client->fetchHistory(requestedBaseCurrency, requestedQuoteCurrency, from, to);
    if (!raw.ok()) {
        return common::Result<QList<dto::NormalizedHistoryPointDto>>::failure(raw.error());
    }

    return normalizationService_.normalizeHistory(raw.value());
}

gateways::IApiSourceClient* ApiOrchestratorService::findClient(const dto::ApiSource source) const {
    const auto iterator = clients_.find(source);
    return iterator != clients_.end() ? iterator->second : nullptr;
}

}