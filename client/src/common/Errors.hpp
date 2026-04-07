#pragma once

#include <QString>

namespace currency::client::common {

struct Error {
    QString code;
    QString message;
    QString details;
};

namespace Errors {

Error networkError(const QString& message, const QString& details = {});
Error protocolError(const QString& message, const QString& details = {});
Error parseError(const QString& message, const QString& details = {});
Error validationError(const QString& message, const QString& details = {});
Error unsupportedError(const QString& message, const QString& details = {});
Error internalError(const QString& message, const QString& details = {});

}

}