#include "ClientTestSupport.hpp"
#include "serialization/TextProtocolCodec.hpp"
#include "services/ApiOrchestratorService.hpp"

#include <gtest/gtest.h>

namespace currency::client::tests {

TEST(ApiOrchestratorServiceTests, FetchCurrenciesSortsCatalog) {
    MockLineServer server({makeEnvelope("ok", {
        {"currency_count", "2"},
        {"currency0_code", "USD"},
        {"currency0_name", "US Dollar"},
        {"currency0_minor_units", "2"},
        {"currency1_code", "EUR"},
        {"currency1_name", "Euro"},
        {"currency1_minor_units", "2"},
    })});

    serialization::TextProtocolCodec codec;
    gateways::ServerGateway gateway(codec);
    ASSERT_TRUE(connectGateway(gateway, server.port()).ok());

    services::ApiOrchestratorService orchestrator(gateway);
    const auto result = orchestrator.fetchCurrencies();

    ASSERT_TRUE(result.ok());
    ASSERT_EQ(result.value().size(), 2);
    EXPECT_EQ(result.value().at(0).code, QString("EUR"));
    EXPECT_EQ(result.value().at(1).code, QString("USD"));
}

TEST(ApiOrchestratorServiceTests, FetchCurrenciesPassesProviderToServer) {
    MockLineServer server({makeEnvelope("ok", {
        {"currency_count", "1"},
        {"currency0_code", "RUB"},
        {"currency0_name", "Russian Ruble"},
        {"currency0_minor_units", "2"},
    })});

    serialization::TextProtocolCodec codec;
    gateways::ServerGateway gateway(codec);
    ASSERT_TRUE(connectGateway(gateway, server.port()).ok());

    services::ApiOrchestratorService orchestrator(gateway);
    const auto result = orchestrator.fetchCurrencies(dto::ApiSource::Cbr);

    ASSERT_TRUE(result.ok());
    ASSERT_EQ(result.value().size(), 1);
    ASSERT_TRUE(waitUntil([&server] { return !server.receivedLines().isEmpty(); }, 1000));
    EXPECT_TRUE(server.receivedLines().first().contains("provider=cbr"));
}

TEST(ApiOrchestratorServiceTests, FetchCurrenciesFailsWhenCurrencyCodeIsMissing) {
    MockLineServer server({makeEnvelope("ok", {
        {"currency_count", "1"},
        {"currency0_name", "No code"},
        {"currency0_minor_units", "2"},
    })});

    serialization::TextProtocolCodec codec;
    gateways::ServerGateway gateway(codec);
    ASSERT_TRUE(connectGateway(gateway, server.port()).ok());

    services::ApiOrchestratorService orchestrator(gateway);
    const auto result = orchestrator.fetchCurrencies();

    ASSERT_FALSE(result.ok());
    EXPECT_EQ(result.error().code, QString("protocol_error"));
}

TEST(ApiOrchestratorServiceTests, FetchHistoryValidatesDateRange) {
    serialization::TextProtocolCodec codec;
    gateways::ServerGateway gateway(codec);
    services::ApiOrchestratorService orchestrator(gateway);

    const auto result = orchestrator.fetchHistory(
        dto::ApiSource::Ecb,
        "EUR",
        "USD",
        QDate(),
        QDate(2026, 4, 18),
        "1d");

    ASSERT_FALSE(result.ok());
    EXPECT_EQ(result.error().code, QString("validation_error"));
}

TEST(ApiOrchestratorServiceTests, FetchHistoryRejectsMalformedRate) {
    MockLineServer server({makeEnvelope("ok", {
        {"point_count", "1"},
        {"base_code", "EUR"},
        {"quote_code", "USD"},
        {"point0_rate", "oops"},
        {"point0_timestamp", "2026-04-18T10:30:00Z"},
    })});

    serialization::TextProtocolCodec codec;
    gateways::ServerGateway gateway(codec);
    ASSERT_TRUE(connectGateway(gateway, server.port()).ok());

    services::ApiOrchestratorService orchestrator(gateway);
    const auto result = orchestrator.fetchHistory(
        dto::ApiSource::Ecb,
        "EUR",
        "USD",
        QDate(2026, 4, 17),
        QDate(2026, 4, 18),
        "1d");

    ASSERT_FALSE(result.ok());
    EXPECT_EQ(result.error().code, QString("protocol_error"));
}

TEST(ApiOrchestratorServiceTests, ConvertParsesResponse) {
    MockLineServer server({makeEnvelope("ok", {
        {"provider", "ecb"},
        {"from_currency", "EUR"},
        {"to_currency", "USD"},
        {"rate", "1.25"},
        {"result", "12.50"},
        {"timestamp", "2026-04-18T10:30:00Z"},
        {"stale", "true"},
        {"warning", "cached"},
    })});

    serialization::TextProtocolCodec codec;
    gateways::ServerGateway gateway(codec);
    ASSERT_TRUE(connectGateway(gateway, server.port()).ok());

    services::ApiOrchestratorService orchestrator(gateway);
    const auto result = orchestrator.convert(dto::ApiSource::Ecb, "EUR", "USD", 10.0);

    ASSERT_TRUE(result.ok());
    EXPECT_EQ(result.value().providerKey, QString("ecb"));
    EXPECT_EQ(result.value().fromCurrency, QString("EUR"));
    EXPECT_EQ(result.value().toCurrency, QString("USD"));
    EXPECT_TRUE(result.value().stale);
    EXPECT_EQ(result.value().warning, QString("cached"));
    EXPECT_DOUBLE_EQ(result.value().result, 12.5);
}

TEST(ApiOrchestratorServiceTests, ConvertRejectsMalformedResult) {
    MockLineServer server({makeEnvelope("ok", {
        {"provider", "ecb"},
        {"from_currency", "EUR"},
        {"to_currency", "USD"},
        {"rate", "1.25"},
        {"result", "not_a_number"},
        {"timestamp", "2026-04-18T10:30:00Z"},
    })});

    serialization::TextProtocolCodec codec;
    gateways::ServerGateway gateway(codec);
    ASSERT_TRUE(connectGateway(gateway, server.port()).ok());

    services::ApiOrchestratorService orchestrator(gateway);
    const auto result = orchestrator.convert(dto::ApiSource::Ecb, "EUR", "USD", 10.0);

    ASSERT_FALSE(result.ok());
    EXPECT_EQ(result.error().code, QString("protocol_error"));
}

}
