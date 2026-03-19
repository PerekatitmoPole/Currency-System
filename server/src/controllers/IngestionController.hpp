#pragma once

#include "dto/ProtocolDtos.hpp"
#include "serialization/TextProtocolSerializer.hpp"
#include "services/IngestionService.hpp"

#include <string>

namespace currency::controllers {

class IngestionController {
public:
    IngestionController(services::IngestionService& service, serialization::TextProtocolSerializer& serializer);

    std::string handle(const dto::FieldMap& payload) const;

private:
    services::IngestionService& service_;
    serialization::TextProtocolSerializer& serializer_;
};

}
