#include "controllers/IngestionController.hpp"

namespace currency::controllers {

IngestionController::IngestionController(
    services::IngestionService& service,
    serialization::TextProtocolSerializer& serializer)
    : service_(service), serializer_(serializer) {}

std::string IngestionController::handle(const dto::FieldMap& payload) const {
    const auto request = serializer_.parseUpdateQuotesRequest(payload);
    const auto response = service_.ingest(request);
    return serializer_.successResponse(serializer_.toFields(response));
}

}
