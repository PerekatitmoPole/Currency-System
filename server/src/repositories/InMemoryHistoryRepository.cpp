#include "repositories/InMemoryHistoryRepository.hpp"

#include <algorithm>

#include <mutex>

namespace currency::repositories {

void InMemoryHistoryRepository::append(
    const std::string& provider,
    const std::string& baseCurrency,
    const std::string& quoteCurrency,
    const domain::HistoryPoint& point) {
    std::unique_lock lock(mutex_);
    auto& bucket = history_[makeKey(provider, baseCurrency, quoteCurrency)];
    bucket.push_back(point);
    std::ranges::sort(bucket, {}, &domain::HistoryPoint::timestamp);
}

std::vector<domain::HistoryPoint> InMemoryHistoryRepository::query(
    const std::string& provider,
    const std::string& baseCurrency,
    const std::string& quoteCurrency,
    const std::chrono::system_clock::time_point from,
    const std::chrono::system_clock::time_point to) const {
    std::shared_lock lock(mutex_);
    std::vector<domain::HistoryPoint> result;
    const auto iterator = history_.find(makeKey(provider, baseCurrency, quoteCurrency));
    if (iterator == history_.end()) {
        return result;
    }

    for (const auto& point : iterator->second) {
        if (point.timestamp >= from && point.timestamp <= to) {
            result.push_back(point);
        }
    }
    return result;
}

std::string InMemoryHistoryRepository::makeKey(
    const std::string& provider,
    const std::string& baseCurrency,
    const std::string& quoteCurrency) {
    return provider + '|' + baseCurrency + '|' + quoteCurrency;
}

}
