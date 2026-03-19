#pragma once

#include <chrono>
#include <string>

namespace currency::common {

std::string toIsoString(std::chrono::system_clock::time_point value);
std::chrono::system_clock::time_point fromIsoString(const std::string& value);
std::chrono::seconds parseStep(const std::string& step);

}
