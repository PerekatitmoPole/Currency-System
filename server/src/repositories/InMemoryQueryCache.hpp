#pragma once

#include <chrono>
#include <map>
#include <shared_mutex>
#include <string>

namespace currency::repositories {

class InMemoryQueryCache {
public:
    bool tryGet(const std::string& key, std::string& payload) const;
    void put(const std::string& key, std::string payload, std::chrono::seconds ttl);
    void invalidateAll();

private:
    struct CacheEntry {
        std::string payload;
        std::chrono::steady_clock::time_point expiresAt;
    };

    mutable std::shared_mutex mutex_;
    std::map<std::string, CacheEntry> entries_;
};

}
