#include "common/Exceptions.hpp"
#include "serialization/TextProtocolSerializer.hpp"

#include <gtest/gtest.h>

namespace currency::tests {

TEST(TextProtocolSerializerTests, ParsesRequestEnvelope) {
    serialization::TextProtocolSerializer serializer;
    const auto request = serializer.parseRequestEnvelope(
        "command=get_rates;provider=ECB;base_code=EUR;quote_codes=USD,GBP");

    EXPECT_EQ(request.command, "get_rates");
    EXPECT_EQ(request.payload.at("provider"), "ECB");
    EXPECT_EQ(request.payload.at("base_code"), "EUR");
    EXPECT_EQ(request.payload.at("quote_codes"), "USD,GBP");
}

TEST(TextProtocolSerializerTests, ParsesIngestionBatch) {
    serialization::TextProtocolSerializer serializer;
    const auto request = serializer.parseRequestEnvelope(
        "command=ingest_quotes;provider=ECB;batch_timestamp=2026-03-19T09%3A00%3A00Z;quote_count=1;"
        "quote0_base_code=EUR;quote0_base_name=Euro;quote0_quote_code=USD;quote0_quote_name=US%20Dollar;"
        "quote0_rate=1.0825;quote0_source_timestamp=2026-03-19T09%3A00%3A00Z");

    const auto dto = serializer.parseUpdateQuotesRequest(request.payload);
    ASSERT_EQ(dto.quotes.size(), 1U);
    EXPECT_EQ(dto.provider, "ECB");
    EXPECT_EQ(dto.quotes.front().quoteName, "US Dollar");
    EXPECT_DOUBLE_EQ(dto.quotes.front().rate, 1.0825);
}

TEST(TextProtocolSerializerTests, CreatesSuccessEnvelope) {
    serialization::TextProtocolSerializer serializer;
    dto::ConvertResponseDto response{
        .provider = "ECB",
        .fromCurrency = "EUR",
        .toCurrency = "USD",
        .amount = 10.0,
        .rate = 1.08,
        .result = 10.8,
        .timestamp = "2026-03-19T09:00:00Z",
    };

    const auto raw = serializer.successResponse(serializer.toFields(response));
    EXPECT_NE(raw.find("status=ok"), std::string::npos);
    EXPECT_NE(raw.find("provider=ECB"), std::string::npos);
    EXPECT_NE(raw.find("from_currency=EUR"), std::string::npos);
}

TEST(TextProtocolSerializerTests, ParsesConvertRequest) {
    serialization::TextProtocolSerializer serializer;
    const auto envelope = serializer.parseRequestEnvelope(
        "command=convert;provider=ECB;from_currency=eur;to_currency=usd;amount=10.5");

    const auto request = serializer.parseConvertRequest(envelope.payload);
    EXPECT_EQ(request.provider, "ECB");
    EXPECT_EQ(request.fromCurrency, "eur");
    EXPECT_EQ(request.toCurrency, "usd");
    EXPECT_DOUBLE_EQ(request.amount, 10.5);
}

TEST(TextProtocolSerializerTests, ParsesGetRatesRequestWithMultipleQuotes) {
    serialization::TextProtocolSerializer serializer;
    const auto envelope = serializer.parseRequestEnvelope(
        "command=get_rates;provider=ECB;base_code=EUR;quote_codes=USD,GBP,CHF");

    const auto request = serializer.parseGetRatesRequest(envelope.payload);
    ASSERT_EQ(request.quoteCodes.size(), 3U);
    EXPECT_EQ(request.quoteCodes[0], "USD");
    EXPECT_EQ(request.quoteCodes[2], "CHF");
}

TEST(TextProtocolSerializerTests, ParsesGetHistoryRequest) {
    serialization::TextProtocolSerializer serializer;
    const auto envelope = serializer.parseRequestEnvelope(
        "command=get_history;provider=ECB;base_code=EUR;quote_code=USD;from=2026-03-19T09%3A00%3A00Z;"
        "to=2026-03-19T12%3A00%3A00Z;step=1h");

    const auto request = serializer.parseGetHistoryRequest(envelope.payload);
    EXPECT_EQ(request.provider, "ECB");
    EXPECT_EQ(request.baseCode, "EUR");
    EXPECT_EQ(request.quoteCode, "USD");
    EXPECT_EQ(request.step, "1h");
}

TEST(TextProtocolSerializerTests, SerializesUpdateQuotesResponseFields) {
    serialization::TextProtocolSerializer serializer;
    const dto::UpdateQuotesResponseDto response{
        .provider = "ECB",
        .processedAt = "2026-03-19T10:00:00Z",
        .acceptedCount = 2,
    };

    const auto fields = serializer.toFields(response);
    EXPECT_EQ(fields.at("provider"), "ECB");
    EXPECT_EQ(fields.at("processed_at"), "2026-03-19T10:00:00Z");
    EXPECT_EQ(fields.at("accepted_count"), "2");
}

TEST(TextProtocolSerializerTests, SerializesGetCurrenciesResponseFields) {
    serialization::TextProtocolSerializer serializer;
    const dto::GetCurrenciesResponseDto response{
        .currencies = {
            dto::CurrencyDto{"USD", "US Dollar", 2},
            dto::CurrencyDto{"JPY", "Japanese Yen", 0},
        },
    };

    const auto fields = serializer.toFields(response);
    EXPECT_EQ(fields.at("currency_count"), "2");
    EXPECT_EQ(fields.at("currency0_code"), "USD");
    EXPECT_EQ(fields.at("currency1_minor_units"), "0");
}

TEST(TextProtocolSerializerTests, SerializesGetRatesResponseFields) {
    serialization::TextProtocolSerializer serializer;
    const dto::GetRatesResponseDto response{
        .quotes = {
            dto::RateDto{"ECB", "EUR", "USD", 1.2, "2026-03-19T10:00:00Z"},
        },
    };

    const auto fields = serializer.toFields(response);
    EXPECT_EQ(fields.at("rate_count"), "1");
    EXPECT_EQ(fields.at("rate0_provider"), "ECB");
    EXPECT_EQ(fields.at("rate0_quote_code"), "USD");
}

TEST(TextProtocolSerializerTests, SerializesGetHistoryResponseFields) {
    serialization::TextProtocolSerializer serializer;
    const dto::GetHistoryResponseDto response{
        .provider = "ECB",
        .baseCode = "EUR",
        .quoteCode = "USD",
        .from = "2026-03-19T10:00:00Z",
        .to = "2026-03-19T12:00:00Z",
        .step = "1h",
        .points = {
            dto::HistoryPointDto{"2026-03-19T10:00:00Z", 1.1},
            dto::HistoryPointDto{"2026-03-19T11:00:00Z", 1.2},
        },
    };

    const auto fields = serializer.toFields(response);
    EXPECT_EQ(fields.at("point_count"), "2");
    EXPECT_EQ(fields.at("point1_timestamp"), "2026-03-19T11:00:00Z");
    EXPECT_EQ(fields.at("step"), "1h");
}

TEST(TextProtocolSerializerTests, CanonicalizeOrdersKeysLexicographically) {
    serialization::TextProtocolSerializer serializer;
    dto::FieldMap fields{{"zeta", "1"}, {"alpha", "2"}, {"mid", "3"}};

    EXPECT_EQ(serializer.canonicalize(fields), "alpha=2;mid=3;zeta=1");
}

TEST(TextProtocolSerializerTests, DecodesPercentEncodedCharactersInEnvelope) {
    serialization::TextProtocolSerializer serializer;
    const auto envelope = serializer.parseRequestEnvelope(
        "command=get_rates;provider=ECB;base_code=US%20D;quote_codes=CHF%2CJPY");

    EXPECT_EQ(envelope.payload.at("base_code"), "US D");
    EXPECT_EQ(envelope.payload.at("quote_codes"), "CHF,JPY");
}

TEST(TextProtocolSerializerTests, SuccessResponseEncodesReservedCharacters) {
    serialization::TextProtocolSerializer serializer;
    const dto::ErrorDto error{.code = "validation_error", .message = "bad=value;still bad"};

    const auto raw = serializer.errorResponse(error);
    EXPECT_NE(raw.find("message=bad%3Dvalue%3Bstill%20bad"), std::string::npos);
}

TEST(TextProtocolSerializerTests, MissingCommandThrowsProtocolError) {
    serialization::TextProtocolSerializer serializer;
    EXPECT_THROW(serializer.parseRequestEnvelope("provider=ECB"), common::ProtocolError);
}

TEST(TextProtocolSerializerTests, TokenWithoutEqualsThrowsProtocolError) {
    serialization::TextProtocolSerializer serializer;
    EXPECT_THROW(serializer.parseRequestEnvelope("command=get_rates;broken_token"), common::ProtocolError);
}

TEST(TextProtocolSerializerTests, EmptyKeyThrowsProtocolError) {
    serialization::TextProtocolSerializer serializer;
    EXPECT_THROW(serializer.parseRequestEnvelope("command=get_rates;=EUR"), common::ProtocolError);
}

TEST(TextProtocolSerializerTests, InvalidPercentEncodingThrowsProtocolError) {
    serialization::TextProtocolSerializer serializer;
    EXPECT_THROW(serializer.parseRequestEnvelope("command=get_rates;base_code=EU%ZZ"), common::ProtocolError);
}

TEST(TextProtocolSerializerTests, MissingRequiredFieldInConvertRequestThrowsProtocolError) {
    serialization::TextProtocolSerializer serializer;
    const dto::FieldMap payload{{"provider", "ECB"}, {"from_currency", "EUR"}, {"amount", "1.0"}};
    EXPECT_THROW(serializer.parseConvertRequest(payload), common::ProtocolError);
}

TEST(TextProtocolSerializerTests, InvalidDoubleInConvertRequestThrowsProtocolError) {
    serialization::TextProtocolSerializer serializer;
    const dto::FieldMap payload{{"provider", "ECB"}, {"from_currency", "EUR"}, {"to_currency", "USD"}, {"amount", "x"}};
    EXPECT_THROW(serializer.parseConvertRequest(payload), common::ProtocolError);
}

TEST(TextProtocolSerializerTests, InvalidQuoteCountThrowsProtocolError) {
    serialization::TextProtocolSerializer serializer;
    const dto::FieldMap payload{{"provider", "ECB"}, {"batch_timestamp", "2026-03-19T10:00:00Z"}, {"quote_count", "x"}};
    EXPECT_THROW(serializer.parseUpdateQuotesRequest(payload), common::ProtocolError);
}

TEST(TextProtocolSerializerTests, ParseGetRatesRequestDropsEmptyQuoteCodes) {
    serialization::TextProtocolSerializer serializer;
    const dto::FieldMap payload{{"provider", "ECB"}, {"base_code", "EUR"}, {"quote_codes", "USD,,JPY,"}};

    const auto request = serializer.parseGetRatesRequest(payload);
    ASSERT_EQ(request.quoteCodes.size(), 2U);
    EXPECT_EQ(request.quoteCodes[0], "USD");
    EXPECT_EQ(request.quoteCodes[1], "JPY");
}

}
