#include "app/Application.hpp"

#include <cstdlib>
#include <filesystem>
#include <thread>

namespace currency::app {

namespace {

std::filesystem::path resolveLogPath() {
    if (const auto* configured = std::getenv("CURRENCY_LOG_DIR"); configured != nullptr && *configured != '\0') {
        return std::filesystem::path(configured) / "server.jsonl";
    }

    return std::filesystem::path("logs") / "server.jsonl";
}

}

Application::Application(const unsigned short port, const std::size_t workerThreads)
    : ioContext_(static_cast<int>(workerThreads)),
      workGuard_(boost::asio::make_work_guard(ioContext_)),
      frankfurterProvider_(httpClient_),
      ecbProvider_(httpClient_),
      cbrProvider_(httpClient_),
      ingestionService_(currencyRepository_, quoteRepository_, historyRepository_, queryCache_),
      marketDataRefreshService_(logger_, currencyRepository_, quoteRepository_, historyRepository_),
      currencyQueryService_(currencyRepository_),
      quoteQueryService_(currencyRepository_, quoteRepository_),
      historyQueryService_(historyRepository_),
      conversionService_(quoteRepository_),
      ingestionController_(ingestionService_, serializer_),
      currencyController_(currencyQueryService_, marketDataRefreshService_, queryCache_, serializer_),
      quoteController_(logger_, marketDataRefreshService_, quoteQueryService_, queryCache_, serializer_),
      historyController_(logger_, marketDataRefreshService_, historyQueryService_, queryCache_, serializer_),
      conversionController_(marketDataRefreshService_, conversionService_, serializer_),
      requestRouter_(
          logger_,
          ingestionController_,
          currencyController_,
          quoteController_,
          historyController_,
          conversionController_,
          serializer_),
      server_(ioContext_, port, requestRouter_, logger_),
      workerThreads_(workerThreads),
      port_(port) {
    consoleLogSink_ = std::make_shared<logging::ConsoleLogSink>();
    logger_.addSink(consoleLogSink_);

    try {
        jsonFileLogSink_ = std::make_shared<logging::JsonFileLogSink>(resolveLogPath());
        logger_.addSink(jsonFileLogSink_);
    } catch (const std::exception& error) {
        logger_.warning(
            "application",
            "File log sink is disabled",
            {
                {"message", error.what()},
            });
    }

    marketDataRefreshService_.registerProvider(frankfurterProvider_);
    marketDataRefreshService_.registerProvider(ecbProvider_);
    marketDataRefreshService_.registerProvider(cbrProvider_);

    logger_.info(
        "application",
        "Application initialized",
        {
            {"port", std::to_string(port_)},
            {"worker_threads", std::to_string(workerThreads_)},
        });
}

void Application::run() {
    logger_.info(
        "application",
        "Server is starting",
        {
            {"port", std::to_string(port_)},
            {"worker_threads", std::to_string(workerThreads_)},
        });
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
