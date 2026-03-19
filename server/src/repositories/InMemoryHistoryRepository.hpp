#pragma once

#include "domain/HistoryPoint.hpp"

#include <chrono>
#include <map>
#include <shared_mutex>
#include <string>
#include <vector>

namespace currency::repositories {

class InMemoryHistoryRepository {
public:
    void append(
        const std::string& provider,
        const std::string& baseCurrency,
        const std::string& quoteCurrency,
        const domain::HistoryPoint& point);

    std::vector<domain::HistoryPoint> query(
        const std::string& provider,
        const std::string& baseCurrency,
        const std::string& quoteCurrency,
        std::chrono::system_clock::time_point from,
        std::chrono::system_clock::time_point to) const;

private:
    static std::string makeKey(
        const std::string& provider,
        const std::string& baseCurrency,
        const std::string& quoteCurrency);

    mutable std::shared_mutex mutex_;
    std::map<std::string, std::vector<domain::HistoryPoint>> history_;
};

}
