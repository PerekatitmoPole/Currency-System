#include "repositories/InMemoryQuoteRepository.hpp"

#include "common/Validation.hpp"

#include <mutex>

namespace currency::repositories {

void InMemoryQuoteRepository::upsert(const domain::Quote& quote) {
    auto normalized = quote;
    normalized.key.provider = common::normalizeProviderKey(normalized.key.provider);
    normalized.key.baseCurrency = common::normalizeCurrencyCode(normalized.key.baseCurrency);
    normalized.key.quoteCurrency = common::normalizeCurrencyCode(normalized.key.quoteCurrency);

    std::unique_lock lock(mutex_);
    quotes_[normalized.key.toString()] = std::move(normalized);
}

std::optional<domain::Quote> InMemoryQuoteRepository::tryGet(
    const std::string& provider,
    const std::string& baseCurrency,
    const std::string& quoteCurrency) const {
    const auto normalizedProvider = common::normalizeProviderKey(provider);
    const auto normalizedBase = common::normalizeCurrencyCode(baseCurrency);
    const auto normalizedQuote = common::normalizeCurrencyCode(quoteCurrency);

    std::shared_lock lock(mutex_);
    const auto key = domain::QuoteKey{normalizedProvider, normalizedBase, normalizedQuote}.toString();
    const auto iterator = quotes_.find(key);
    if (iterator == quotes_.end()) {
        return std::nullopt;
    }

    return iterator->second;
}

std::vector<domain::Quote> InMemoryQuoteRepository::findByBase(
    const std::string& provider,
    const std::string& baseCurrency) const {
    const auto normalizedProvider = common::normalizeProviderKey(provider);
    const auto normalizedBase = common::normalizeCurrencyCode(baseCurrency);

    std::shared_lock lock(mutex_);
    std::vector<domain::Quote> result;
    for (const auto& [_, quote] : quotes_) {
        if (quote.key.provider == normalizedProvider && quote.key.baseCurrency == normalizedBase) {
            result.push_back(quote);
        }
    }
    return result;
}

}
