#include "logging/JsonFileLogSink.hpp"

#include <nlohmann/json.hpp>

#include <stdexcept>

namespace currency::logging {

JsonFileLogSink::JsonFileLogSink(std::filesystem::path path)
    : path_(std::move(path)) {
    if (path_.has_parent_path()) {
        std::filesystem::create_directories(path_.parent_path());
    }

    stream_.open(path_, std::ios::app);
    if (!stream_.is_open()) {
        throw std::runtime_error("Failed to open log file: " + path_.string());
    }
}

void JsonFileLogSink::write(const LogEntry& entry) {
    nlohmann::json payload{
        {"timestamp", toIsoUtc(entry.timestamp)},
        {"level", toString(entry.level)},
        {"category", entry.category},
        {"message", entry.message},
        {"context", entry.context},
    };

    std::scoped_lock lock(mutex_);
    stream_ << payload.dump(
        -1,
        ' ',
        false,
        nlohmann::json::error_handler_t::replace) << '\n';
    stream_.flush();
}

}
