#pragma once

#include "dto/QueryDtos.hpp"
#include "repositories/InMemoryCurrencyRepository.hpp"

namespace currency::services {

class CurrencyQueryService {
public:
    explicit CurrencyQueryService(repositories::InMemoryCurrencyRepository& currencyRepository);

    dto::GetCurrenciesResponseDto getCurrencies() const;

private:
    repositories::InMemoryCurrencyRepository& currencyRepository_;
};

}
