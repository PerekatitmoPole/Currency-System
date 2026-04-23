#pragma once

#include "logging/LogSink.hpp"

#include <filesystem>
#include <fstream>
#include <mutex>

namespace currency::logging {

class JsonFileLogSink final : public LogSink {
public:
    explicit JsonFileLogSink(std::filesystem::path path);

    void write(const LogEntry& entry) override;

private:
    std::filesystem::path path_;
    std::ofstream stream_;
    std::mutex mutex_;
};

}
