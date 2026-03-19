#pragma once

#include "dto/ProtocolDtos.hpp"
#include "serialization/TextProtocolSerializer.hpp"
#include "services/ConversionService.hpp"

#include <string>

namespace currency::controllers {

class ConversionController {
public:
    ConversionController(services::ConversionService& service, serialization::TextProtocolSerializer& serializer);

    std::string handle(const dto::FieldMap& payload) const;

private:
    services::ConversionService& service_;
    serialization::TextProtocolSerializer& serializer_;
};

}
