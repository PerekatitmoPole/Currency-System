#include "serialization/JsonPayloadParser.hpp"

#include "common/Errors.hpp"

#include <QJsonParseError>

namespace currency::client::serialization {

common::Result<QJsonDocument> JsonPayloadParser::parse(const QByteArray& payload) const {
    QJsonParseError error;
    const auto document = QJsonDocument::fromJson(payload, &error);
    if (error.error != QJsonParseError::NoError) {
        return common::Result<QJsonDocument>::failure(
            common::Errors::parseError("Failed to parse JSON payload", error.errorString()));
    }

    return common::Result<QJsonDocument>::success(document);
}

}