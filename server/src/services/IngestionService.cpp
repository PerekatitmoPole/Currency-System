#include "common/Exceptions.hpp"
#include "services/IngestionService.hpp"

#include "common/TimeUtils.hpp"
#include "common/Validation.hpp"
#include "domain/HistoryPoint.hpp"
#include "domain/Quote.hpp"

#include <chrono>

namespace currency::services {

IngestionService::IngestionService(
    repositories::InMemoryCurrencyRepository& currencyRepository,
    repositories::InMemoryQuoteRepository& quoteRepository,
    repositories::InMemoryHistoryRepository& historyRepository,
    repositories::InMemoryQueryCache& queryCache)
    : currencyRepository_(currencyRepository),
      quoteRepository_(quoteRepository),
      historyRepository_(historyRepository),
      queryCache_(queryCache) {}

dto::UpdateQuotesResponseDto IngestionService::ingest(const dto::UpdateQuotesRequestDto& request) const {
    const auto normalizedProvider = common::normalizeProviderKey(request.provider);
    common::requireNotBlank(request.batchTimestamp, "batch timestamp");
    if (request.quotes.empty()) {
        throw common::ValidationError("Quotes collection must not be empty");
    }

    const auto batchTimestamp = common::fromIsoString(request.batchTimestamp);
    std::size_t accepted = 0;

    for (const auto& item : request.quotes) {
        const auto baseCode = common::normalizeCurrencyCode(item.baseCode);
        const auto quoteCode = common::normalizeCurrencyCode(item.quoteCode);
        common::requirePositive(item.rate, "rate");
        const auto sourceTimestamp = common::fromIsoString(item.sourceTimestamp);

        currencyRepository_.upsert(domain::Currency{baseCode, item.baseName.empty() ? baseCode : item.baseName, 2});
        currencyRepository_.upsert(domain::Currency{quoteCode, item.quoteName.empty() ? quoteCode : item.quoteName, 2});

        const domain::Quote quote{
            .key = domain::QuoteKey{normalizedProvider, baseCode, quoteCode},
            .rate = item.rate,
            .sourceTimestamp = sourceTimestamp,
            .receivedAt = batchTimestamp,
        };
        quoteRepository_.upsert(quote);

        historyRepository_.append(
            normalizedProvider,
            baseCode,
            quoteCode,
            domain::HistoryPoint{sourceTimestamp, item.rate});
        ++accepted;
    }

    queryCache_.invalidateAll();

    return dto::UpdateQuotesResponseDto{
        .provider = normalizedProvider,
        .processedAt = common::toIsoString(std::chrono::system_clock::now()),
        .acceptedCount = accepted,
    };
}

}
