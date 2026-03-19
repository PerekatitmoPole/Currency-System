#pragma once

#include "dto/CommonDtos.hpp"

#include <map>
#include <optional>
#include <string>

namespace currency::dto {

using FieldMap = std::map<std::string, std::string>;

struct RequestEnvelopeDto {
    std::string command;
    FieldMap payload;
};

struct ResponseEnvelopeDto {
    std::string status;
    FieldMap payload;
    std::optional<ErrorDto> error;
};

}
