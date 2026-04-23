#pragma once

#include "common/TimeUtils.hpp"
#include "controllers/ConversionController.hpp"
#include "controllers/CurrencyController.hpp"
#include "controllers/HistoryController.hpp"
#include "controllers/IngestionController.hpp"
#include "controllers/QuoteController.hpp"
#include "logging/Logger.hpp"
#include "network/RequestRouter.hpp"
#include "providers/MarketDataProvider.hpp"
#include "repositories/InMemoryCurrencyRepository.hpp"
#include "repositories/InMemoryHistoryRepository.hpp"
#include "repositories/InMemoryQueryCache.hpp"
#include "repositories/InMemoryQuoteRepository.hpp"
#include "serialization/TextProtocolSerializer.hpp"
#include "services/ConversionService.hpp"
#include "services/CurrencyQueryService.hpp"
#include "services/HistoryQueryService.hpp"
#include "services/IngestionService.hpp"
#include "services/MarketDataRefreshService.hpp"
#include "services/QuoteQueryService.hpp"

#include "common/Exceptions.hpp"

#include <map>
#include <string>

namespace currency::tests {

class FallbackProvider final : public providers::MarketDataProvider {
public:
    explicit FallbackProvider(std::string providerKey)
        : providerKey_(std::move(providerKey)) {}

    std::string key() const override {
        return providerKey_;
    }

    std::vector<providers::QuoteSnapshot> fetchLatest(
        const std::string&,
        const std::vector<std::string>&) const override {
        return {};
    }

    std::vector<domain::HistoryPoint> fetchHistory(
        const std::string&,
        const std::string&,
        std::chrono::system_clock::time_point,
        std::chrono::system_clock::time_point) const override {
        return {};
    }

private:
    std::string providerKey_;
};

struct TestAppContext {
    logging::Logger logger;
    repositories::InMemoryCurrencyRepository currencyRepository;
    repositories::InMemoryQuoteRepository quoteRepository;
    repositories::InMemoryHistoryRepository historyRepository;
    repositories::InMemoryQueryCache cache;
    serialization::TextProtocolSerializer serializer;
    services::IngestionService ingestionService{currencyRepository, quoteRepository, historyRepository, cache};
    services::MarketDataRefreshService marketDataRefreshService{logger, currencyRepository, quoteRepository, historyRepository};
    services::CurrencyQueryService currencyQueryService{currencyRepository};
    services::QuoteQueryService quoteQueryService{currencyRepository, quoteRepository};
    services::HistoryQueryService historyQueryService{historyRepository};
    services::ConversionService conversionService{quoteRepository};
    FallbackProvider ecbProvider{"ecb"};
    FallbackProvider frankfurterProvider{"frankfurter"};
    FallbackProvider cbrProvider{"cbr"};
    controllers::IngestionController ingestionController{ingestionService, serializer};
    controllers::CurrencyController currencyController{currencyQueryService, marketDataRefreshService, cache, serializer};
    controllers::QuoteController quoteController{logger, marketDataRefreshService, quoteQueryService, cache, serializer};
    controllers::HistoryController historyController{logger, marketDataRefreshService, historyQueryService, cache, serializer};
    controllers::ConversionController conversionController{marketDataRefreshService, conversionService, serializer};
    network::RequestRouter router{logger, ingestionController, currencyController, quoteController, historyController, conversionController, serializer};

    TestAppContext() {
        marketDataRefreshService.registerProvider(ecbProvider);
        marketDataRefreshService.registerProvider(frankfurterProvider);
        marketDataRefreshService.registerProvider(cbrProvider);
    }
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
