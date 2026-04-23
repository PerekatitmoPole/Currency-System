#include "repositories/InMemoryHistoryRepository.hpp"

#include "common/Validation.hpp"

#include <algorithm>

#include <mutex>

namespace currency::repositories {

void InMemoryHistoryRepository::append(
    const std::string& provider,
    const std::string& baseCurrency,
    const std::string& quoteCurrency,
    const domain::HistoryPoint& point) {
    const auto normalizedProvider = common::normalizeProviderKey(provider);
    const auto normalizedBase = common::normalizeCurrencyCode(baseCurrency);
    const auto normalizedQuote = common::normalizeCurrencyCode(quoteCurrency);

    std::unique_lock lock(mutex_);
    auto& bucket = history_[makeKey(normalizedProvider, normalizedBase, normalizedQuote)];
    const auto existing = std::find_if(bucket.begin(), bucket.end(), [&point](const auto& current) {
        return current.timestamp == point.timestamp;
    });

    if (existing != bucket.end()) {
        existing->rate = point.rate;
    } else {
        bucket.push_back(point);
    }

    std::ranges::sort(bucket, {}, &domain::HistoryPoint::timestamp);
}

std::vector<domain::HistoryPoint> InMemoryHistoryRepository::query(
    const std::string& provider,
    const std::string& baseCurrency,
    const std::string& quoteCurrency,
    const std::chrono::system_clock::time_point from,
    const std::chrono::system_clock::time_point to) const {
    const auto normalizedProvider = common::normalizeProviderKey(provider);
    const auto normalizedBase = common::normalizeCurrencyCode(baseCurrency);
    const auto normalizedQuote = common::normalizeCurrencyCode(quoteCurrency);

    std::shared_lock lock(mutex_);
    std::vector<domain::HistoryPoint> result;
    const auto iterator = history_.find(makeKey(normalizedProvider, normalizedBase, normalizedQuote));
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
