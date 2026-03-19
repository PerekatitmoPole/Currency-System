#pragma once

#include "dto/ProtocolDtos.hpp"
#include "repositories/InMemoryQueryCache.hpp"
#include "serialization/TextProtocolSerializer.hpp"
#include "services/QuoteQueryService.hpp"

#include <string>

namespace currency::controllers {

class QuoteController {
public:
    QuoteController(
        services::QuoteQueryService& service,
        repositories::InMemoryQueryCache& cache,
        serialization::TextProtocolSerializer& serializer);

    std::string handle(const dto::FieldMap& payload) const;

private:
    services::QuoteQueryService& service_;
    repositories::InMemoryQueryCache& cache_;
    serialization::TextProtocolSerializer& serializer_;
};

}
