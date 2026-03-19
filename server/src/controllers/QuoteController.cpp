#include "controllers/QuoteController.hpp"

#include <chrono>

namespace currency::controllers {

QuoteController::QuoteController(
    services::QuoteQueryService& service,
    repositories::InMemoryQueryCache& cache,
    serialization::TextProtocolSerializer& serializer)
    : service_(service), cache_(cache), serializer_(serializer) {}

std::string QuoteController::handle(const dto::FieldMap& payload) const {
    const std::string cacheKey = "get_rates|" + serializer_.canonicalize(payload);
    std::string cached;
    if (cache_.tryGet(cacheKey, cached)) {
        return cached;
    }

    const auto request = serializer_.parseGetRatesRequest(payload);
    const auto response = service_.getRates(request);
    auto serialized = serializer_.successResponse(serializer_.toFields(response));
    cache_.put(cacheKey, serialized, std::chrono::seconds(10));
    return serialized;
}

}
