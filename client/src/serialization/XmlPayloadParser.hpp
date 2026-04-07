#pragma once

#include "common/Result.hpp"

#include <QByteArray>
#include <QDomDocument>

namespace currency::client::serialization {

class XmlPayloadParser {
public:
    common::Result<QDomDocument> parse(const QByteArray& payload) const;
};

}