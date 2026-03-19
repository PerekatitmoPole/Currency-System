#include "common/TimeUtils.hpp"
#include "repositories/InMemoryCurrencyRepository.hpp"
#include "repositories/InMemoryHistoryRepository.hpp"
#include "repositories/InMemoryQuoteRepository.hpp"
#include "services/HistoryQueryService.hpp"
#include "services/QuoteQueryService.hpp"

#include <gtest/gtest.h>

namespace currency::tests {

TEST(QuoteHistoryTests, ReturnsRequestedQuotes) {
    repositories::InMemoryCurrencyRepository currencyRepository;
    repositories::InMemoryQuoteRepository quoteRepository;

    quoteRepository.upsert(domain::Quote{
        .key = domain::QuoteKey{"ECB", "EUR", "USD"},
        .rate = 1.09,
        .sourceTimestamp = common::fromIsoString("2026-03-18T10:00:00Z"),
        .receivedAt = common::fromIsoString("2026-03-18T10:01:00Z"),
    });

    services::QuoteQueryService service(currencyRepository, quoteRepository);
    const dto::GetRatesRequestDto request{
        .provider = "ECB",
        .baseCode = "EUR",
        .quoteCodes = {"USD"},
    };

    const auto response = service.getRates(request);
    ASSERT_EQ(response.quotes.size(), 1U);
    EXPECT_DOUBLE_EQ(response.quotes.front().rate, 1.09);
}

TEST(QuoteHistoryTests, AggregatesHistoryByStep) {
    repositories::InMemoryHistoryRepository historyRepository;
    historyRepository.append("ECB", "EUR", "USD", {common::fromIsoString("2026-03-18T10:00:00Z"), 1.01});
    historyRepository.append("ECB", "EUR", "USD", {common::fromIsoString("2026-03-18T10:10:00Z"), 1.02});
    historyRepository.append("ECB", "EUR", "USD", {common::fromIsoString("2026-03-18T11:30:00Z"), 1.05});

    services::HistoryQueryService service(historyRepository);
    const dto::GetHistoryRequestDto request{
        .provider = "ECB",
        .baseCode = "EUR",
        .quoteCode = "USD",
        .from = "2026-03-18T10:00:00Z",
        .to = "2026-03-18T12:00:00Z",
        .step = "1h",
    };

    const auto response = service.getHistory(request);
    EXPECT_EQ(response.points.size(), 2U);
}

}
