#include "controllers/QuoteController.hpp"

#include "common/Exceptions.hpp"

#include <chrono>

namespace currency::controllers {

QuoteController::QuoteController(
    logging::Logger& logger,
    services::MarketDataRefreshService& refreshService,
    services::QuoteQueryService& service,
    repositories::InMemoryQueryCache& cache,
    serialization::TextProtocolSerializer& serializer)
    : logger_(logger),
      refreshService_(refreshService),
      service_(service),
      cache_(cache),
      serializer_(serializer) {}

std::string QuoteController::handle(const dto::FieldMap& payload) const {
    const std::string cacheKey = "get_rates|" + serializer_.canonicalize(payload);
    std::string cached;
    if (cache_.tryGet(cacheKey, cached)) {
        return cached;
    }

    std::string staleCached;
    const bool hasStaleCached = cache_.tryGetStale(cacheKey, staleCached);

    try {
        const auto request = serializer_.parseGetRatesRequest(payload);
        const auto refresh = refreshService_.ensureLatest(request.provider, request.baseCode, request.quoteCodes);
        const auto response = service_.getRates(request);
        auto fields = serializer_.toFields(response);
        if (refresh.usedFallback) {
            fields["stale"] = "true";
            fields["warning"] = refresh.warning;
        }

        auto serialized = serializer_.successResponse(fields);
        cache_.put(cacheKey, serialized, std::chrono::seconds(10));
        return serialized;
    }
    catch (const common::AppError& error) {
        if (!hasStaleCached) {
            throw;
        }

        logger_.warning(
            "cache.stale",
            "Serving stale cached rates",
            {
                {"cache_key", cacheKey},
                {"error", error.what()},
            });
        return staleCached;
    }
}

}
