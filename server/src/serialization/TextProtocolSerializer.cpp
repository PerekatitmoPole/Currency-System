#include "serialization/TextProtocolSerializer.hpp"

#include "common/Exceptions.hpp"

#include <charconv>
#include <cctype>
#include <iomanip>
#include <sstream>
#include <stdexcept>

namespace currency::serialization {

namespace {

std::string makeIndexedKey(const std::string& prefix, const std::size_t index, const std::string& field) {
    return prefix + std::to_string(index) + "_" + field;
}

}

dto::RequestEnvelopeDto TextProtocolSerializer::parseRequestEnvelope(const std::string& rawMessage) const {
    const auto fields = parseFields(rawMessage);

    dto::RequestEnvelopeDto request;
    request.command = readString(fields, "command");
    request.payload = fields;
    request.payload.erase("command");
    return request;
}

std::string TextProtocolSerializer::successResponse(const dto::FieldMap& payload) const {
    dto::FieldMap envelope = payload;
    envelope.insert_or_assign("status", "ok");
    return serializeFields(envelope);
}

std::string TextProtocolSerializer::errorResponse(const dto::ErrorDto& error) const {
    dto::FieldMap envelope;
    envelope["status"] = "error";
    envelope["code"] = error.code;
    envelope["message"] = error.message;
    return serializeFields(envelope);
}

dto::UpdateQuotesRequestDto TextProtocolSerializer::parseUpdateQuotesRequest(const dto::FieldMap& payload) const {
    dto::UpdateQuotesRequestDto request;
    request.provider = readString(payload, "provider");
    request.batchTimestamp = readString(payload, "batch_timestamp");

    const auto quoteCount = readSize(payload, "quote_count");
    request.quotes.reserve(quoteCount);

    for (std::size_t index = 0; index < quoteCount; ++index) {
        request.quotes.push_back(dto::QuoteSnapshotDto{
            .baseCode = readString(payload, makeIndexedKey("quote", index, "base_code")),
            .baseName = readString(payload, makeIndexedKey("quote", index, "base_name")),
            .quoteCode = readString(payload, makeIndexedKey("quote", index, "quote_code")),
            .quoteName = readString(payload, makeIndexedKey("quote", index, "quote_name")),
            .rate = readDouble(payload, makeIndexedKey("quote", index, "rate")),
            .sourceTimestamp = readString(payload, makeIndexedKey("quote", index, "source_timestamp")),
        });
    }

    return request;
}

dto::GetRatesRequestDto TextProtocolSerializer::parseGetRatesRequest(const dto::FieldMap& payload) const {
    dto::GetRatesRequestDto request;
    request.provider = readString(payload, "provider");
    request.baseCode = readString(payload, "base_code");
    request.quoteCodes = splitList(readString(payload, "quote_codes"), ',');
    return request;
}

dto::GetHistoryRequestDto TextProtocolSerializer::parseGetHistoryRequest(const dto::FieldMap& payload) const {
    return dto::GetHistoryRequestDto{
        .provider = readString(payload, "provider"),
        .baseCode = readString(payload, "base_code"),
        .quoteCode = readString(payload, "quote_code"),
        .from = readString(payload, "from"),
        .to = readString(payload, "to"),
        .step = readString(payload, "step"),
    };
}

dto::ConvertRequestDto TextProtocolSerializer::parseConvertRequest(const dto::FieldMap& payload) const {
    return dto::ConvertRequestDto{
        .provider = readString(payload, "provider"),
        .fromCurrency = readString(payload, "from_currency"),
        .toCurrency = readString(payload, "to_currency"),
        .amount = readDouble(payload, "amount"),
    };
}

dto::FieldMap TextProtocolSerializer::toFields(const dto::UpdateQuotesResponseDto& response) const {
    dto::FieldMap fields;
    fields["provider"] = response.provider;
    fields["processed_at"] = response.processedAt;
    fields["accepted_count"] = std::to_string(response.acceptedCount);
    return fields;
}

dto::FieldMap TextProtocolSerializer::toFields(const dto::GetCurrenciesResponseDto& response) const {
    dto::FieldMap fields;
    fields["currency_count"] = std::to_string(response.currencies.size());
    for (std::size_t index = 0; index < response.currencies.size(); ++index) {
        const auto& currency = response.currencies[index];
        fields[makeIndexedKey("currency", index, "code")] = currency.code;
        fields[makeIndexedKey("currency", index, "name")] = currency.name;
        fields[makeIndexedKey("currency", index, "minor_units")] = std::to_string(currency.minorUnits);
    }
    return fields;
}

dto::FieldMap TextProtocolSerializer::toFields(const dto::GetRatesResponseDto& response) const {
    dto::FieldMap fields;
    fields["rate_count"] = std::to_string(response.quotes.size());
    for (std::size_t index = 0; index < response.quotes.size(); ++index) {
        const auto& quote = response.quotes[index];
        fields[makeIndexedKey("rate", index, "provider")] = quote.provider;
        fields[makeIndexedKey("rate", index, "base_code")] = quote.baseCode;
        fields[makeIndexedKey("rate", index, "quote_code")] = quote.quoteCode;
        fields[makeIndexedKey("rate", index, "rate")] = std::to_string(quote.rate);
        fields[makeIndexedKey("rate", index, "source_timestamp")] = quote.sourceTimestamp;
    }
    return fields;
}

dto::FieldMap TextProtocolSerializer::toFields(const dto::GetHistoryResponseDto& response) const {
    dto::FieldMap fields;
    fields["provider"] = response.provider;
    fields["base_code"] = response.baseCode;
    fields["quote_code"] = response.quoteCode;
    fields["from"] = response.from;
    fields["to"] = response.to;
    fields["step"] = response.step;
    fields["point_count"] = std::to_string(response.points.size());

    for (std::size_t index = 0; index < response.points.size(); ++index) {
        const auto& point = response.points[index];
        fields[makeIndexedKey("point", index, "timestamp")] = point.timestamp;
        fields[makeIndexedKey("point", index, "rate")] = std::to_string(point.rate);
    }

    return fields;
}

dto::FieldMap TextProtocolSerializer::toFields(const dto::ConvertResponseDto& response) const {
    dto::FieldMap fields;
    fields["provider"] = response.provider;
    fields["from_currency"] = response.fromCurrency;
    fields["to_currency"] = response.toCurrency;
    fields["amount"] = std::to_string(response.amount);
    fields["rate"] = std::to_string(response.rate);
    fields["result"] = std::to_string(response.result);
    fields["timestamp"] = response.timestamp;
    return fields;
}

std::string TextProtocolSerializer::canonicalize(const dto::FieldMap& payload) const {
    return serializeFields(payload);
}

dto::FieldMap TextProtocolSerializer::parseFields(const std::string& rawMessage) {
    dto::FieldMap fields;
    std::size_t start = 0;

    while (start < rawMessage.size()) {
        const auto separator = rawMessage.find(';', start);
        const auto token = rawMessage.substr(start, separator == std::string::npos ? std::string::npos : separator - start);

        if (!token.empty()) {
            const auto equalSign = token.find('=');
            if (equalSign == std::string::npos) {
                throw common::ProtocolError("Each token must be in key=value format");
            }

            const auto key = token.substr(0, equalSign);
            const auto value = token.substr(equalSign + 1);
            if (key.empty()) {
                throw common::ProtocolError("Field key must not be empty");
            }

            fields[key] = decode(value);
        }

        if (separator == std::string::npos) {
            break;
        }

        start = separator + 1;
    }

    return fields;
}

std::string TextProtocolSerializer::serializeFields(const dto::FieldMap& fields) {
    std::ostringstream output;
    bool first = true;
    for (const auto& [key, value] : fields) {
        if (!first) {
            output << ';';
        }
        first = false;
        output << key << '=' << encode(value);
    }
    return output.str();
}

std::string TextProtocolSerializer::encode(const std::string& value) {
    std::ostringstream output;
    output << std::uppercase << std::hex;

    for (const unsigned char character : value) {
        if (std::isalnum(character) != 0 || character == '-' || character == '_' || character == '.' || character == '~' || character == ',') {
            output << static_cast<char>(character);
        } else {
            output << '%' << std::setw(2) << std::setfill('0') << static_cast<int>(character);
        }
    }

    return output.str();
}

std::string TextProtocolSerializer::decode(const std::string& value) {
    std::string result;
    result.reserve(value.size());

    for (std::size_t index = 0; index < value.size(); ++index) {
        if (value[index] == '%') {
            if (index + 2 >= value.size()) {
                throw common::ProtocolError("Invalid percent-encoding in payload");
            }

            const auto hex = value.substr(index + 1, 2);
            unsigned int decoded = 0;
            std::istringstream input(hex);
            input >> std::hex >> decoded;
            if (input.fail()) {
                throw common::ProtocolError("Invalid percent-encoding in payload");
            }

            result.push_back(static_cast<char>(decoded));
            index += 2;
            continue;
        }

        result.push_back(value[index]);
    }

    return result;
}

std::string TextProtocolSerializer::readString(const dto::FieldMap& payload, const std::string& key) {
    const auto iterator = payload.find(key);
    if (iterator == payload.end()) {
        throw common::ProtocolError("Required field '" + key + "' is missing");
    }
    return iterator->second;
}

double TextProtocolSerializer::readDouble(const dto::FieldMap& payload, const std::string& key) {
    const auto raw = readString(payload, key);
    double value = 0.0;
    const auto* begin = raw.data();
    const auto* end = raw.data() + raw.size();
    const auto result = std::from_chars(begin, end, value);
    if (result.ec != std::errc() || result.ptr != end) {
        throw common::ProtocolError("Field '" + key + "' must be a valid number");
    }
    return value;
}

std::size_t TextProtocolSerializer::readSize(const dto::FieldMap& payload, const std::string& key) {
    const auto raw = readString(payload, key);
    std::size_t value = 0;
    const auto* begin = raw.data();
    const auto* end = raw.data() + raw.size();
    const auto result = std::from_chars(begin, end, value);
    if (result.ec != std::errc() || result.ptr != end) {
        throw common::ProtocolError("Field '" + key + "' must be a valid integer");
    }
    return value;
}

std::vector<std::string> TextProtocolSerializer::splitList(const std::string& value, const char delimiter) {
    std::vector<std::string> items;
    std::size_t start = 0;

    while (start <= value.size()) {
        const auto separator = value.find(delimiter, start);
        const auto token = value.substr(start, separator == std::string::npos ? std::string::npos : separator - start);
        if (!token.empty()) {
            items.push_back(token);
        }

        if (separator == std::string::npos) {
            break;
        }
        start = separator + 1;
    }

    return items;
}

std::string TextProtocolSerializer::joinList(const std::vector<std::string>& values, const char delimiter) {
    std::ostringstream output;
    for (std::size_t index = 0; index < values.size(); ++index) {
        if (index != 0) {
            output << delimiter;
        }
        output << values[index];
    }
    return output.str();
}

}
