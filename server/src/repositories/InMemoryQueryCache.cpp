#include "repositories/InMemoryQueryCache.hpp"

#include <mutex>

namespace currency::repositories {

bool InMemoryQueryCache::tryGet(const std::string& key, std::string& payload) const {
    std::shared_lock lock(mutex_);
    const auto iterator = entries_.find(key);
    if (iterator == entries_.end()) {
        return false;
    }

    if (std::chrono::steady_clock::now() > iterator->second.expiresAt) {
        return false;
    }

    payload = iterator->second.payload;
    return true;
}

bool InMemoryQueryCache::tryGetStale(const std::string& key, std::string& payload) const {
    std::shared_lock lock(mutex_);
    const auto iterator = entries_.find(key);
    if (iterator == entries_.end()) {
        return false;
    }

    payload = iterator->second.payload;
    return true;
}

void InMemoryQueryCache::put(const std::string& key, std::string payload, const std::chrono::seconds ttl) {
    std::unique_lock lock(mutex_);
    entries_[key] = CacheEntry{std::move(payload), std::chrono::steady_clock::now() + ttl};
}

void InMemoryQueryCache::invalidateAll() {
    std::unique_lock lock(mutex_);
    entries_.clear();
}

}
