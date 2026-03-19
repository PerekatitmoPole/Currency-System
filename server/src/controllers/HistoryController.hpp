#pragma once

#include "dto/ProtocolDtos.hpp"
#include "repositories/InMemoryQueryCache.hpp"
#include "serialization/TextProtocolSerializer.hpp"
#include "services/HistoryQueryService.hpp"

#include <string>

namespace currency::controllers {

class HistoryController {
public:
    HistoryController(
        services::HistoryQueryService& service,
        repositories::InMemoryQueryCache& cache,
        serialization::TextProtocolSerializer& serializer);

    std::string handle(const dto::FieldMap& payload) const;

private:
    services::HistoryQueryService& service_;
    repositories::InMemoryQueryCache& cache_;
    serialization::TextProtocolSerializer& serializer_;
};

}
