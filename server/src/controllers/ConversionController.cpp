#include "controllers/ConversionController.hpp"

namespace currency::controllers {

ConversionController::ConversionController(
    services::ConversionService& service,
    serialization::TextProtocolSerializer& serializer)
    : service_(service), serializer_(serializer) {}

std::string ConversionController::handle(const dto::FieldMap& payload) const {
    const auto request = serializer_.parseConvertRequest(payload);
    const auto response = service_.convert(request);
    return serializer_.successResponse(serializer_.toFields(response));
}

}
