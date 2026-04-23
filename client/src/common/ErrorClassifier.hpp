#pragma once

#include "common/Errors.hpp"

namespace currency::client::common {

inline bool isTransportError(const Error& error) {
    if (error.code == "protocol_error") {
        return true;
    }
    if (error.code != "network_error") {
        return false;
    }
    // Domain/provider failures are delivered via network_error in some legacy flows.
    return error.details != "provider_unavailable";
}

}
