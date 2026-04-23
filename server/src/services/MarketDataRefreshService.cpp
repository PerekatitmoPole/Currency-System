#include "services/MarketDataRefreshService.hpp"

#include "common/Exceptions.hpp"
#include "common/TimeUtils.hpp"
#include "common/Validation.hpp"

#include <chrono>
#include <exception>

namespace currency::services {

MarketDataRefreshService::MarketDataRefreshService(
    logging::Logger& logger,
    repositories::InMemoryCurrencyRepository& currencyRepository,
    repositories::InMemoryQuoteRepository& quoteRepository,
    repositories::InMemoryHistoryRepository& historyRepository)
    : logger_(logger),
      currencyRepository_(currencyRepository),
      quoteRepository_(quoteRepository),
      historyRepository_(historyRepository) {}

void MarketDataRefreshService::registerProvider(providers::MarketDataProvider& provider) {
    providers_[provider.key()] = &provider;
}

RefreshOutcome MarketDataRefreshService::ensureLatest(
    const std::string& provider,
    const std::string& baseCode,
    const std::vector<std::string>& quoteCodes) const {
    const auto normalizedProvider = common::normalizeProviderKey(provider);
    const auto normalizedBase = common::normalizeCurrencyCode(baseCode);
    std::vector<std::string> normalizedQuotes;
    normalizedQuotes.reserve(quoteCodes.size());
    for (const auto& quoteCode : quoteCodes) {
        normalizedQuotes.push_back(common::normalizeCurrencyCode(quoteCode));
    }

    try {
        const auto snapshots = resolveProvider(normalizedProvider).fetchLatest(normalizedBase, normalizedQuotes);
        for (const auto& snapshot : snapshots) {
            upsertCurrency(snapshot.baseCode);
            upsertCurrency(snapshot.quoteCode);
            quoteRepository_.upsert(domain::Quote{
                .key = domain::QuoteKey{normalizedProvider, snapshot.baseCode, snapshot.quoteCode},
                .rate = snapshot.rate,
                .sourceTimestamp = snapshot.timestamp,
                .receivedAt = std::chrono::system_clock::now(),
            });
            historyRepository_.append(
                normalizedProvider,
                snapshot.baseCode,
                snapshot.quoteCode,
                domain::HistoryPoint{snapshot.timestamp, snapshot.rate});
        }

        logger_.info(
            "provider.refresh",
            "Latest quotes refreshed",
            {
                {"provider", normalizedProvider},
                {"base_code", normalizedBase},
                {"quote_count", std::to_string(normalizedQuotes.size())},
            });
        return {};
    } catch (const common::AppError& error) {
        bool canUseFallback = true;
        for (const auto& quoteCode : normalizedQuotes) {
            if (!quoteRepository_.tryGet(normalizedProvider, normalizedBase, quoteCode).has_value()) {
                canUseFallback = false;
                break;
            }
        }

        if (!canUseFallback) {
            logger_.error(
                "provider.refresh",
                "Latest quotes refresh failed",
                {
                    {"provider", normalizedProvider},
                    {"base_code", normalizedBase},
                    {"error", error.what()},
                });
            throw;
        }

        logger_.warning(
            "provider.refresh",
            "Using cached latest quotes",
            {
                {"provider", normalizedProvider},
                {"base_code", normalizedBase},
                {"error", error.what()},
            });
        return RefreshOutcome{.usedFallback = true, .warning = error.what()};
    }
}

RefreshOutcome MarketDataRefreshService::ensureHistory(
    const std::string& provider,
    const std::string& baseCode,
    const std::string& quoteCode,
    const std::chrono::system_clock::time_point from,
    const std::chrono::system_clock::time_point to) const {
    const auto normalizedProvider = common::normalizeProviderKey(provider);
    const auto normalizedBase = common::normalizeCurrencyCode(baseCode);
    const auto normalizedQuote = common::normalizeCurrencyCode(quoteCode);

    try {
        const auto points = resolveProvider(normalizedProvider).fetchHistory(normalizedBase, normalizedQuote, from, to);
        upsertCurrency(normalizedBase);
        upsertCurrency(normalizedQuote);

        for (const auto& point : points) {
            historyRepository_.append(normalizedProvider, normalizedBase, normalizedQuote, point);
        }

        logger_.info(
            "provider.refresh",
            "History refreshed",
            {
                {"provider", normalizedProvider},
                {"base_code", normalizedBase},
                {"quote_code", normalizedQuote},
                {"point_count", std::to_string(points.size())},
            });
        return {};
    } catch (const common::AppError& error) {
        const auto cachedPoints = historyRepository_.query(normalizedProvider, normalizedBase, normalizedQuote, from, to);
        if (cachedPoints.empty()) {
            logger_.error(
                "provider.refresh",
                "History refresh failed",
                {
                    {"provider", normalizedProvider},
                    {"base_code", normalizedBase},
                    {"quote_code", normalizedQuote},
                    {"error", error.what()},
                });
            throw;
        }

        logger_.warning(
            "provider.refresh",
            "Using cached history",
            {
                {"provider", normalizedProvider},
                {"base_code", normalizedBase},
                {"quote_code", normalizedQuote},
                {"error", error.what()},
            });
        return RefreshOutcome{.usedFallback = true, .warning = error.what()};
    }
}

std::size_t MarketDataRefreshService::refreshCurrencyCatalog() const {
    std::size_t total = 0;
    for (const auto& [providerKey, provider] : providers_) {
        if (provider == nullptr) {
            continue;
        }

        try {
            const auto currencies = provider->fetchCurrencies();
            for (const auto& currency : currencies) {
                upsertCurrency(currency);
            }
            total += currencies.size();
            if (!currencies.empty()) {
                logger_.info(
                    "provider.currencies",
                    "Currency catalog refreshed",
                    {
                        {"provider", providerKey},
                        {"currency_count", std::to_string(currencies.size())},
                    });
            }
        } catch (const common::AppError& error) {
            logger_.warning(
                "provider.currencies",
                "Currency catalog refresh failed",
                {
                    {"provider", providerKey},
                    {"error", error.what()},
                });
        } catch (const std::exception& error) {
            logger_.warning(
                "provider.currencies",
                "Currency catalog refresh failed",
                {
                    {"provider", providerKey},
                    {"error", error.what()},
                });
        }
    }

    return total;
}

std::vector<domain::Currency> MarketDataRefreshService::refreshCurrencyCatalog(const std::string& provider) const {
    const auto normalizedProvider = common::normalizeProviderKey(provider);
    auto& resolvedProvider = resolveProvider(normalizedProvider);
    const auto currencies = resolvedProvider.fetchCurrencies();
    for (const auto& currency : currencies) {
        upsertCurrency(currency);
    }

    logger_.info(
        "provider.currencies",
        "Currency catalog refreshed for provider",
        {
            {"provider", normalizedProvider},
            {"currency_count", std::to_string(currencies.size())},
        });

    return currencies;
}

providers::MarketDataProvider& MarketDataRefreshService::resolveProvider(const std::string& provider) const {
    const auto iterator = providers_.find(provider);
    if (iterator == providers_.end() || iterator->second == nullptr) {
        throw common::NotFoundError("Provider is not supported: " + provider);
    }

    return *iterator->second;
}

void MarketDataRefreshService::upsertCurrency(const std::string& code) const {
    if (currencyRepository_.exists(code)) {
        return;
    }

    currencyRepository_.upsert(domain::Currency{code, code, 2});
}

void MarketDataRefreshService::upsertCurrency(const domain::Currency& currency) const {
    const auto code = common::normalizeCurrencyCode(currency.code);
    if (code.empty()) {
        return;
    }

    currencyRepository_.upsert(domain::Currency{
        .code = code,
        .name = currency.name.empty() ? code : currency.name,
        .minorUnits = currency.minorUnits,
    });
}

}
