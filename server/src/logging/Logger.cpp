#include "logging/Logger.hpp"

#include <chrono>
#include <iostream>
#include <utility>

namespace currency::logging {

Logger::Logger(std::vector<std::shared_ptr<LogSink>> sinks)
    : sinks_(std::move(sinks)) {}

void Logger::addSink(std::shared_ptr<LogSink> sink) {
    if (sink) {
        sinks_.push_back(std::move(sink));
    }
}

void Logger::info(std::string category, std::string message, LogContext context) const {
    log(LogLevel::Info, std::move(category), std::move(message), std::move(context));
}

void Logger::warning(std::string category, std::string message, LogContext context) const {
    log(LogLevel::Warning, std::move(category), std::move(message), std::move(context));
}

void Logger::error(std::string category, std::string message, LogContext context) const {
    log(LogLevel::Error, std::move(category), std::move(message), std::move(context));
}

void Logger::log(
    const LogLevel level,
    std::string category,
    std::string message,
    LogContext context) const {
    LogEntry entry{
        .timestamp = std::chrono::system_clock::now(),
        .level = level,
        .category = std::move(category),
        .message = std::move(message),
        .context = std::move(context),
    };

    for (const auto& sink : sinks_) {
        if (!sink) {
            continue;
        }

        try {
            sink->write(entry);
        } catch (const std::exception& error) {
            std::cerr << "[currency_server] logger failure: " << error.what() << '\n';
        }
    }
}

}
