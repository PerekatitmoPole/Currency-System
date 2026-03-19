#include "app/Application.hpp"

#include <thread>

namespace currency::app {

Application::Application(const unsigned short port, const std::size_t workerThreads)
    : ioContext_(static_cast<int>(workerThreads)),
      workGuard_(boost::asio::make_work_guard(ioContext_)),
      ingestionService_(currencyRepository_, quoteRepository_, historyRepository_, queryCache_),
      currencyQueryService_(currencyRepository_),
      quoteQueryService_(currencyRepository_, quoteRepository_),
      historyQueryService_(historyRepository_),
      conversionService_(quoteRepository_),
      ingestionController_(ingestionService_, serializer_),
      currencyController_(currencyQueryService_, queryCache_, serializer_),
      quoteController_(quoteQueryService_, queryCache_, serializer_),
      historyController_(historyQueryService_, queryCache_, serializer_),
      conversionController_(conversionService_, serializer_),
      requestRouter_(
          ingestionController_,
          currencyController_,
          quoteController_,
          historyController_,
          conversionController_,
          serializer_),
      server_(ioContext_, port, requestRouter_),
      workerThreads_(workerThreads) {}

void Application::run() {
    server_.start();

    std::vector<std::jthread> workers;
    workers.reserve(workerThreads_);
    for (std::size_t index = 0; index < workerThreads_; ++index) {
        workers.emplace_back([this] {
            ioContext_.run();
        });
    }
}

}
