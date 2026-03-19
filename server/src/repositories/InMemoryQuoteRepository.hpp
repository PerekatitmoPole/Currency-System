#pragma once

#include "domain/Quote.hpp"

#include <map>
#include <optional>
#include <shared_mutex>
#include <string>
#include <vector>

namespace currency::repositories {

class InMemoryQuoteRepository {
public:
    void upsert(const domain::Quote& quote);
    std::optional<domain::Quote> tryGet(
        const std::string& provider,
        const std::string& baseCurrency,
        const std::string& quoteCurrency) const;
    std::vector<domain::Quote> findByBase(
        const std::string& provider,
        const std::string& baseCurrency) const;

private:
    mutable std::shared_mutex mutex_;
    std::map<std::string, domain::Quote> quotes_;
};

}
