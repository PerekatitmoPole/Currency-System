#pragma once

#include <stdexcept>
#include <string>

namespace currency::common {

class AppError : public std::runtime_error {
public:
    AppError(std::string code, std::string message)
        : std::runtime_error(std::move(message)), code_(std::move(code)) {}

    const std::string& code() const noexcept { return code_; }

private:
    std::string code_;
};

class ValidationError final : public AppError {
public:
    explicit ValidationError(const std::string& message)
        : AppError("validation_error", message) {}
};

class NotFoundError final : public AppError {
public:
    explicit NotFoundError(const std::string& message)
        : AppError("not_found", message) {}
};

class ProtocolError final : public AppError {
public:
    explicit ProtocolError(const std::string& message)
        : AppError("protocol_error", message) {}
};

class ExternalApiError final : public AppError {
public:
    explicit ExternalApiError(const std::string& message)
        : AppError("provider_unavailable", message) {}
};

}
