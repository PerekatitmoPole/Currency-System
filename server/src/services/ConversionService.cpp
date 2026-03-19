#include "services/ConversionService.hpp"

#include "common/Exceptions.hpp"
#include "common/TimeUtils.hpp"
#include "common/Validation.hpp"

namespace currency::services {

ConversionService::ConversionService(repositories::InMemoryQuoteRepository& quoteRepository)
    : quoteRepository_(quoteRepository) {}

dto::ConvertResponseDto ConversionService::convert(const dto::ConvertRequestDto& request) const {
    common::requireNotBlank(request.provider, "provider");
    common::requirePositive(request.amount, "amount");
    const auto fromCurrency = common::normalizeCurrencyCode(request.fromCurrency);
    const auto toCurrency = common::normalizeCurrencyCode(request.toCurrency);

    if (fromCurrency == toCurrency) {
        return dto::ConvertResponseDto{
            .provider = request.provider,
            .fromCurrency = fromCurrency,
            .toCurrency = toCurrency,
            .amount = request.amount,
            .rate = 1.0,
            .result = request.amount,
            .timestamp = common::toIsoString(std::chrono::system_clock::now()),
        };
    }

    const auto quote = quoteRepository_.tryGet(request.provider, fromCurrency, toCurrency);
    if (!quote.has_value()) {
        throw common::NotFoundError(
            "No conversion quote found for pair " + fromCurrency + "/" + toCurrency + " and provider " + request.provider);
    }

    return dto::ConvertResponseDto{
        .provider = request.provider,
        .fromCurrency = fromCurrency,
        .toCurrency = toCurrency,
        .amount = request.amount,
        .rate = quote->rate,
        .result = request.amount * quote->rate,
        .timestamp = common::toIsoString(quote->sourceTimestamp),
    };
}

}
