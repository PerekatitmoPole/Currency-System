#pragma once

#include "domain/Currency.hpp"

#include <map>
#include <optional>
#include <shared_mutex>
#include <string>
#include <vector>

namespace currency::repositories {

class InMemoryCurrencyRepository {
public:
    InMemoryCurrencyRepository();

    void upsert(const domain::Currency& currency);
    std::vector<domain::Currency> list() const;
    bool exists(const std::string& code) const;
    std::optional<domain::Currency> tryGet(const std::string& code) const;

private:
    mutable std::shared_mutex mutex_;
    std::map<std::string, domain::Currency> currencies_;
};

}
