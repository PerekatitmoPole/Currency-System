#pragma once

#include "controllers/ConversionController.hpp"
#include "controllers/CurrencyController.hpp"
#include "controllers/HistoryController.hpp"
#include "controllers/IngestionController.hpp"
#include "controllers/QuoteController.hpp"
#include "serialization/TextProtocolSerializer.hpp"

#include <string>

namespace currency::network {

class RequestRouter {
public:
    RequestRouter(
        controllers::IngestionController& ingestionController,
        controllers::CurrencyController& currencyController,
        controllers::QuoteController& quoteController,
        controllers::HistoryController& historyController,
        controllers::ConversionController& conversionController,
        serialization::TextProtocolSerializer& serializer);

    std::string handle(const std::string& rawMessage) const;

private:
    controllers::IngestionController& ingestionController_;
    controllers::CurrencyController& currencyController_;
    controllers::QuoteController& quoteController_;
    controllers::HistoryController& historyController_;
    controllers::ConversionController& conversionController_;
    serialization::TextProtocolSerializer& serializer_;
};

}
