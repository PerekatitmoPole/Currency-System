#include "repositories/InMemoryQuoteRepository.hpp"

#include <mutex>

namespace currency::repositories {

void InMemoryQuoteRepository::upsert(const domain::Quote& quote) {
    std::unique_lock lock(mutex_);
    quotes_[quote.key.toString()] = quote;
}

std::optional<domain::Quote> InMemoryQuoteRepository::tryGet(
    const std::string& provider,
    const std::string& baseCurrency,
    const std::string& quoteCurrency) const {
    std::shared_lock lock(mutex_);
    const auto key = domain::QuoteKey{provider, baseCurrency, quoteCurrency}.toString();
    const auto iterator = quotes_.find(key);
    if (iterator == quotes_.end()) {
        return std::nullopt;
    }

    return iterator->second;
}

std::vector<domain::Quote> InMemoryQuoteRepository::findByBase(
    const std::string& provider,
    const std::string& baseCurrency) const {
    std::shared_lock lock(mutex_);
    std::vector<domain::Quote> result;
    for (const auto& [_, quote] : quotes_) {
        if (quote.key.provider == provider && quote.key.baseCurrency == baseCurrency) {
            result.push_back(quote);
        }
    }
    return result;
}

}
