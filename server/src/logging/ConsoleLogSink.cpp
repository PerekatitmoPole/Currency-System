#include "logging/ConsoleLogSink.hpp"

#include <iostream>

namespace currency::logging {

void ConsoleLogSink::write(const LogEntry& entry) {
    std::scoped_lock lock(mutex_);

    auto& stream = entry.level == LogLevel::Error ? std::cerr : std::clog;
    stream << '[' << toIsoUtc(entry.timestamp) << "] ["
           << toString(entry.level) << "] ["
           << entry.category << "] "
           << entry.message;

    for (const auto& [key, value] : entry.context) {
        stream << ' ' << key << '=' << value;
    }

    stream << '\n';
}

}
