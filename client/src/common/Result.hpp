#pragma once

#include "common/Errors.hpp"

#include <optional>
#include <utility>

namespace currency::client::common {

template <typename T>
class Result {
public:
    static Result success(T value) {
        return Result(std::move(value), std::nullopt);
    }

    static Result failure(Error error) {
        return Result(std::nullopt, std::move(error));
    }

    [[nodiscard]] bool ok() const {
        return value_.has_value();
    }

    [[nodiscard]] const T& value() const {
        return *value_;
    }

    [[nodiscard]] T& value() {
        return *value_;
    }

    [[nodiscard]] const Error& error() const {
        return *error_;
    }

private:
    Result(std::optional<T> value, std::optional<Error> error)
        : value_(std::move(value)), error_(std::move(error)) {}

    std::optional<T> value_;
    std::optional<Error> error_;
};

template <>
class Result<void> {
public:
    static Result success() {
        return Result(true, std::nullopt);
    }

    static Result failure(Error error) {
        return Result(false, std::move(error));
    }

    [[nodiscard]] bool ok() const {
        return ok_;
    }

    [[nodiscard]] const Error& error() const {
        return *error_;
    }

private:
    Result(bool ok, std::optional<Error> error)
        : ok_(ok), error_(std::move(error)) {}

    bool ok_{false};
    std::optional<Error> error_;
};

}