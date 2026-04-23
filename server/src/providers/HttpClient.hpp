#pragma once

#include <chrono>
#include <string>

namespace currency::providers {

class HttpClient {
public:
    std::string get(const std::string& url, std::chrono::milliseconds timeout) const;
};

}
