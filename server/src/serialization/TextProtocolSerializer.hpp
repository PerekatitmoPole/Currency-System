#pragma once

#include "dto/ProtocolDtos.hpp"
#include "dto/QueryDtos.hpp"
#include "dto/UpdateDtos.hpp"

#include <string>
#include <vector>

namespace currency::serialization {

class TextProtocolSerializer {
public:
    dto::RequestEnvelopeDto parseRequestEnvelope(const std::string& rawMessage) const;

    std::string successResponse(const dto::FieldMap& payload) const;
    std::string errorResponse(const dto::ErrorDto& error) const;

    dto::UpdateQuotesRequestDto parseUpdateQuotesRequest(const dto::FieldMap& payload) const;
    dto::GetRatesRequestDto parseGetRatesRequest(const dto::FieldMap& payload) const;
    dto::GetHistoryRequestDto parseGetHistoryRequest(const dto::FieldMap& payload) const;
    dto::ConvertRequestDto parseConvertRequest(const dto::FieldMap& payload) const;

    dto::FieldMap toFields(const dto::UpdateQuotesResponseDto& response) const;
    dto::FieldMap toFields(const dto::GetCurrenciesResponseDto& response) const;
    dto::FieldMap toFields(const dto::GetRatesResponseDto& response) const;
    dto::FieldMap toFields(const dto::GetHistoryResponseDto& response) const;
    dto::FieldMap toFields(const dto::ConvertResponseDto& response) const;

    std::string canonicalize(const dto::FieldMap& payload) const;

private:
    static dto::FieldMap parseFields(const std::string& rawMessage);
    static std::string serializeFields(const dto::FieldMap& fields);
    static std::string encode(const std::string& value);
    static std::string decode(const std::string& value);
    static std::string readString(const dto::FieldMap& payload, const std::string& key);
    static double readDouble(const dto::FieldMap& payload, const std::string& key);
    static std::size_t readSize(const dto::FieldMap& payload, const std::string& key);
    static std::vector<std::string> splitList(const std::string& value, char delimiter);
    static std::string joinList(const std::vector<std::string>& values, char delimiter);
};

}
