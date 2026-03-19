#include "common/Validation.hpp"

#include "common/Exceptions.hpp"

#include <algorithm>
#include <cctype>

namespace currency::common {

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
