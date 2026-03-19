#pragma once

#include <chrono>

namespace currency::domain {

struct HistoryPoint {
    std::chrono::system_clock::time_point timestamp{};
    double rate{0.0};
};

}
