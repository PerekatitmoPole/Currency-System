#pragma once

#include "logging/LogEntry.hpp"

namespace currency::logging {

class LogSink {
public:
    virtual ~LogSink() = default;

    virtual void write(const LogEntry& entry) = 0;
};

}
