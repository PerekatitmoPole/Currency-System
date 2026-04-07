#include "serialization/XmlPayloadParser.hpp"

#include "common/Errors.hpp"

namespace currency::client::serialization {

common::Result<QDomDocument> XmlPayloadParser::parse(const QByteArray& payload) const {
    QDomDocument document;
    QString errorMessage;
    int errorLine = 0;
    int errorColumn = 0;
    if (!document.setContent(payload, &errorMessage, &errorLine, &errorColumn)) {
        return common::Result<QDomDocument>::failure(common::Errors::parseError(
            "Failed to parse XML payload",
            QString("Line %1, column %2: %3").arg(errorLine).arg(errorColumn).arg(errorMessage)));
    }

    return common::Result<QDomDocument>::success(document);
}

}