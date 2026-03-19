#include "network/Session.hpp"
#include "network/TcpServer.hpp"
#include "TestHelpers.hpp"

#include <gtest/gtest.h>

#include <boost/asio.hpp>

#include <thread>

namespace currency::tests {
namespace {

std::string readLine(boost::asio::ip::tcp::socket& socket) {
    boost::asio::streambuf buffer;
    boost::asio::read_until(socket, buffer, '\n');
    std::istream input(&buffer);
    std::string line;
    std::getline(input, line);
    return line;
}

}

TEST(NetworkIntegrationTests, SessionHandlesConvertRequestOverSocketPair) {
    TestAppContext context;
    seedQuote(context.quoteRepository, "ECB", "EUR", "USD", 1.20, "2026-03-19T12:00:00Z");

    boost::asio::io_context ioContext;
    boost::asio::ip::tcp::acceptor acceptor(ioContext, {boost::asio::ip::tcp::v4(), 0});
    const auto port = acceptor.local_endpoint().port();

    boost::asio::ip::tcp::socket clientSocket(ioContext);
    clientSocket.connect({boost::asio::ip::address_v4::loopback(), port});
    auto serverSocket = acceptor.accept();

    auto session = std::make_shared<network::Session>(std::move(serverSocket), context.router);
    session->start();

    std::jthread worker([&ioContext] { ioContext.run(); });

    boost::asio::write(clientSocket, boost::asio::buffer(std::string(
        "command=convert;provider=ECB;from_currency=EUR;to_currency=USD;amount=10\n")));

    const auto response = readLine(clientSocket);
    const auto fields = parseEnvelopePayload(context.serializer, response);
    EXPECT_EQ(requireField(fields, "status"), "ok");
    EXPECT_EQ(requireField(fields, "result"), "12.000000");

    ioContext.stop();
}

TEST(NetworkIntegrationTests, SessionReturnsProtocolErrorForMalformedPayload) {
    TestAppContext context;

    boost::asio::io_context ioContext;
    boost::asio::ip::tcp::acceptor acceptor(ioContext, {boost::asio::ip::tcp::v4(), 0});
    const auto port = acceptor.local_endpoint().port();

    boost::asio::ip::tcp::socket clientSocket(ioContext);
    clientSocket.connect({boost::asio::ip::address_v4::loopback(), port});
    auto serverSocket = acceptor.accept();

    auto session = std::make_shared<network::Session>(std::move(serverSocket), context.router);
    session->start();

    std::jthread worker([&ioContext] { ioContext.run(); });

    boost::asio::write(clientSocket, boost::asio::buffer(std::string("broken_payload\n")));

    const auto response = readLine(clientSocket);
    const auto fields = parseEnvelopePayload(context.serializer, response);
    EXPECT_EQ(requireField(fields, "status"), "error");
    EXPECT_EQ(requireField(fields, "code"), "protocol_error");

    ioContext.stop();
}

TEST(NetworkIntegrationTests, TcpServerAcceptsConnectionAndHandlesGetCurrencies) {
    TestAppContext context;

    boost::asio::io_context ioContext;
    network::TcpServer server(ioContext, 23145, context.router);
    server.start();
    std::jthread worker([&ioContext] { ioContext.run(); });

    boost::asio::ip::tcp::socket clientSocket(ioContext);
    clientSocket.connect({boost::asio::ip::address_v4::loopback(), 23145});
    boost::asio::write(clientSocket, boost::asio::buffer(std::string("command=get_currencies\n")));

    const auto response = readLine(clientSocket);
    const auto fields = parseEnvelopePayload(context.serializer, response);
    EXPECT_EQ(requireField(fields, "status"), "ok");
    EXPECT_EQ(requireField(fields, "currency_count"), "7");

    ioContext.stop();
}

TEST(NetworkIntegrationTests, TcpServerHandlesIngestThenConvertOnSingleConnection) {
    TestAppContext context;

    boost::asio::io_context ioContext;
    network::TcpServer server(ioContext, 23146, context.router);
    server.start();
    std::jthread worker([&ioContext] { ioContext.run(); });

    boost::asio::ip::tcp::socket clientSocket(ioContext);
    clientSocket.connect({boost::asio::ip::address_v4::loopback(), 23146});

    boost::asio::write(clientSocket, boost::asio::buffer(std::string(
        "command=ingest_quotes;provider=ECB;batch_timestamp=2026-03-19T10%3A00%3A00Z;quote_count=1;"
        "quote0_base_code=EUR;quote0_base_name=Euro;quote0_quote_code=USD;quote0_quote_name=US%20Dollar;"
        "quote0_rate=1.25;quote0_source_timestamp=2026-03-19T10%3A00%3A00Z\n")));
    const auto ingestResponse = readLine(clientSocket);
    const auto ingestFields = parseEnvelopePayload(context.serializer, ingestResponse);
    EXPECT_EQ(requireField(ingestFields, "status"), "ok");

    boost::asio::write(clientSocket, boost::asio::buffer(std::string(
        "command=convert;provider=ECB;from_currency=EUR;to_currency=USD;amount=2\n")));
    const auto convertResponse = readLine(clientSocket);
    const auto convertFields = parseEnvelopePayload(context.serializer, convertResponse);
    EXPECT_EQ(requireField(convertFields, "status"), "ok");
    EXPECT_EQ(requireField(convertFields, "result"), "2.500000");

    ioContext.stop();
}

}
