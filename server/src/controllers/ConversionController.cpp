#include "controllers/ConversionController.hpp"

#include "common/Validation.hpp"

namespace currency::controllers {

ConversionController::ConversionController(
    services::MarketDataRefreshService& refreshService,
    services::ConversionService& service,
    serialization::TextProtocolSerializer& serializer)
    : refreshService_(refreshService), service_(service), serializer_(serializer) {}

std::string ConversionController::handle(const dto::FieldMap& payload) const {
    const auto request = serializer_.parseConvertRequest(payload);
    const auto sameCurrency =
        common::normalizeCurrencyCode(request.fromCurrency) == common::normalizeCurrencyCode(request.toCurrency);

    const auto refresh = sameCurrency
        ? services::RefreshOutcome{}
        : refreshService_.ensureLatest(request.provider, request.fromCurrency, {request.toCurrency});
    const auto response = service_.convert(request);
    auto fields = serializer_.toFields(response);
    if (refresh.usedFallback) {
        fields["stale"] = "true";
        fields["warning"] = refresh.warning;
    }

    return serializer_.successResponse(fields);
}

}
