#include "common/Exceptions.hpp"
#include "common/TimeUtils.hpp"
#include "services/ConversionService.hpp"
#include "services/CurrencyQueryService.hpp"
#include "services/HistoryQueryService.hpp"
#include "services/IngestionService.hpp"
#include "services/QuoteQueryService.hpp"
#include "TestHelpers.hpp"

#include <gtest/gtest.h>

namespace currency::tests {

class CatalogProvider final : public providers::MarketDataProvider {
public:
    std::string key() const override {
        return "catalog";
    }

    std::vector<providers::QuoteSnapshot> fetchLatest(
        const std::string&,
        const std::vector<std::string>&) const override {
        return {};
    }

    std::vector<domain::HistoryPoint> fetchHistory(
        const std::string&,
        const std::string&,
        std::chrono::system_clock::time_point,
        std::chrono::system_clock::time_point) const override {
        return {};
    }

    std::vector<domain::Currency> fetchCurrencies() const override {
        return {domain::Currency{"AUD", "Australian Dollar", 2}};
    }
};

TEST(IngestionServiceTests, StoresLatestQuoteAndRegistersCurrencies) {
    TestAppContext context;

    const dto::UpdateQuotesRequestDto request{
        .provider = "ECB",
        .batchTimestamp = "2026-03-18T12:00:00Z",
        .quotes = {
            dto::QuoteSnapshotDto{
                .baseCode = "EUR",
                .baseName = "Euro",
                .quoteCode = "USD",
                .quoteName = "US Dollar",
                .rate = 1.09,
                .sourceTimestamp = "2026-03-18T12:00:00Z",
            },
        },
    };

    const auto result = context.ingestionService.ingest(request);

    EXPECT_EQ(result.provider, "ecb");
    EXPECT_EQ(result.acceptedCount, 1U);
    EXPECT_TRUE(context.currencyRepository.exists("EUR"));
    EXPECT_TRUE(context.currencyRepository.exists("USD"));

    const auto quote = context.quoteRepository.tryGet("ECB", "EUR", "USD");
    ASSERT_TRUE(quote.has_value());
    EXPECT_DOUBLE_EQ(quote->rate, 1.09);
}

TEST(IngestionServiceTests, StoresMultipleQuotesInSingleBatch) {
    TestAppContext context;
    const dto::UpdateQuotesRequestDto request{
        .provider = "ECB",
        .batchTimestamp = "2026-03-18T12:00:00Z",
        .quotes = {
            dto::QuoteSnapshotDto{"EUR", "Euro", "USD", "US Dollar", 1.09, "2026-03-18T12:00:00Z"},
            dto::QuoteSnapshotDto{"EUR", "Euro", "GBP", "Pound Sterling", 0.86, "2026-03-18T12:00:00Z"},
        },
    };

    const auto result = context.ingestionService.ingest(request);
    EXPECT_EQ(result.acceptedCount, 2U);
    EXPECT_TRUE(context.quoteRepository.tryGet("ECB", "EUR", "USD").has_value());
    EXPECT_TRUE(context.quoteRepository.tryGet("ECB", "EUR", "GBP").has_value());
}

TEST(IngestionServiceTests, AppendsHistoryPoints) {
    TestAppContext context;
    const dto::UpdateQuotesRequestDto request{
        .provider = "ECB",
        .batchTimestamp = "2026-03-18T12:00:00Z",
        .quotes = {dto::QuoteSnapshotDto{"EUR", "Euro", "USD", "US Dollar", 1.09, "2026-03-18T12:00:00Z"}},
    };

    context.ingestionService.ingest(request);
    const auto history = context.historyRepository.query(
        "ECB", "EUR", "USD", common::fromIsoString("2026-03-18T11:00:00Z"), common::fromIsoString("2026-03-18T13:00:00Z"));
    ASSERT_EQ(history.size(), 1U);
    EXPECT_DOUBLE_EQ(history.front().rate, 1.09);
}

TEST(IngestionServiceTests, RejectsNegativeRate) {
    TestAppContext context;
    const dto::UpdateQuotesRequestDto request{
        .provider = "ECB",
        .batchTimestamp = "2026-03-18T12:00:00Z",
        .quotes = {dto::QuoteSnapshotDto{"EUR", "Euro", "USD", "US Dollar", -1.09, "2026-03-18T12:00:00Z"}},
    };

    EXPECT_THROW(context.ingestionService.ingest(request), common::ValidationError);
}

TEST(IngestionServiceTests, RejectsEmptyProvider) {
    TestAppContext context;
    const dto::UpdateQuotesRequestDto request{
        .provider = "",
        .batchTimestamp = "2026-03-18T12:00:00Z",
        .quotes = {dto::QuoteSnapshotDto{"EUR", "Euro", "USD", "US Dollar", 1.09, "2026-03-18T12:00:00Z"}},
    };

    EXPECT_THROW(context.ingestionService.ingest(request), common::ValidationError);
}

TEST(IngestionServiceTests, RejectsEmptyQuotesCollection) {
    TestAppContext context;
    const dto::UpdateQuotesRequestDto request{
        .provider = "ECB",
        .batchTimestamp = "2026-03-18T12:00:00Z",
        .quotes = {},
    };

    EXPECT_THROW(context.ingestionService.ingest(request), common::ValidationError);
}

TEST(CurrencyQueryServiceTests, ReturnsAllCurrenciesSortedByCode) {
    TestAppContext context;
    context.currencyRepository.upsert(domain::Currency{"AUD", "Australian Dollar", 2});
    const auto result = context.currencyQueryService.getCurrencies();
    ASSERT_FALSE(result.currencies.empty());
    EXPECT_EQ(result.currencies.front().code, "AUD");
}

TEST(CurrencyQueryServiceTests, ContainsBuiltInCodes) {
    TestAppContext context;
    const auto result = context.currencyQueryService.getCurrencies();
    bool hasUsd = false;
    bool hasJpy = false;
    for (const auto& currency : result.currencies) {
        hasUsd = hasUsd || currency.code == "USD";
        hasJpy = hasJpy || currency.code == "JPY";
    }
    EXPECT_TRUE(hasUsd);
    EXPECT_TRUE(hasJpy);
}

TEST(MarketDataRefreshServiceTests, RefreshCurrencyCatalogUpsertsProviderCurrencies) {
    TestAppContext context;
    CatalogProvider provider;
    context.marketDataRefreshService.registerProvider(provider);

    const auto count = context.marketDataRefreshService.refreshCurrencyCatalog();

    EXPECT_GE(count, 1U);
    const auto currency = context.currencyRepository.tryGet("AUD");
    ASSERT_TRUE(currency.has_value());
    EXPECT_EQ(currency->name, "Australian Dollar");
}

TEST(QuoteQueryServiceTests, ReturnsRequestedQuotes) {
    TestAppContext context;
    seedQuote(context.quoteRepository, "ECB", "EUR", "USD", 1.09, "2026-03-18T10:00:00Z");

    const dto::GetRatesRequestDto request{.provider = "ECB", .baseCode = "EUR", .quoteCodes = {"USD"}};
    const auto response = context.quoteQueryService.getRates(request);
    ASSERT_EQ(response.quotes.size(), 1U);
    EXPECT_DOUBLE_EQ(response.quotes.front().rate, 1.09);
}

TEST(QuoteQueryServiceTests, ReturnsMultipleRequestedQuotes) {
    TestAppContext context;
    seedQuote(context.quoteRepository, "ECB", "EUR", "USD", 1.09, "2026-03-18T10:00:00Z");
    seedQuote(context.quoteRepository, "ECB", "EUR", "GBP", 0.86, "2026-03-18T10:00:00Z");

    const dto::GetRatesRequestDto request{.provider = "ECB", .baseCode = "EUR", .quoteCodes = {"USD", "GBP"}};
    const auto response = context.quoteQueryService.getRates(request);
    ASSERT_EQ(response.quotes.size(), 2U);
}

TEST(QuoteQueryServiceTests, RejectsUnknownBaseCurrency) {
    TestAppContext context;
    const dto::GetRatesRequestDto request{.provider = "ECB", .baseCode = "ABC", .quoteCodes = {"USD"}};
    EXPECT_THROW(context.quoteQueryService.getRates(request), common::NotFoundError);
}

TEST(QuoteQueryServiceTests, RejectsEmptyQuoteCodes) {
    TestAppContext context;
    const dto::GetRatesRequestDto request{.provider = "ECB", .baseCode = "EUR", .quoteCodes = {}};
    EXPECT_THROW(context.quoteQueryService.getRates(request), common::ValidationError);
}

TEST(QuoteQueryServiceTests, RejectsMissingQuoteInRepository) {
    TestAppContext context;
    const dto::GetRatesRequestDto request{.provider = "ECB", .baseCode = "EUR", .quoteCodes = {"USD"}};
    EXPECT_THROW(context.quoteQueryService.getRates(request), common::NotFoundError);
}

TEST(HistoryQueryServiceTests, AggregatesHistoryByStep) {
    TestAppContext context;
    context.historyRepository.append("ECB", "EUR", "USD", {common::fromIsoString("2026-03-18T10:00:00Z"), 1.01});
    context.historyRepository.append("ECB", "EUR", "USD", {common::fromIsoString("2026-03-18T10:10:00Z"), 1.02});
    context.historyRepository.append("ECB", "EUR", "USD", {common::fromIsoString("2026-03-18T11:30:00Z"), 1.05});

    const dto::GetHistoryRequestDto request{
        .provider = "ECB",
        .baseCode = "EUR",
        .quoteCode = "USD",
        .from = "2026-03-18T10:00:00Z",
        .to = "2026-03-18T12:00:00Z",
        .step = "1h",
    };

    const auto response = context.historyQueryService.getHistory(request);
    EXPECT_EQ(response.points.size(), 2U);
}

TEST(HistoryQueryServiceTests, ReturnsEmptyForUnknownPair) {
    TestAppContext context;
    const dto::GetHistoryRequestDto request{
        .provider = "ECB",
        .baseCode = "EUR",
        .quoteCode = "USD",
        .from = "2026-03-18T10:00:00Z",
        .to = "2026-03-18T12:00:00Z",
        .step = "1h",
    };

    const auto response = context.historyQueryService.getHistory(request);
    EXPECT_TRUE(response.points.empty());
}

TEST(HistoryQueryServiceTests, RejectsInvalidRange) {
    TestAppContext context;
    const dto::GetHistoryRequestDto request{
        .provider = "ECB",
        .baseCode = "EUR",
        .quoteCode = "USD",
        .from = "2026-03-18T12:00:00Z",
        .to = "2026-03-18T10:00:00Z",
        .step = "1h",
    };

    EXPECT_THROW(context.historyQueryService.getHistory(request), common::ValidationError);
}

TEST(HistoryQueryServiceTests, NormalizesCurrencyCodesInResponse) {
    TestAppContext context;
    context.historyRepository.append("ECB", "EUR", "USD", {common::fromIsoString("2026-03-18T10:00:00Z"), 1.01});
    const dto::GetHistoryRequestDto request{
        .provider = "ECB",
        .baseCode = "eur",
        .quoteCode = "usd",
        .from = "2026-03-18T09:00:00Z",
        .to = "2026-03-18T11:00:00Z",
        .step = "1h",
    };

    const auto response = context.historyQueryService.getHistory(request);
    EXPECT_EQ(response.baseCode, "EUR");
    EXPECT_EQ(response.quoteCode, "USD");
}

TEST(ConversionServiceTests, ConvertsUsingStoredRate) {
    TestAppContext context;
    seedQuote(context.quoteRepository, "ECB", "EUR", "USD", 1.20, "2026-03-18T12:00:00Z");

    const dto::ConvertRequestDto request{.provider = "ECB", .fromCurrency = "EUR", .toCurrency = "USD", .amount = 10.0};
    const auto response = context.conversionService.convert(request);
    EXPECT_DOUBLE_EQ(response.result, 12.0);
    EXPECT_DOUBLE_EQ(response.rate, 1.20);
}

TEST(ConversionServiceTests, ReturnsIdentityConversionWhenCurrenciesMatch) {
    TestAppContext context;
    const dto::ConvertRequestDto request{.provider = "ECB", .fromCurrency = "USD", .toCurrency = "USD", .amount = 10.0};
    const auto response = context.conversionService.convert(request);
    EXPECT_DOUBLE_EQ(response.rate, 1.0);
    EXPECT_DOUBLE_EQ(response.result, 10.0);
}

TEST(ConversionServiceTests, RejectsNegativeAmount) {
    TestAppContext context;
    const dto::ConvertRequestDto request{.provider = "ECB", .fromCurrency = "EUR", .toCurrency = "USD", .amount = -1.0};
    EXPECT_THROW(context.conversionService.convert(request), common::ValidationError);
}

TEST(ConversionServiceTests, RejectsZeroAmount) {
    TestAppContext context;
    const dto::ConvertRequestDto request{.provider = "ECB", .fromCurrency = "EUR", .toCurrency = "USD", .amount = 0.0};
    EXPECT_THROW(context.conversionService.convert(request), common::ValidationError);
}

TEST(ConversionServiceTests, ThrowsWhenRateMissing) {
    TestAppContext context;
    const dto::ConvertRequestDto request{.provider = "ECB", .fromCurrency = "EUR", .toCurrency = "USD", .amount = 10.0};
    EXPECT_THROW(context.conversionService.convert(request), common::NotFoundError);
}

TEST(ConversionServiceTests, NormalizesCurrencyCodesBeforeLookup) {
    TestAppContext context;
    seedQuote(context.quoteRepository, "ECB", "EUR", "USD", 1.20, "2026-03-18T12:00:00Z");
    const dto::ConvertRequestDto request{.provider = "ECB", .fromCurrency = "eur", .toCurrency = "usd", .amount = 5.0};
    const auto response = context.conversionService.convert(request);
    EXPECT_EQ(response.fromCurrency, "EUR");
    EXPECT_EQ(response.toCurrency, "USD");
    EXPECT_DOUBLE_EQ(response.result, 6.0);
}

}
