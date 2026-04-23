#pragma once

#include "dto/ProtocolDtos.hpp"
#include "serialization/TextProtocolSerializer.hpp"
#include "services/ConversionService.hpp"
#include "services/MarketDataRefreshService.hpp"

#include <string>

namespace currency::controllers {

class ConversionController {
public:
    ConversionController(
        services::MarketDataRefreshService& refreshService,
        services::ConversionService& service,
        serialization::TextProtocolSerializer& serializer);

    std::string handle(const dto::FieldMap& payload) const;

private:
    services::MarketDataRefreshService& refreshService_;
    services::ConversionService& service_;
    serialization::TextProtocolSerializer& serializer_;
};

}
