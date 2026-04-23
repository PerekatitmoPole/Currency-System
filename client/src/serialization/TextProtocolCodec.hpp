#pragma once

#include "common/Result.hpp"
#include "dto/ServerDtos.hpp"

#include <QString>

namespace currency::client::serialization {

class TextProtocolCodec {
public:
    QString encodeRequest(const QString& command, const dto::FieldMap& payload) const;
    common::Result<dto::ServerEnvelopeDto> decodeEnvelope(const QString& rawMessage) const;

    dto::FieldMap toFields(const dto::GetRatesRequestDto& request) const;
    dto::FieldMap toFields(const dto::GetHistoryRequestDto& request) const;
    dto::FieldMap toFields(const dto::ConvertRequestDto& request) const;

private:
    static dto::FieldMap parseFields(const QString& rawMessage);
    static QString serializeFields(const dto::FieldMap& fields);
    static QString encode(const QString& value);
    static QString decode(const QString& value);
};

}