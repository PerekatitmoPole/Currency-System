#pragma once

#include "controllers/ConversionController.hpp"
#include "controllers/CurrencyController.hpp"
#include "controllers/HistoryController.hpp"
#include "controllers/IngestionController.hpp"
#include "controllers/QuoteController.hpp"
#include "network/RequestRouter.hpp"
#include "network/TcpServer.hpp"
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

#include <boost/asio.hpp>

#include <memory>
#include <vector>

namespace currency::app {

class Application {
public:
    Application(unsigned short port, std::size_t workerThreads);

    void run();

private:
    boost::asio::io_context ioContext_;
    boost::asio::executor_work_guard<boost::asio::io_context::executor_type> workGuard_;
    repositories::InMemoryCurrencyRepository currencyRepository_;
    repositories::InMemoryQuoteRepository quoteRepository_;
    repositories::InMemoryHistoryRepository historyRepository_;
    repositories::InMemoryQueryCache queryCache_;
    serialization::TextProtocolSerializer serializer_;
    services::IngestionService ingestionService_;
    services::CurrencyQueryService currencyQueryService_;
    services::QuoteQueryService quoteQueryService_;
    services::HistoryQueryService historyQueryService_;
    services::ConversionService conversionService_;
    controllers::IngestionController ingestionController_;
    controllers::CurrencyController currencyController_;
    controllers::QuoteController quoteController_;
    controllers::HistoryController historyController_;
    controllers::ConversionController conversionController_;
    network::RequestRouter requestRouter_;
    network::TcpServer server_;
    std::size_t workerThreads_;
};

}
