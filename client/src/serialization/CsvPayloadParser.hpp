#pragma once

#include "common/Result.hpp"

#include <QByteArray>
#include <QStringList>

namespace currency::client::serialization {


class CsvPayloadParser {
public:
    common::Result<QList<QStringList>> parse(const QByteArray& payload) const;
};

}