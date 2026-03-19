#include "controllers/CurrencyController.hpp"

#include <chrono>

namespace currency::controllers {

CurrencyController::CurrencyController(
    services::CurrencyQueryService& service,
    repositories::InMemoryQueryCache& cache,
    serialization::TextProtocolSerializer& serializer)
    : service_(service), cache_(cache), serializer_(serializer) {}

std::string CurrencyController::handle(const dto::FieldMap& payload) const {
    const std::string cacheKey = "get_currencies|" + serializer_.canonicalize(payload);
    std::string cached;
    if (cache_.tryGet(cacheKey, cached)) {
        return cached;
    }

    const auto response = service_.getCurrencies();
    auto serialized = serializer_.successResponse(serializer_.toFields(response));
    cache_.put(cacheKey, serialized, std::chrono::seconds(60));
    return serialized;
}

}
