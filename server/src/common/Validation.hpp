#pragma once

#include <string>

namespace currency::common {

std::string normalizeCurrencyCode(std::string value);
std::string normalizeProviderKey(std::string value);
void requireNotBlank(const std::string& value, const std::string& fieldName);
void requirePositive(double value, const std::string& fieldName);

}
