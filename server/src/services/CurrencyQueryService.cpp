#include "services/CurrencyQueryService.hpp"

#include <algorithm>

namespace currency::services {

CurrencyQueryService::CurrencyQueryService(repositories::InMemoryCurrencyRepository& currencyRepository)
    : currencyRepository_(currencyRepository) {}

dto::GetCurrenciesResponseDto CurrencyQueryService::getCurrencies() const {
    dto::GetCurrenciesResponseDto response;
    const auto currencies = currencyRepository_.list();
    response.currencies.reserve(currencies.size());

    for (const auto& currency : currencies) {
        response.currencies.push_back(dto::CurrencyDto{
            .code = currency.code,
            .name = currency.name,
            .minorUnits = static_cast<int>(currency.minorUnits),
        });
    }

    std::ranges::sort(response.currencies, {}, &dto::CurrencyDto::code);
    return response;
}

}
