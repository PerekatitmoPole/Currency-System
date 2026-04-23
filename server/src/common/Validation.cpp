#include "common/Validation.hpp"

#include "common/Exceptions.hpp"

#include <algorithm>
#include <cctype>

namespace currency::common {

namespace {

std::string sanitizeKey(std::string value) {
    std::string normalized;
    normalized.reserve(value.size());

    for (const unsigned char ch : value) {
        if (std::isalnum(ch) != 0) {
            normalized.push_back(static_cast<char>(std::tolower(ch)));
        }
    }

    return normalized;
}

}

std::string normalizeCurrencyCode(std::string value) {
    requireNotBlank(value, "currency code");
    std::transform(value.begin(), value.end(), value.begin(), [](const unsigned char ch) {
        return static_cast<char>(std::toupper(ch));
    });

    if (value.size() != 3) {
        throw ValidationError("Currency code must contain exactly 3 letters");
    }

    for (const char ch : value) {
        if (std::isalpha(static_cast<unsigned char>(ch)) == 0) {
            throw ValidationError("Currency code must contain only letters");
        }
    }

    return value;
}

std::string normalizeProviderKey(std::string value) {
    requireNotBlank(value, "provider");
    value = sanitizeKey(std::move(value));
    if (value.empty()) {
        throw ValidationError("Provider must contain letters or digits");
    }

    if (value == "ecb") {
        return "ecb";
    }
    if (value == "frankfurter") {
        return "frankfurter";
    }
    if (value == "cbr" || value == "cbrrf") {
        return "cbr";
    }
    if (value == "currencyapi") {
        return "currencyapi";
    }
    if (value == "openexchangerates") {
        return "openexchangerates";
    }
    if (value == "exchangeratehost" || value == "exchangeratehostcom") {
        return "exchangeratehost";
    }
    if (value == "fixer") {
        return "fixer";
    }
    if (value == "currencyfreaks") {
        return "currencyfreaks";
    }
    if (value == "alphavantage") {
        return "alphavantage";
    }

    return value;
}

void requireNotBlank(const std::string& value, const std::string& fieldName) {
    if (value.empty()) {
        throw ValidationError(fieldName + " must not be empty");
    }
}

void requirePositive(const double value, const std::string& fieldName) {
    if (value <= 0.0) {
        throw ValidationError(fieldName + " must be positive");
    }
}

}
