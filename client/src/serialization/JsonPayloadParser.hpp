#pragma once

#include "common/Result.hpp"

#include <QByteArray>
#include <QJsonDocument>

namespace currency::client::serialization {

class JsonPayloadParser {
public:
    common::Result<QJsonDocument> parse(const QByteArray& payload) const;
};

}