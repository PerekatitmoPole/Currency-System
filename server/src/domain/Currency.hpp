#pragma once

#include <cstdint>
#include <string>

namespace currency::domain {

struct Currency {
    std::string code;
    std::string name;
    std::uint8_t minorUnits{2};
};

}
