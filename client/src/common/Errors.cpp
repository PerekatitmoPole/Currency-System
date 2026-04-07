#include "common/Errors.hpp"

namespace currency::client::common::Errors {

namespace {

Error makeError(const QString& code, const QString& message, const QString& details) {
    return Error{
        .code = code,
        .message = message,
        .details = details,
    };
}

}

Error networkError(const QString& message, const QString& details) {
    return makeError("network_error", message, details);
}

Error protocolError(const QString& message, const QString& details) {
    return makeError("protocol_error", message, details);
}

Error parseError(const QString& message, const QString& details) {
    return makeError("parse_error", message, details);
}

Error validationError(const QString& message, const QString& details) {
    return makeError("validation_error", message, details);
}

Error unsupportedError(const QString& message, const QString& details) {
    return makeError("unsupported_error", message, details);
}

Error internalError(const QString& message, const QString& details) {
    return makeError("internal_error", message, details);
}

}