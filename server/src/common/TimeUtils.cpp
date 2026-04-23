#include "common/TimeUtils.hpp"

#include "common/Exceptions.hpp"

#include <array>
#include <cctype>
#include <ctime>
#include <iomanip>
#include <sstream>

namespace currency::common {

namespace {

std::time_t toUtcTime(std::tm* timeInfo) {
#ifdef _WIN32
    return _mkgmtime(timeInfo);
#else
    return timegm(timeInfo);
#endif
}

}

std::string toIsoString(const std::chrono::system_clock::time_point value) {
    const auto raw = std::chrono::system_clock::to_time_t(value);
    std::tm timeInfo{};
#ifdef _WIN32
    gmtime_s(&timeInfo, &raw);
#else
    gmtime_r(&raw, &timeInfo);
#endif

    std::ostringstream output;
    output << std::put_time(&timeInfo, "%Y-%m-%dT%H:%M:%SZ");
    return output.str();
}

std::chrono::system_clock::time_point fromIsoString(const std::string& value) {
    std::tm timeInfo{};
    std::istringstream input(value);
    input >> std::get_time(&timeInfo, "%Y-%m-%dT%H:%M:%SZ");
    if (input.fail()) {
        throw ValidationError("Invalid ISO-8601 UTC timestamp: " + value);
    }

    return std::chrono::system_clock::from_time_t(toUtcTime(&timeInfo));
}

std::chrono::seconds parseStep(const std::string& step) {
    if (step.empty()) {
        throw ValidationError("Step must not be empty");
    }

    std::size_t index = 0;
    while (index < step.size() && std::isdigit(static_cast<unsigned char>(step[index])) != 0) {
        ++index;
    }

    if (index == 0 || index == step.size()) {
        throw ValidationError("Invalid step format: " + step);
    }

    const int amount = std::stoi(step.substr(0, index));
    const std::string unit = step.substr(index);

    if (amount <= 0) {
        throw ValidationError("Step amount must be positive");
    }

    if (unit == "s") {
        return std::chrono::seconds(amount);
    }
    if (unit == "min" || unit == "mins") {
        return std::chrono::minutes(amount);
    }
    if (unit == "h") {
        return std::chrono::hours(amount);
    }
    if (unit == "d") {
        return std::chrono::hours(24 * amount);
    }
    if (unit == "m" || unit == "mo" || unit == "mon" || unit == "month" || unit == "months") {
        return std::chrono::hours(24 * 30 * amount);
    }
    if (unit == "y" || unit == "yr" || unit == "year" || unit == "years") {
        return std::chrono::hours(24 * 365 * amount);
    }

    throw ValidationError("Unsupported step unit: " + unit);
}

}
