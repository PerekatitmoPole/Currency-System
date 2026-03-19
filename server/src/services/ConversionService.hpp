#pragma once

#include "dto/QueryDtos.hpp"
#include "repositories/InMemoryQuoteRepository.hpp"

namespace currency::services {

class ConversionService {
public:
    explicit ConversionService(repositories::InMemoryQuoteRepository& quoteRepository);

    dto::ConvertResponseDto convert(const dto::ConvertRequestDto& request) const;

private:
    repositories::InMemoryQuoteRepository& quoteRepository_;
};

}
