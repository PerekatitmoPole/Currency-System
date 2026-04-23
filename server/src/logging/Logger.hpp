#pragma once

#include "logging/LogSink.hpp"

#include <memory>
#include <vector>

namespace currency::logging {

class Logger {
public:
    Logger() = default;
    explicit Logger(std::vector<std::shared_ptr<LogSink>> sinks);

    void addSink(std::shared_ptr<LogSink> sink);

    void info(std::string category, std::string message, LogContext context = {}) const;
    void warning(std::string category, std::string message, LogContext context = {}) const;
    void error(std::string category, std::string message, LogContext context = {}) const;

private:
    void log(LogLevel level, std::string category, std::string message, LogContext context) const;

    std::vector<std::shared_ptr<LogSink>> sinks_;
};

}
