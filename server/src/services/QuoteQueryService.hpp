#pragma once

#include "dto/QueryDtos.hpp"
#include "repositories/InMemoryCurrencyRepository.hpp"
#include "repositories/InMemoryQuoteRepository.hpp"

namespace currency::services {

class QuoteQueryService {
public:
    QuoteQueryService(
        repositories::InMemoryCurrencyRepository& currencyRepository,
        repositories::InMemoryQuoteRepository& quoteRepository);

        dto::GetRatesResponseDto getRates(const dto::GetRatesRequestDto& request) const;

private:
    repositories::InMemoryCurrencyRepository& currencyRepository_;
    repositories::InMemoryQuoteRepository& quoteRepository_;
};

}
