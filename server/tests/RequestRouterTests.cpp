#include "common/Exceptions.hpp"
#include "TestHelpers.hpp"

#include <gtest/gtest.h>

namespace currency::tests {

TEST(RequestRouterTests, RoutesConvertCommand) {
    TestAppContext context;
    seedQuote(context.quoteRepository, "ECB", "EUR", "USD", 1.20, "2026-03-19T12:00:00Z");
    const auto raw = context.router.handle("command=convert;provider=ECB;from_currency=EUR;to_currency=USD;amount=10");
    const auto fields = parseEnvelopePayload(context.serializer, raw);
    EXPECT_EQ(requireField(fields, "status"), "ok");
    EXPECT_EQ(requireField(fields, "result"), "12.000000");
}

TEST(RequestRouterTests, RoutesGetCurrenciesCommand) {
    TestAppContext context;
    const auto raw = context.router.handle("command=get_currencies");
    const auto fields = parseEnvelopePayload(context.serializer, raw);
    EXPECT_EQ(requireField(fields, "status"), "ok");
    EXPECT_EQ(requireField(fields, "currency_count"), "7");
}

TEST(RequestRouterTests, RoutesGetRatesCommand) {
    TestAppContext context;
    seedQuote(context.quoteRepository, "ECB", "EUR", "USD", 1.20, "2026-03-19T12:00:00Z");
    const auto raw = context.router.handle("command=get_rates;provider=ECB;base_code=EUR;quote_codes=USD");
    const auto fields = parseEnvelopePayload(context.serializer, raw);
    EXPECT_EQ(requireField(fields, "status"), "ok");
    EXPECT_EQ(requireField(fields, "rate_count"), "1");
}

TEST(RequestRouterTests, RoutesGetHistoryCommand) {
    TestAppContext context;
    context.historyRepository.append("ECB", "EUR", "USD", {common::fromIsoString("2026-03-19T10:00:00Z"), 1.1});
    const auto raw = context.router.handle(
        "command=get_history;provider=ECB;base_code=EUR;quote_code=USD;from=2026-03-19T09%3A00%3A00Z;to=2026-03-19T11%3A00%3A00Z;step=1h");
    const auto fields = parseEnvelopePayload(context.serializer, raw);
    EXPECT_EQ(requireField(fields, "status"), "ok");
    EXPECT_EQ(requireField(fields, "point_count"), "1");
}

TEST(RequestRouterTests, RoutesIngestionCommand) {
    TestAppContext context;
    const auto raw = context.router.handle(
        "command=ingest_quotes;provider=ECB;batch_timestamp=2026-03-19T10%3A00%3A00Z;quote_count=1;"
        "quote0_base_code=EUR;quote0_base_name=Euro;quote0_quote_code=USD;quote0_quote_name=US%20Dollar;"
        "quote0_rate=1.15;quote0_source_timestamp=2026-03-19T10%3A00%3A00Z");
    const auto fields = parseEnvelopePayload(context.serializer, raw);
    EXPECT_EQ(requireField(fields, "status"), "ok");
    EXPECT_EQ(requireField(fields, "accepted_count"), "1");
}

TEST(RequestRouterTests, ReturnsErrorForUnknownCommand) {
    TestAppContext context;
    const auto raw = context.router.handle("command=unknown");
    const auto fields = parseEnvelopePayload(context.serializer, raw);
    EXPECT_EQ(requireField(fields, "status"), "error");
    EXPECT_EQ(requireField(fields, "code"), "unknown_command");
}

TEST(RequestRouterTests, ReturnsProtocolErrorForMalformedRequest) {
    TestAppContext context;
    const auto raw = context.router.handle("command");
    const auto fields = parseEnvelopePayload(context.serializer, raw);
    EXPECT_EQ(requireField(fields, "status"), "error");
    EXPECT_EQ(requireField(fields, "code"), "protocol_error");
}

TEST(RequestRouterTests, ConvertsServiceExceptionsToErrorEnvelope) {
    TestAppContext context;
    const auto raw = context.router.handle("command=convert;provider=ECB;from_currency=EUR;to_currency=USD;amount=10");
    const auto fields = parseEnvelopePayload(context.serializer, raw);
    EXPECT_EQ(requireField(fields, "status"), "error");
    EXPECT_EQ(requireField(fields, "code"), "not_found");
}

}
