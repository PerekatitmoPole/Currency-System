#include "serialization/TextProtocolCodec.hpp"

#include "common/DateTimeUtils.hpp"
#include "common/Errors.hpp"

#include <QUrl>
#include <stdexcept>

namespace currency::client::serialization {

QString TextProtocolCodec::encodeRequest(const QString& command, const dto::FieldMap& payload) const {
    auto fields = payload;
    fields.insert("command", command);
    return serializeFields(fields);
}

common::Result<dto::ServerEnvelopeDto> TextProtocolCodec::decodeEnvelope(const QString& rawMessage) const {
    try {
        auto fields = parseFields(rawMessage);
        dto::ServerEnvelopeDto envelope;
        envelope.status = fields.take("status");
        envelope.errorCode = fields.take("code");
        envelope.errorMessage = fields.take("message");
        envelope.fields = fields;
        return common::Result<dto::ServerEnvelopeDto>::success(envelope);
    } catch (const std::exception& error) {
        return common::Result<dto::ServerEnvelopeDto>::failure(
            common::Errors::protocolError("Failed to decode server response", error.what()));
    }
}

dto::FieldMap TextProtocolCodec::toFields(const dto::GetRatesRequestDto& request) const {
    dto::FieldMap fields;
    fields.insert("provider", request.provider);
    fields.insert("base_code", request.baseCode);
    fields.insert("quote_codes", request.quoteCodes.join(','));
    return fields;
}

dto::FieldMap TextProtocolCodec::toFields(const dto::GetHistoryRequestDto& request) const {
    dto::FieldMap fields;
    fields.insert("provider", request.provider);
    fields.insert("base_code", request.baseCode);
    fields.insert("quote_code", request.quoteCode);
    fields.insert("from", common::DateTimeUtils::toIsoUtc(request.from));
    fields.insert("to", common::DateTimeUtils::toIsoUtc(request.to));
    fields.insert("step", request.step);
    return fields;
}

dto::FieldMap TextProtocolCodec::toFields(const dto::ConvertRequestDto& request) const {
    dto::FieldMap fields;
    fields.insert("provider", request.provider);
    fields.insert("from_currency", request.fromCurrency);
    fields.insert("to_currency", request.toCurrency);
    fields.insert("amount", QString::number(request.amount, 'f', 6));
    return fields;
}

dto::FieldMap TextProtocolCodec::parseFields(const QString& rawMessage) {
    dto::FieldMap fields;
    const auto tokens = rawMessage.split(';', Qt::SkipEmptyParts);
    for (const auto& token : tokens) {
        const auto separatorIndex = token.indexOf('=');
        if (separatorIndex <= 0) {
            throw std::runtime_error("Malformed protocol token");
        }

        const auto key = token.left(separatorIndex);
        const auto value = token.mid(separatorIndex + 1);
        fields.insert(key, decode(value));
    }

    return fields;
}

QString TextProtocolCodec::serializeFields(const dto::FieldMap& fields) {
    QStringList tokens;
    for (auto iterator = fields.cbegin(); iterator != fields.cend(); ++iterator) {
        tokens.push_back(iterator.key() + '=' + encode(iterator.value()));
    }
    return tokens.join(';');
}

QString TextProtocolCodec::encode(const QString& value) {
    return QString::fromLatin1(QUrl::toPercentEncoding(value, QByteArray("-_.~,")));
}

QString TextProtocolCodec::decode(const QString& value) {
    return QUrl::fromPercentEncoding(value.toLatin1());
}

}