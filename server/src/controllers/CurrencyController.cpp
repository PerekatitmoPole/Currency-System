#include "controllers/CurrencyController.hpp"

#include <chrono>
#include <vector>

namespace currency::controllers {

namespace {

dto::GetCurrenciesResponseDto toCurrencyResponse(const std::vector<domain::Currency>& currencies) {
    dto::GetCurrenciesResponseDto response;
    response.currencies.reserve(currencies.size());
    for (const auto& currency : currencies) {
        response.currencies.push_back(dto::CurrencyDto{
            .code = currency.code,
            .name = currency.name,
            .minorUnits = static_cast<int>(currency.minorUnits),
        });
    }
    return response;
}

}

CurrencyController::CurrencyController(
    services::CurrencyQueryService& service,
    services::MarketDataRefreshService& refreshService,
    repositories::InMemoryQueryCache& cache,
    serialization::TextProtocolSerializer& serializer)
    : service_(service), refreshService_(refreshService), cache_(cache), serializer_(serializer) {}

std::string CurrencyController::handle(const dto::FieldMap& payload) const {
    const std::string cacheKey = "get_currencies|" + serializer_.canonicalize(payload);
    std::string cached;
    if (cache_.tryGet(cacheKey, cached)) {
        return cached;
    }

    dto::GetCurrenciesResponseDto response;
    const auto provider = payload.find("provider");
    if (provider != payload.end() && !provider->second.empty()) {
        response = toCurrencyResponse(refreshService_.refreshCurrencyCatalog(provider->second));
    } else {
        refreshService_.refreshCurrencyCatalog();
        response = service_.getCurrencies();
    }
    auto serialized = serializer_.successResponse(serializer_.toFields(response));
    cache_.put(cacheKey, serialized, std::chrono::seconds(60));
    return serialized;
}

}
