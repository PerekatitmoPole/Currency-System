#include "services/QuoteQueryService.hpp"

#include "common/Exceptions.hpp"
#include "common/TimeUtils.hpp"
#include "common/Validation.hpp"

namespace currency::services {

QuoteQueryService::QuoteQueryService(
    repositories::InMemoryCurrencyRepository& currencyRepository,
    repositories::InMemoryQuoteRepository& quoteRepository)
    : currencyRepository_(currencyRepository),
      quoteRepository_(quoteRepository) {}

dto::GetRatesResponseDto QuoteQueryService::getRates(const dto::GetRatesRequestDto& request) const {
    const auto provider = common::normalizeProviderKey(request.provider);
    const auto baseCode = common::normalizeCurrencyCode(request.baseCode);

    if (!currencyRepository_.exists(baseCode)) {
        throw common::NotFoundError("Base currency is not supported: " + baseCode);
    }
    if (request.quoteCodes.empty()) {
        throw common::ValidationError("At least one quote currency must be provided");
    }

    dto::GetRatesResponseDto response;
    for (const auto& requestedQuote : request.quoteCodes) {
        const auto quoteCode = common::normalizeCurrencyCode(requestedQuote);
        if (!currencyRepository_.exists(quoteCode)) {
            throw common::NotFoundError("Quote currency is not supported: " + quoteCode);
        }

        const auto quote = quoteRepository_.tryGet(provider, baseCode, quoteCode);
        if (!quote.has_value()) {
            throw common::NotFoundError(
                "No latest quote found for pair " + baseCode + "/" + quoteCode + " and provider " + provider);
        }

        response.quotes.push_back(dto::RateDto{
            .provider = quote->key.provider,
            .baseCode = quote->key.baseCurrency,
            .quoteCode = quote->key.quoteCurrency,
            .rate = quote->rate,
            .sourceTimestamp = common::toIsoString(quote->sourceTimestamp),
        });
    }

    return response;
}

}
