#include "network/RequestRouter.hpp"

#include "common/Exceptions.hpp"

namespace currency::network {

RequestRouter::RequestRouter(
    logging::Logger& logger,
    controllers::IngestionController& ingestionController,
    controllers::CurrencyController& currencyController,
    controllers::QuoteController& quoteController,
    controllers::HistoryController& historyController,
    controllers::ConversionController& conversionController,
    serialization::TextProtocolSerializer& serializer)
    : logger_(logger),
      ingestionController_(ingestionController),
      currencyController_(currencyController),
      quoteController_(quoteController),
      historyController_(historyController),
      conversionController_(conversionController),
      serializer_(serializer) {}

std::string RequestRouter::handle(const std::string& rawMessage) const {
    try {
        const auto request = serializer_.parseRequestEnvelope(rawMessage);

        if (request.command == "ingest_quotes") {
            auto response = ingestionController_.handle(request.payload);
            logger_.info("request_router", "Handled request", {{"command", request.command}, {"status", "ok"}});
            return response;
        }
        if (request.command == "get_currencies") {
            auto response = currencyController_.handle(request.payload);
            logger_.info("request_router", "Handled request", {{"command", request.command}, {"status", "ok"}});
            return response;
        }
        if (request.command == "get_rates") {
            auto response = quoteController_.handle(request.payload);
            logger_.info("request_router", "Handled request", {{"command", request.command}, {"status", "ok"}});
            return response;
        }
        if (request.command == "get_history") {
            auto response = historyController_.handle(request.payload);
            logger_.info("request_router", "Handled request", {{"command", request.command}, {"status", "ok"}});
            return response;
        }
        if (request.command == "convert") {
            auto response = conversionController_.handle(request.payload);
            logger_.info("request_router", "Handled request", {{"command", request.command}, {"status", "ok"}});
            return response;
        }

        logger_.warning("request_router", "Unsupported command", {{"command", request.command}});
        return serializer_.errorResponse(dto::ErrorDto{
            .code = "unknown_command",
            .message = "Unsupported command: " + request.command,
        });
    } catch (const common::AppError& error) {
        logger_.warning(
            "request_router",
            "Application error while handling request",
            {
                {"code", error.code()},
                {"message", error.what()},
            });
        return serializer_.errorResponse(dto::ErrorDto{.code = error.code(), .message = error.what()});
    } catch (const std::exception& error) {
        logger_.error(
            "request_router",
            "Unhandled exception while processing request",
            {
                {"message", error.what()},
            });
        return serializer_.errorResponse(dto::ErrorDto{.code = "internal_error", .message = error.what()});
    }
}

}
