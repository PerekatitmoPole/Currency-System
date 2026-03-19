#include "network/RequestRouter.hpp"

#include "common/Exceptions.hpp"

namespace currency::network {

RequestRouter::RequestRouter(
    controllers::IngestionController& ingestionController,
    controllers::CurrencyController& currencyController,
    controllers::QuoteController& quoteController,
    controllers::HistoryController& historyController,
    controllers::ConversionController& conversionController,
    serialization::TextProtocolSerializer& serializer)
    : ingestionController_(ingestionController),
      currencyController_(currencyController),
      quoteController_(quoteController),
      historyController_(historyController),
      conversionController_(conversionController),
      serializer_(serializer) {}

std::string RequestRouter::handle(const std::string& rawMessage) const {
    try {
        const auto request = serializer_.parseRequestEnvelope(rawMessage);

        if (request.command == "ingest_quotes") {
            return ingestionController_.handle(request.payload);
        }
        if (request.command == "get_currencies") {
            return currencyController_.handle(request.payload);
        }
        if (request.command == "get_rates") {
            return quoteController_.handle(request.payload);
        }
        if (request.command == "get_history") {
            return historyController_.handle(request.payload);
        }
        if (request.command == "convert") {
            return conversionController_.handle(request.payload);
        }

        return serializer_.errorResponse(dto::ErrorDto{
            .code = "unknown_command",
            .message = "Unsupported command: " + request.command,
        });
    } catch (const common::AppError& error) {
        return serializer_.errorResponse(dto::ErrorDto{.code = error.code(), .message = error.what()});
    } catch (const std::exception& error) {
        return serializer_.errorResponse(dto::ErrorDto{.code = "internal_error", .message = error.what()});
    }
}

}
