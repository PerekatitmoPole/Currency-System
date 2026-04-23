#pragma once

#include "dto/ProtocolDtos.hpp"
#include "logging/Logger.hpp"
#include "repositories/InMemoryQueryCache.hpp"
#include "serialization/TextProtocolSerializer.hpp"
#include "services/HistoryQueryService.hpp"
#include "services/MarketDataRefreshService.hpp"

#include <string>

namespace currency::controllers {

class HistoryController {
public:
    HistoryController(
        logging::Logger& logger,
        services::MarketDataRefreshService& refreshService,
        services::HistoryQueryService& service,
        repositories::InMemoryQueryCache& cache,
        serialization::TextProtocolSerializer& serializer);

    std::string handle(const dto::FieldMap& payload) const;

private:
    logging::Logger& logger_;
    services::MarketDataRefreshService& refreshService_;
    services::HistoryQueryService& service_;
    repositories::InMemoryQueryCache& cache_;
    serialization::TextProtocolSerializer& serializer_;
};

}
