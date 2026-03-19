#pragma once

#include "dto/ProtocolDtos.hpp"
#include "repositories/InMemoryQueryCache.hpp"
#include "serialization/TextProtocolSerializer.hpp"
#include "services/CurrencyQueryService.hpp"

#include <string>

namespace currency::controllers {

class CurrencyController {
public:
    CurrencyController(
        services::CurrencyQueryService& service,
        repositories::InMemoryQueryCache& cache,
        serialization::TextProtocolSerializer& serializer);

    std::string handle(const dto::FieldMap& payload) const;

private:
    services::CurrencyQueryService& service_;
    repositories::InMemoryQueryCache& cache_;
    serialization::TextProtocolSerializer& serializer_;
};

}
