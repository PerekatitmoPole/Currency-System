#pragma once

#include "domain/Currency.hpp"
#include "logging/Logger.hpp"
#include "providers/MarketDataProvider.hpp"
#include "repositories/InMemoryCurrencyRepository.hpp"
#include "repositories/InMemoryHistoryRepository.hpp"
#include "repositories/InMemoryQuoteRepository.hpp"

#include <map>
#include <string>

namespace currency::services {

struct RefreshOutcome {
    bool usedFallback{false};
    std::string warning;
};

class MarketDataRefreshService {
public:
    MarketDataRefreshService(
        logging::Logger& logger,
        repositories::InMemoryCurrencyRepository& currencyRepository,
        repositories::InMemoryQuoteRepository& quoteRepository,
        repositories::InMemoryHistoryRepository& historyRepository);

    void registerProvider(providers::MarketDataProvider& provider);

    RefreshOutcome ensureLatest(
        const std::string& provider,
        const std::string& baseCode,
        const std::vector<std::string>& quoteCodes) const;
    RefreshOutcome ensureHistory(
        const std::string& provider,
        const std::string& baseCode,
        const std::string& quoteCode,
        std::chrono::system_clock::time_point from,
        std::chrono::system_clock::time_point to) const;
    std::size_t refreshCurrencyCatalog() const;
    std::vector<domain::Currency> refreshCurrencyCatalog(const std::string& provider) const;

private:
    providers::MarketDataProvider& resolveProvider(const std::string& provider) const;
    void upsertCurrency(const std::string& code) const;
    void upsertCurrency(const domain::Currency& currency) const;

    logging::Logger& logger_;
    repositories::InMemoryCurrencyRepository& currencyRepository_;
    repositories::InMemoryQuoteRepository& quoteRepository_;
    repositories::InMemoryHistoryRepository& historyRepository_;
    std::map<std::string, providers::MarketDataProvider*> providers_;
};

}
