#pragma once

#include "common/TimeUtils.hpp"
#include "controllers/ConversionController.hpp"
#include "controllers/CurrencyController.hpp"
#include "controllers/HistoryController.hpp"
#include "controllers/IngestionController.hpp"
#include "controllers/QuoteController.hpp"
#include "network/RequestRouter.hpp"
#include "repositories/InMemoryCurrencyRepository.hpp"
#include "repositories/InMemoryHistoryRepository.hpp"
#include "repositories/InMemoryQueryCache.hpp"
#include "repositories/InMemoryQuoteRepository.hpp"
#include "serialization/TextProtocolSerializer.hpp"
#include "services/ConversionService.hpp"
#include "services/CurrencyQueryService.hpp"
#include "services/HistoryQueryService.hpp"
#include "services/IngestionService.hpp"
#include "services/QuoteQueryService.hpp"

#include <map>
#include <string>

namespace currency::tests {

struct TestAppContext {
    repositories::InMemoryCurrencyRepository currencyRepository;
    repositories::InMemoryQuoteRepository quoteRepository;
    repositories::InMemoryHistoryRepository historyRepository;
    repositories::InMemoryQueryCache cache;
    serialization::TextProtocolSerializer serializer;
    services::IngestionService ingestionService{currencyRepository, quoteRepository, historyRepository, cache};
    services::CurrencyQueryService currencyQueryService{currencyRepository};
    services::QuoteQueryService quoteQueryService{currencyRepository, quoteRepository};
    services::HistoryQueryService historyQueryService{historyRepository};
    services::ConversionService conversionService{quoteRepository};
    controllers::IngestionController ingestionController{ingestionService, serializer};
    controllers::CurrencyController currencyController{currencyQueryService, cache, serializer};
    controllers::QuoteController quoteController{quoteQueryService, cache, serializer};
    controllers::HistoryController historyController{historyQueryService, cache, serializer};
    controllers::ConversionController conversionController{conversionService, serializer};
    network::RequestRouter router{ingestionController, currencyController, quoteController, historyController, conversionController, serializer};
};

inline dto::FieldMap parseEnvelopePayload(serialization::TextProtocolSerializer& serializer, const std::string& raw) {
    return serializer.parseRequestEnvelope("command=_;" + raw).payload;
}

inline std::string requireField(const dto::FieldMap& fields, const std::string& key) {
    const auto it = fields.find(key);
    if (it == fields.end()) {
        throw std::runtime_error("Missing field: " + key);
    }
    return it->second;
}

inline void seedQuote(
    repositories::InMemoryQuoteRepository& quoteRepository,
    const std::string& provider,
    const std::string& baseCurrency,
    const std::string& quoteCurrency,
    double rate,
    const std::string& timestamp) {
    quoteRepository.upsert(domain::Quote{
        .key = domain::QuoteKey{provider, baseCurrency, quoteCurrency},
        .rate = rate,
        .sourceTimestamp = common::fromIsoString(timestamp),
        .receivedAt = common::fromIsoString(timestamp),
    });
}

}
