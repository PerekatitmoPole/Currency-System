#include "controllers/HistoryController.hpp"

#include "common/Exceptions.hpp"
#include "common/TimeUtils.hpp"

#include <chrono>

namespace currency::controllers {

HistoryController::HistoryController(
    logging::Logger& logger,
    services::MarketDataRefreshService& refreshService,
    services::HistoryQueryService& service,
    repositories::InMemoryQueryCache& cache,
    serialization::TextProtocolSerializer& serializer)
    : logger_(logger),
      refreshService_(refreshService),
      service_(service),
      cache_(cache),
      serializer_(serializer) {}

std::string HistoryController::handle(const dto::FieldMap& payload) const {
    const std::string cacheKey = "get_history|" + serializer_.canonicalize(payload);
    std::string cached;
    if (cache_.tryGet(cacheKey, cached)) {
        return cached;
    }

    std::string staleCached;
    const bool hasStaleCached = cache_.tryGetStale(cacheKey, staleCached);

    try {
        const auto request = serializer_.parseGetHistoryRequest(payload);
        const auto refresh = refreshService_.ensureHistory(
            request.provider,
            request.baseCode,
            request.quoteCode,
            common::fromIsoString(request.from),
            common::fromIsoString(request.to));
        const auto response = service_.getHistory(request);
        auto fields = serializer_.toFields(response);
        if (refresh.usedFallback) {
            fields["stale"] = "true";
            fields["warning"] = refresh.warning;
        }

        auto serialized = serializer_.successResponse(fields);
        cache_.put(cacheKey, serialized, std::chrono::seconds(30));
        return serialized;
    }
    catch (const common::AppError& error) {
        if (!hasStaleCached) {
            throw;
        }

        logger_.warning(
            "cache.stale",
            "Serving stale cached history",
            {
                {"cache_key", cacheKey},
                {"error", error.what()},
            });
        return staleCached;
    }
}

}
