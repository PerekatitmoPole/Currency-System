#pragma once

#include <chrono>
#include <map>
#include <string>

namespace currency::logging {

enum class LogLevel {
    Info,
    Warning,
    Error,
};

using LogContext = std::map<std::string, std::string>;

struct LogEntry {
    std::chrono::system_clock::time_point timestamp;
    LogLevel level{LogLevel::Info};
    std::string category;
    std::string message;
    LogContext context;
};

std::string toString(LogLevel level);
std::string toIsoUtc(std::chrono::system_clock::time_point timestamp);

}
