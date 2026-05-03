#include "logging/LogEntry.hpp"

#include <ctime>
#include <iomanip>
#include <sstream>

extern "C" {
#include <time.h>
}

namespace currency::logging {

std::string toString(const LogLevel level) {
    switch (level) {
    case LogLevel::Info:
        return "info";
    case LogLevel::Warning:
        return "warning";
    case LogLevel::Error:
        return "error";
    }

    return "info";
}

std::string toIsoUtc(const std::chrono::system_clock::time_point timestamp) {
    const auto time = std::chrono::system_clock::to_time_t(timestamp);
    std::tm utcTime{};
#if defined(_WIN32)
    if (gmtime_s(&utcTime, &time) != 0) {
        return {};
    }
#else
    if (gmtime_r(&time, &utcTime) == nullptr) {
        return {};
    }
#endif

    std::ostringstream stream;
    stream << std::put_time(&utcTime, "%Y-%m-%dT%H:%M:%SZ");
    return stream.str();
}

}
