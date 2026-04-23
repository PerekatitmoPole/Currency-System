#pragma once

#include "dto/ProtocolDtos.hpp"
#include "logging/Logger.hpp"
#include "repositories/InMemoryQueryCache.hpp"
#include "serialization/TextProtocolSerializer.hpp"
#include "services/MarketDataRefreshService.hpp"
#include "services/QuoteQueryService.hpp"

#include <string>

namespace currency::controllers {

class QuoteController {
public:
    QuoteController(
        logging::Logger& logger,
        services::MarketDataRefreshService& refreshService,
        services::QuoteQueryService& service,
        repositories::InMemoryQueryCache& cache,
        serialization::TextProtocolSerializer& serializer);

    std::string handle(const dto::FieldMap& payload) const;

private:
    logging::Logger& logger_;
    services::MarketDataRefreshService& refreshService_;
    services::QuoteQueryService& service_;
    repositories::InMemoryQueryCache& cache_;
    serialization::TextProtocolSerializer& serializer_;
};

}
