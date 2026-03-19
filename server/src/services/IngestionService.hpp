#pragma once

#include "dto/UpdateDtos.hpp"
#include "repositories/InMemoryCurrencyRepository.hpp"
#include "repositories/InMemoryHistoryRepository.hpp"
#include "repositories/InMemoryQueryCache.hpp"
#include "repositories/InMemoryQuoteRepository.hpp"

namespace currency::services {

class IngestionService {
public:
    IngestionService(
        repositories::InMemoryCurrencyRepository& currencyRepository,
        repositories::InMemoryQuoteRepository& quoteRepository,
        repositories::InMemoryHistoryRepository& historyRepository,
        repositories::InMemoryQueryCache& queryCache);

        dto::UpdateQuotesResponseDto ingest(const dto::UpdateQuotesRequestDto& request) const;

private:
    repositories::InMemoryCurrencyRepository& currencyRepository_;
    repositories::InMemoryQuoteRepository& quoteRepository_;
    repositories::InMemoryHistoryRepository& historyRepository_;
    repositories::InMemoryQueryCache& queryCache_;
};

}
