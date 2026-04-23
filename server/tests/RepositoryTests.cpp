#include "common/TimeUtils.hpp"
#include "repositories/InMemoryCurrencyRepository.hpp"
#include "repositories/InMemoryHistoryRepository.hpp"
#include "repositories/InMemoryQueryCache.hpp"
#include "repositories/InMemoryQuoteRepository.hpp"

#include <gtest/gtest.h>

#include <thread>

namespace currency::tests {

TEST(CurrencyRepositoryTests, HasBuiltInCurrencies) {
    repositories::InMemoryCurrencyRepository repository;
    EXPECT_TRUE(repository.exists("USD"));
    EXPECT_TRUE(repository.exists("JPY"));
}

TEST(CurrencyRepositoryTests, UpsertAddsOrReplacesCurrency) {
    repositories::InMemoryCurrencyRepository repository;
    repository.upsert(domain::Currency{"BTC", "Bitcoin", 8});
    const auto currency = repository.tryGet("BTC");
    ASSERT_TRUE(currency.has_value());
    EXPECT_EQ(currency->name, "Bitcoin");
    EXPECT_EQ(currency->minorUnits, 8);
}

TEST(CurrencyRepositoryTests, TryGetUnknownReturnsNullopt) {
    repositories::InMemoryCurrencyRepository repository;
    EXPECT_FALSE(repository.tryGet("ZZZ").has_value());
}

TEST(CurrencyRepositoryTests, ListReturnsSortedMapOrder) {
    repositories::InMemoryCurrencyRepository repository;
    repository.upsert(domain::Currency{"AUD", "Australian Dollar", 2});
    const auto currencies = repository.list();
    ASSERT_FALSE(currencies.empty());
    EXPECT_EQ(currencies.front().code, "AUD");
}

TEST(QuoteRepositoryTests, TryGetReturnsStoredQuote) {
    repositories::InMemoryQuoteRepository repository;
    repository.upsert(domain::Quote{
        .key = domain::QuoteKey{"ECB", "EUR", "USD"},
        .rate = 1.2,
        .sourceTimestamp = common::fromIsoString("2026-03-19T10:00:00Z"),
        .receivedAt = common::fromIsoString("2026-03-19T10:00:00Z"),
    });

    const auto quote = repository.tryGet("ECB", "EUR", "USD");
    ASSERT_TRUE(quote.has_value());
    EXPECT_DOUBLE_EQ(quote->rate, 1.2);
}

TEST(QuoteRepositoryTests, UpsertReplacesExistingQuote) {
    repositories::InMemoryQuoteRepository repository;
    repository.upsert(domain::Quote{
        .key = domain::QuoteKey{"ECB", "EUR", "USD"},
        .rate = 1.1,
        .sourceTimestamp = common::fromIsoString("2026-03-19T10:00:00Z"),
        .receivedAt = common::fromIsoString("2026-03-19T10:00:00Z"),
    });
    repository.upsert(domain::Quote{
        .key = domain::QuoteKey{"ECB", "EUR", "USD"},
        .rate = 1.3,
        .sourceTimestamp = common::fromIsoString("2026-03-19T11:00:00Z"),
        .receivedAt = common::fromIsoString("2026-03-19T11:00:00Z"),
    });

    const auto quote = repository.tryGet("ECB", "EUR", "USD");
    ASSERT_TRUE(quote.has_value());
    EXPECT_DOUBLE_EQ(quote->rate, 1.3);
}

TEST(QuoteRepositoryTests, FindByBaseFiltersProviderAndBaseCurrency) {
    repositories::InMemoryQuoteRepository repository;
    repository.upsert(domain::Quote{.key = domain::QuoteKey{"ECB", "EUR", "USD"}, .rate = 1.1,
                                   .sourceTimestamp = common::fromIsoString("2026-03-19T10:00:00Z"),
                                   .receivedAt = common::fromIsoString("2026-03-19T10:00:00Z")});
    repository.upsert(domain::Quote{.key = domain::QuoteKey{"ECB", "EUR", "GBP"}, .rate = 0.8,
                                   .sourceTimestamp = common::fromIsoString("2026-03-19T10:00:00Z"),
                                   .receivedAt = common::fromIsoString("2026-03-19T10:00:00Z")});
    repository.upsert(domain::Quote{.key = domain::QuoteKey{"OTHER", "EUR", "CHF"}, .rate = 0.9,
                                   .sourceTimestamp = common::fromIsoString("2026-03-19T10:00:00Z"),
                                   .receivedAt = common::fromIsoString("2026-03-19T10:00:00Z")});

    const auto quotes = repository.findByBase("ECB", "EUR");
    ASSERT_EQ(quotes.size(), 2U);
}

TEST(QuoteRepositoryTests, NormalizesKeysOnInsertAndLookup) {
    repositories::InMemoryQuoteRepository repository;
    repository.upsert(domain::Quote{
        .key = domain::QuoteKey{"ECB", "eur", "usd"},
        .rate = 1.2,
        .sourceTimestamp = common::fromIsoString("2026-03-19T10:00:00Z"),
        .receivedAt = common::fromIsoString("2026-03-19T10:00:00Z"),
    });

    const auto quote = repository.tryGet("ecb", "EUR", "USD");
    ASSERT_TRUE(quote.has_value());
    EXPECT_EQ(quote->key.provider, "ecb");
    EXPECT_EQ(quote->key.baseCurrency, "EUR");
    EXPECT_EQ(quote->key.quoteCurrency, "USD");
}

TEST(HistoryRepositoryTests, AppendSortsPointsByTimestamp) {
    repositories::InMemoryHistoryRepository repository;
    repository.append("ECB", "EUR", "USD", {common::fromIsoString("2026-03-19T11:00:00Z"), 1.2});
    repository.append("ECB", "EUR", "USD", {common::fromIsoString("2026-03-19T10:00:00Z"), 1.1});

    const auto result = repository.query(
        "ECB", "EUR", "USD", common::fromIsoString("2026-03-19T09:00:00Z"), common::fromIsoString("2026-03-19T12:00:00Z"));
    ASSERT_EQ(result.size(), 2U);
    EXPECT_LT(result[0].timestamp, result[1].timestamp);
}

TEST(HistoryRepositoryTests, QueryFiltersByRangeInclusively) {
    repositories::InMemoryHistoryRepository repository;
    repository.append("ECB", "EUR", "USD", {common::fromIsoString("2026-03-19T10:00:00Z"), 1.1});
    repository.append("ECB", "EUR", "USD", {common::fromIsoString("2026-03-19T11:00:00Z"), 1.2});
    repository.append("ECB", "EUR", "USD", {common::fromIsoString("2026-03-19T12:00:00Z"), 1.3});

    const auto result = repository.query(
        "ECB", "EUR", "USD", common::fromIsoString("2026-03-19T11:00:00Z"), common::fromIsoString("2026-03-19T12:00:00Z"));
    ASSERT_EQ(result.size(), 2U);
    EXPECT_DOUBLE_EQ(result.front().rate, 1.2);
    EXPECT_DOUBLE_EQ(result.back().rate, 1.3);
}

TEST(HistoryRepositoryTests, QueryUnknownPairReturnsEmptyVector) {
    repositories::InMemoryHistoryRepository repository;
    const auto result = repository.query(
        "ECB", "EUR", "USD", common::fromIsoString("2026-03-19T11:00:00Z"), common::fromIsoString("2026-03-19T12:00:00Z"));
    EXPECT_TRUE(result.empty());
}

TEST(QueryCacheTests, PutAndTryGetReturnsPayload) {
    repositories::InMemoryQueryCache cache;
    cache.put("key", "payload", std::chrono::seconds(60));
    std::string value;
    EXPECT_TRUE(cache.tryGet("key", value));
    EXPECT_EQ(value, "payload");
}

TEST(QueryCacheTests, ExpiredEntryIsNotReturned) {
    repositories::InMemoryQueryCache cache;
    cache.put("key", "payload", std::chrono::seconds(0));
    std::this_thread::sleep_for(std::chrono::milliseconds(1));
    std::string value;
    EXPECT_FALSE(cache.tryGet("key", value));
}

TEST(QueryCacheTests, InvalidateAllClearsEntries) {
    repositories::InMemoryQueryCache cache;
    cache.put("key", "payload", std::chrono::seconds(60));
    cache.invalidateAll();
    std::string value;
    EXPECT_FALSE(cache.tryGet("key", value));
}

TEST(QueryCacheTests, ExpiredEntryCanStillBeReadAsStale) {
    repositories::InMemoryQueryCache cache;
    cache.put("key", "payload", std::chrono::seconds(0));
    std::this_thread::sleep_for(std::chrono::milliseconds(1));
    std::string value;
    EXPECT_TRUE(cache.tryGetStale("key", value));
    EXPECT_EQ(value, "payload");
}

}
