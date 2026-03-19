#include "controllers/HistoryController.hpp"

#include <chrono>

namespace currency::controllers {

HistoryController::HistoryController(
    services::HistoryQueryService& service,
    repositories::InMemoryQueryCache& cache,
    serialization::TextProtocolSerializer& serializer)
    : service_(service), cache_(cache), serializer_(serializer) {}

std::string HistoryController::handle(const dto::FieldMap& payload) const {
    const std::string cacheKey = "get_history|" + serializer_.canonicalize(payload);
    std::string cached;
    if (cache_.tryGet(cacheKey, cached)) {
        return cached;
    }

    const auto request = serializer_.parseGetHistoryRequest(payload);
    const auto response = service_.getHistory(request);
    auto serialized = serializer_.successResponse(serializer_.toFields(response));
    cache_.put(cacheKey, serialized, std::chrono::seconds(30));
    return serialized;
}

}
