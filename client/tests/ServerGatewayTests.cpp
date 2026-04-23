#include "ClientTestSupport.hpp"
#include "serialization/TextProtocolCodec.hpp"

#include <QTcpServer>
#include <gtest/gtest.h>

namespace currency::client::tests {

TEST(ServerGatewayTests, RejectsMissingEndpoint) {
    serialization::TextProtocolCodec codec;
    gateways::ServerGateway gateway(codec);

    const auto result = gateway.connectToServer({}, 0);

    ASSERT_FALSE(result.ok());
    EXPECT_EQ(result.error().code, QString("validation_error"));
}

TEST(ServerGatewayTests, ReportsRefusedConnection) {
    QTcpServer probe;
    ASSERT_TRUE(probe.listen(QHostAddress::LocalHost, 0));
    const auto port = probe.serverPort();
    probe.close();

    serialization::TextProtocolCodec codec;
    gateways::ServerGateway gateway(codec);

    const auto result = gateway.connectToServer("127.0.0.1", port);

    ASSERT_FALSE(result.ok());
    EXPECT_EQ(result.error().code, QString("network_error"));
    EXPECT_TRUE(result.error().message.contains(QString::number(port)));
}

TEST(ServerGatewayTests, RoundTripsCurrencyRequest) {
    MockLineServer server({makeEnvelope("ok", {
        {"currency_count", "1"},
        {"currency0_code", "USD"},
        {"currency0_name", "US Dollar"},
        {"currency0_minor_units", "2"},
    })});
    ASSERT_NE(server.port(), 0);

    serialization::TextProtocolCodec codec;
    gateways::ServerGateway gateway(codec);
    ASSERT_TRUE(connectGateway(gateway, server.port()).ok());

    const auto response = gateway.sendGetCurrencies();

    ASSERT_TRUE(response.ok());
    EXPECT_EQ(response.value().status, QString("ok"));
    EXPECT_EQ(response.value().fields.value("currency_count"), QString("1"));
    ASSERT_TRUE(waitUntil([&server] { return !server.receivedLines().isEmpty(); }, 1000));
    EXPECT_TRUE(server.receivedLines().first().contains("command=get_currencies"));
}

TEST(ServerGatewayTests, SendsProviderWhenCurrencyCatalogIsScoped) {
    MockLineServer server({makeEnvelope("ok", {
        {"currency_count", "1"},
        {"currency0_code", "RUB"},
        {"currency0_name", "Russian Ruble"},
        {"currency0_minor_units", "2"},
    })});

    serialization::TextProtocolCodec codec;
    gateways::ServerGateway gateway(codec);
    ASSERT_TRUE(connectGateway(gateway, server.port()).ok());

    const auto response = gateway.sendGetCurrencies("cbr");

    ASSERT_TRUE(response.ok());
    ASSERT_TRUE(waitUntil([&server] { return !server.receivedLines().isEmpty(); }, 1000));
    EXPECT_TRUE(server.receivedLines().first().contains("provider=cbr"));
}

TEST(ServerGatewayTests, RejectsSendWithoutKnownEndpoint) {
    serialization::TextProtocolCodec codec;
    gateways::ServerGateway gateway(codec);

    const auto response = gateway.sendGetCurrencies();

    ASSERT_FALSE(response.ok());
    EXPECT_EQ(response.error().code, QString("validation_error"));
}

TEST(ServerGatewayTests, ReturnsProtocolErrorForMalformedResponse) {
    MockLineServer server({QString::fromUtf8("not_an_envelope")});

    serialization::TextProtocolCodec codec;
    gateways::ServerGateway gateway(codec);
    ASSERT_TRUE(connectGateway(gateway, server.port()).ok());

    const auto response = gateway.sendGetCurrencies();

    ASSERT_FALSE(response.ok());
    EXPECT_EQ(response.error().code, QString("protocol_error"));
}

}
