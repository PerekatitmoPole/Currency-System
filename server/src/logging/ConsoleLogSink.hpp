#pragma once

#include "logging/LogSink.hpp"

#include <mutex>

namespace currency::logging {

class ConsoleLogSink final : public LogSink {
public:
    void write(const LogEntry& entry) override;

private:
    std::mutex mutex_;
};

}
