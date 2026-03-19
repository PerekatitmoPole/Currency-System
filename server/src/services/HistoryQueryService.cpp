#include "common/Exceptions.hpp"
#include "services/HistoryQueryService.hpp"

#include "common/TimeUtils.hpp"
#include "common/Validation.hpp"

namespace currency::services {

namespace {

std::vector<domain::HistoryPoint> aggregateByStep(
    const std::vector<domain::HistoryPoint>& points,
    const std::chrono::seconds step) {
    if (points.empty()) {
        return {};
    }

    std::vector<domain::HistoryPoint> aggregated;
    aggregated.push_back(points.front());
    auto currentBucketStart = points.front().timestamp;

    for (std::size_t index = 1; index < points.size(); ++index) {
        if (points[index].timestamp - currentBucketStart >= step) {
            aggregated.push_back(points[index]);
            currentBucketStart = points[index].timestamp;
        }
    }

    if (aggregated.back().timestamp != points.back().timestamp) {
        aggregated.push_back(points.back());
    }

    return aggregated;
}

}

HistoryQueryService::HistoryQueryService(repositories::InMemoryHistoryRepository& historyRepository)
    : historyRepository_(historyRepository) {}

dto::GetHistoryResponseDto HistoryQueryService::getHistory(const dto::GetHistoryRequestDto& request) const {
    common::requireNotBlank(request.provider, "provider");
    const auto baseCode = common::normalizeCurrencyCode(request.baseCode);
    const auto quoteCode = common::normalizeCurrencyCode(request.quoteCode);
    const auto from = common::fromIsoString(request.from);
    const auto to = common::fromIsoString(request.to);
    const auto step = common::parseStep(request.step);

    if (from > to) {
        throw common::ValidationError("History interval is invalid: from must be less than or equal to to");
    }

    const auto rawPoints = historyRepository_.query(request.provider, baseCode, quoteCode, from, to);
    const auto aggregated = aggregateByStep(rawPoints, step);

    dto::GetHistoryResponseDto response{
        .provider = request.provider,
        .baseCode = baseCode,
        .quoteCode = quoteCode,
        .from = request.from,
        .to = request.to,
        .step = request.step,
    };

    response.points.reserve(aggregated.size());
    for (const auto& point : aggregated) {
        response.points.push_back(dto::HistoryPointDto{
            .timestamp = common::toIsoString(point.timestamp),
            .rate = point.rate,
        });
    }

    return response;
}

}
