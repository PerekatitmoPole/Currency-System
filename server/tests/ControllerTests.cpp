#include "common/Exceptions.hpp"
#include "common/TimeUtils.hpp"
#include "TestHelpers.hpp"

#include <gtest/gtest.h>

namespace currency::tests {

TEST(CurrencyControllerTests, ReturnsCurrenciesResponse) {
    TestAppContext context;
    const auto raw = context.currencyController.handle({});
    const auto fields = parseEnvelopePayload(context.serializer, raw);
    EXPECT_EQ(requireField(fields, "status"), "ok");
    EXPECT_EQ(requireField(fields, "currency0_code"), "CHF");
}

TEST(CurrencyControllerTests, UsesCacheForRepeatedRequests) {
    TestAppContext context;
    const auto first = context.currencyController.handle({});
    context.currencyRepository.upsert(domain::Currency{"BTC", "Bitcoin", 8});
    const auto second = context.currencyController.handle({});
    EXPECT_EQ(first, second);
}

TEST(IngestionControllerTests, ReturnsSuccessForValidBatch) {
    TestAppContext context;
    const dto::FieldMap payload{
        {"provider", "ECB"},
        {"batch_timestamp", "2026-03-19T10:00:00Z"},
        {"quote_count", "1"},
        {"quote0_base_code", "EUR"},
        {"quote0_base_name", "Euro"},
        {"quote0_quote_code", "USD"},
        {"quote0_quote_name", "US Dollar"},
        {"quote0_rate", "1.12"},
        {"quote0_source_timestamp", "2026-03-19T10:00:00Z"},
    };

    const auto raw = context.ingestionController.handle(payload);
    const auto fields = parseEnvelopePayload(context.serializer, raw);
    EXPECT_EQ(requireField(fields, "status"), "ok");
    EXPECT_EQ(requireField(fields, "accepted_count"), "1");
}

TEST(IngestionControllerTests, InvalidBatchThrowsOutOfController) {
    TestAppContext context;
    const dto::FieldMap payload{
        {"provider", "ECB"},
        {"batch_timestamp", "2026-03-19T10:00:00Z"},
        {"quote_count", "1"},
        {"quote0_base_code", "EUR"},
        {"quote0_base_name", "Euro"},
        {"quote0_quote_code", "USD"},
        {"quote0_quote_name", "US Dollar"},
        {"quote0_rate", "-1.12"},
        {"quote0_source_timestamp", "2026-03-19T10:00:00Z"},
    };

    EXPECT_THROW(context.ingestionController.handle(payload), common::ValidationError);
}

TEST(QuoteControllerTests, ReturnsQuotesForValidRequest) {
    TestAppContext context;
    seedQuote(context.quoteRepository, "ECB", "EUR", "USD", 1.15, "2026-03-19T10:00:00Z");
    const dto::FieldMap payload{{"provider", "ECB"}, {"base_code", "EUR"}, {"quote_codes", "USD"}};

    const auto raw = context.quoteController.handle(payload);
    const auto fields = parseEnvelopePayload(context.serializer, raw);
    EXPECT_EQ(requireField(fields, "status"), "ok");
    EXPECT_EQ(requireField(fields, "rate_count"), "1");
    EXPECT_EQ(requireField(fields, "rate0_quote_code"), "USD");
}

TEST(QuoteControllerTests, UsesCacheForRepeatedRequests) {
    TestAppContext context;
    seedQuote(context.quoteRepository, "ECB", "EUR", "USD", 1.15, "2026-03-19T10:00:00Z");
    const dto::FieldMap payload{{"provider", "ECB"}, {"base_code", "EUR"}, {"quote_codes", "USD"}};

    const auto first = context.quoteController.handle(payload);
    seedQuote(context.quoteRepository, "ECB", "EUR", "USD", 1.30, "2026-03-19T11:00:00Z");
    const auto second = context.quoteController.handle(payload);
    EXPECT_EQ(first, second);
}

TEST(QuoteControllerTests, InvalidPayloadThrowsOutOfController) {
    TestAppContext context;
    const dto::FieldMap payload{{"provider", "ECB"}, {"base_code", "EUR"}};
    EXPECT_THROW(context.quoteController.handle(payload), common::ProtocolError);
}

TEST(HistoryControllerTests, ReturnsHistoryForValidRequest) {
    TestAppContext context;
    context.historyRepository.append("ECB", "EUR", "USD", {common::fromIsoString("2026-03-19T10:00:00Z"), 1.1});
    const dto::FieldMap payload{{"provider", "ECB"},
                                {"base_code", "EUR"},
                                {"quote_code", "USD"},
                                {"from", "2026-03-19T09:00:00Z"},
                                {"to", "2026-03-19T11:00:00Z"},
                                {"step", "1h"}};

    const auto raw = context.historyController.handle(payload);
    const auto fields = parseEnvelopePayload(context.serializer, raw);
    EXPECT_EQ(requireField(fields, "status"), "ok");
    EXPECT_EQ(requireField(fields, "point_count"), "1");
}

TEST(HistoryControllerTests, UsesCacheForRepeatedRequests) {
    TestAppContext context;
    context.historyRepository.append("ECB", "EUR", "USD", {common::fromIsoString("2026-03-19T10:00:00Z"), 1.1});
    const dto::FieldMap payload{{"provider", "ECB"},
                                {"base_code", "EUR"},
                                {"quote_code", "USD"},
                                {"from", "2026-03-19T09:00:00Z"},
                                {"to", "2026-03-19T11:00:00Z"},
                                {"step", "1h"}};

    const auto first = context.historyController.handle(payload);
    context.historyRepository.append("ECB", "EUR", "USD", {common::fromIsoString("2026-03-19T10:30:00Z"), 1.2});
    const auto second = context.historyController.handle(payload);
    EXPECT_EQ(first, second);
}

TEST(ConversionControllerTests, ReturnsConvertedAmount) {
    TestAppContext context;
    seedQuote(context.quoteRepository, "ECB", "EUR", "USD", 1.20, "2026-03-19T12:00:00Z");
    const dto::FieldMap payload{{"provider", "ECB"}, {"from_currency", "EUR"}, {"to_currency", "USD"}, {"amount", "10.0"}};

    const auto raw = context.conversionController.handle(payload);
    const auto fields = parseEnvelopePayload(context.serializer, raw);
    EXPECT_EQ(requireField(fields, "status"), "ok");
    EXPECT_EQ(requireField(fields, "result"), "12.000000");
}

TEST(ConversionControllerTests, MissingFieldThrowsOutOfController) {
    TestAppContext context;
    const dto::FieldMap payload{{"provider", "ECB"}, {"from_currency", "EUR"}, {"amount", "10.0"}};
    EXPECT_THROW(context.conversionController.handle(payload), common::ProtocolError);
}

}
