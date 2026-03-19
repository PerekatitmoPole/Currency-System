#include "repositories/InMemoryCurrencyRepository.hpp"

#include <mutex>

namespace currency::repositories {

InMemoryCurrencyRepository::InMemoryCurrencyRepository() {
    currencies_.emplace("USD", domain::Currency{"USD", "US Dollar", 2});
    currencies_.emplace("EUR", domain::Currency{"EUR", "Euro", 2});
    currencies_.emplace("GBP", domain::Currency{"GBP", "Pound Sterling", 2});
    currencies_.emplace("RUB", domain::Currency{"RUB", "Russian Ruble", 2});
    currencies_.emplace("CNY", domain::Currency{"CNY", "Chinese Yuan", 2});
    currencies_.emplace("JPY", domain::Currency{"JPY", "Japanese Yen", 0});
    currencies_.emplace("CHF", domain::Currency{"CHF", "Swiss Franc", 2});
}

void InMemoryCurrencyRepository::upsert(const domain::Currency& currency) {
    std::unique_lock lock(mutex_);
    currencies_[currency.code] = currency;
}

std::vector<domain::Currency> InMemoryCurrencyRepository::list() const {
    std::shared_lock lock(mutex_);
    std::vector<domain::Currency> result;
    result.reserve(currencies_.size());
    for (const auto& [_, currency] : currencies_) {
        result.push_back(currency);
    }
    return result;
}

bool InMemoryCurrencyRepository::exists(const std::string& code) const {
    std::shared_lock lock(mutex_);
    return currencies_.contains(code);
}

std::optional<domain::Currency> InMemoryCurrencyRepository::tryGet(const std::string& code) const {
    std::shared_lock lock(mutex_);
    const auto iterator = currencies_.find(code);
    if (iterator == currencies_.end()) {
        return std::nullopt;
    }

    return iterator->second;
}

}
