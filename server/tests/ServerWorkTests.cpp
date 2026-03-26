#include "network/TcpServer.hpp"
#include "TestHelpers.hpp"

#include <gtest/gtest.h>

#include <boost/asio.hpp>

#include <chrono>
#include <istream>
#include <string>
#include <thread>

namespace currency::tests {
namespace {

using boost::asio::ip::tcp;

std::string readLine(tcp::socket& socket) {
    boost::asio::streambuf buffer;
    boost::asio::read_until(socket, buffer, '\n');

    std::istream input(&buffer);
    std::string line;
    std::getline(input, line);
    return line;
}

void writeLine(tcp::socket& socket, const std::string& line) {
    const std::string payload = line + "\n";
    boost::asio::write(socket, boost::asio::buffer(payload));
}

tcp::socket connectClient(boost::asio::io_context& ioContext, unsigned short port) {
    tcp::socket socket(ioContext);
    socket.connect({boost::asio::ip::address_v4::loopback(), port});
    return socket;
}

class RunningServer {
public:
    RunningServer(const unsigned short port, const TestAppContext& context)
        : ioContext_(),
          server_(ioContext_, port, context.router),
          worker_([this] {
              ioContext_.run();
          }) {
        server_.start();
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
    }

    ~RunningServer() {
        ioContext_.stop();
    }

private:
    boost::asio::io_context ioContext_;
    network::TcpServer server_;
    std::jthread worker_;
};

}

TEST(ServerWorkTests, GetCurrenciesReturnsBuiltInCatalog) {
    TestAppContext context;
    RunningServer server(24101, context);

    boost::asio::io_context clientIo;
    auto socket = connectClient(clientIo, 24101);

    writeLine(socket, "command=get_currencies");

    const auto response = readLine(socket);
    const auto fields = parseEnvelopePayload(context.serializer, response);

    EXPECT_EQ(requireField(fields, "status"), "ok");
    EXPECT_EQ(requireField(fields, "currency_count"), "7");
    EXPECT_EQ(requireField(fields, "currency0_code"), "CHF");
    EXPECT_EQ(requireField(fields, "currency1_code"), "CNY");
    EXPECT_EQ(requireField(fields, "currency2_code"), "EUR");
    EXPECT_EQ(requireField(fields, "currency3_code"), "GBP");
    EXPECT_EQ(requireField(fields, "currency4_code"), "JPY");
    EXPECT_EQ(requireField(fields, "currency5_code"), "RUB");
    EXPECT_EQ(requireField(fields, "currency6_code"), "USD");
}

TEST(NetworkIntegrationTests, IngestQuotesThenGetRatesReturnsStoredRates) {
    TestAppContext context;
    RunningServer server(24102, context);

    boost::asio::io_context clientIo;
    auto socket = connectClient(clientIo, 24102);

    writeLine(
        socket,
        "command=ingest_quotes;"
        "provider=ECB;"
        "batch_timestamp=2026-03-19T10%3A00%3A00Z;"
        "quote_count=2;"
        "quote0_base_code=EUR;"
        "quote0_base_name=Euro;"
        "quote0_quote_code=USD;"
        "quote0_quote_name=US%20Dollar;"
        "quote0_rate=1.250000;"
        "quote0_source_timestamp=2026-03-19T10%3A00%3A00Z;"
        "quote1_base_code=EUR;"
        "quote1_base_name=Euro;"
        "quote1_quote_code=RUB;"
        "quote1_quote_name=Russian%20Ruble;"
        "quote1_rate=100.500000;"
        "quote1_source_timestamp=2026-03-19T10%3A00%3A00Z"
    );

    const auto ingestResponse = readLine(socket);
    const auto ingestFields = parseEnvelopePayload(context.serializer, ingestResponse);

    EXPECT_EQ(requireField(ingestFields, "status"), "ok");
    EXPECT_EQ(requireField(ingestFields, "accepted_count"), "2");

    writeLine(
        socket,
        "command=get_rates;"
        "provider=ECB;"
        "base_code=EUR;"
        "quote_codes=USD,RUB"
    );

    const auto ratesResponse = readLine(socket);
    const auto rateFields = parseEnvelopePayload(context.serializer, ratesResponse);

    ASSERT_EQ(requireField(rateFields, "status"), "ok");
    EXPECT_EQ(requireField(rateFields, "rate_count"), "2");

    EXPECT_EQ(requireField(rateFields, "rate1_base_code"), "EUR");
    EXPECT_EQ(requireField(rateFields, "rate1_quote_code"), "RUB");
    EXPECT_EQ(requireField(rateFields, "rate1_provider"), "ECB");
    EXPECT_EQ(requireField(rateFields, "rate1_rate"), "100.500000");

    EXPECT_EQ(requireField(rateFields, "rate0_base_code"), "EUR");
    EXPECT_EQ(requireField(rateFields, "rate0_quote_code"), "USD");
    EXPECT_EQ(requireField(rateFields, "rate0_provider"), "ECB");
    EXPECT_EQ(requireField(rateFields, "rate0_rate"), "1.250000");
}

TEST(NetworkIntegrationTests, IngestQuotesThenConvertReturnsExpectedAmount) {
    TestAppContext context;
    RunningServer server(24103, context);

    boost::asio::io_context clientIo;
    auto socket = connectClient(clientIo, 24103);

    writeLine(
        socket,
        "command=ingest_quotes;"
        "provider=ECB;"
        "batch_timestamp=2026-03-19T10%3A00%3A00Z;"
        "quote_count=1;"
        "quote0_base_code=EUR;"
        "quote0_base_name=Euro;"
        "quote0_quote_code=USD;"
        "quote0_quote_name=US%20Dollar;"
        "quote0_rate=1.250000;"
        "quote0_source_timestamp=2026-03-19T10%3A00%3A00Z"
    );

    const auto ingestResponse = readLine(socket);
    const auto ingestFields = parseEnvelopePayload(context.serializer, ingestResponse);
    EXPECT_EQ(requireField(ingestFields, "status"), "ok");

    writeLine(
        socket,
        "command=convert;"
        "provider=ECB;"
        "from_currency=EUR;"
        "to_currency=USD;"
        "amount=2.000000"
    );

    const auto convertResponse = readLine(socket);
    const auto convertFields = parseEnvelopePayload(context.serializer, convertResponse);

    EXPECT_EQ(requireField(convertFields, "status"), "ok");
    EXPECT_EQ(requireField(convertFields, "from_currency"), "EUR");
    EXPECT_EQ(requireField(convertFields, "to_currency"), "USD");
    EXPECT_EQ(requireField(convertFields, "amount"), "2.000000");
    EXPECT_EQ(requireField(convertFields, "rate"), "1.250000");
    EXPECT_EQ(requireField(convertFields, "result"), "2.500000");
}

TEST(NetworkIntegrationTests, IngestQuotesThenGetHistoryReturnsStoredPoints) {
    TestAppContext context;
    RunningServer server(24104, context);

    boost::asio::io_context clientIo;
    auto socket = connectClient(clientIo, 24104);

    writeLine(
        socket,
        "command=ingest_quotes;"
        "provider=ECB;"
        "batch_timestamp=2026-03-19T10%3A00%3A00Z;"
        "quote_count=1;"
        "quote0_base_code=EUR;"
        "quote0_base_name=Euro;"
        "quote0_quote_code=USD;"
        "quote0_quote_name=US%20Dollar;"
        "quote0_rate=1.200000;"
        "quote0_source_timestamp=2026-03-19T10%3A00%3A00Z"
    );
    {
        const auto response = readLine(socket);
        const auto fields = parseEnvelopePayload(context.serializer, response);
        EXPECT_EQ(requireField(fields, "status"), "ok");
    }

    writeLine(
        socket,
        "command=ingest_quotes;"
        "provider=ECB;"
        "batch_timestamp=2026-03-19T10%3A05%3A00Z;"
        "quote_count=1;"
        "quote0_base_code=EUR;"
        "quote0_base_name=Euro;"
        "quote0_quote_code=USD;"
        "quote0_quote_name=US%20Dollar;"
        "quote0_rate=1.250000;"
        "quote0_source_timestamp=2026-03-19T10%3A05%3A00Z"
    );
    {
        const auto response = readLine(socket);
        const auto fields = parseEnvelopePayload(context.serializer, response);
        EXPECT_EQ(requireField(fields, "status"), "ok");
    }

    writeLine(
        socket,
        "command=get_history;"
        "provider=ECB;"
        "base_code=EUR;"
        "quote_code=USD;"
        "from=2026-03-19T09%3A59%3A00Z;"
        "to=2026-03-19T10%3A06%3A00Z;"
        "step=1m"
    );

    const auto historyResponse = readLine(socket);
    const auto historyFields = parseEnvelopePayload(context.serializer, historyResponse);

    ASSERT_EQ(requireField(historyFields, "status"), "ok");
    EXPECT_EQ(requireField(historyFields, "point_count"), "2");

    EXPECT_EQ(requireField(historyFields, "point0_rate"), "1.200000");
    EXPECT_EQ(requireField(historyFields, "point1_rate"), "1.250000");
}

TEST(NetworkIntegrationTests, ConvertSameCurrencyReturnsIdentityConversion) {
    TestAppContext context;
    RunningServer server(24105, context);

    boost::asio::io_context clientIo;
    auto socket = connectClient(clientIo, 24105);

    writeLine(
        socket,
        "command=convert;"
        "provider=ECB;"
        "from_currency=USD;"
        "to_currency=USD;"
        "amount=15.000000"
    );

    const auto response = readLine(socket);
    const auto fields = parseEnvelopePayload(context.serializer, response);

    EXPECT_EQ(requireField(fields, "status"), "ok");
    EXPECT_EQ(requireField(fields, "rate"), "1.000000");
    EXPECT_EQ(requireField(fields, "result"), "15.000000");
}

TEST(NetworkIntegrationTests, GetRatesReturnsErrorWhenQuoteIsMissing) {
    TestAppContext context;
    RunningServer server(24106, context);

    boost::asio::io_context clientIo;
    auto socket = connectClient(clientIo, 24106);

    writeLine(
        socket,
        "command=get_rates;"
        "provider=ECB;"
        "base_currency=EUR;"
        "quote_codes=USD"
    );

    const auto response = readLine(socket);
    const auto fields = parseEnvelopePayload(context.serializer, response);

    EXPECT_EQ(requireField(fields, "status"), "error");
}

TEST(NetworkIntegrationTests, ConvertReturnsErrorWhenRateDoesNotExist) {
    TestAppContext context;
    RunningServer server(24107, context);

    boost::asio::io_context clientIo;
    auto socket = connectClient(clientIo, 24107);

    writeLine(
        socket,
        "command=convert;"
        "provider=ECB;"
        "from_currency=EUR;"
        "to_currency=USD;"
        "amount=3.000000"
    );

    const auto response = readLine(socket);
    const auto fields = parseEnvelopePayload(context.serializer, response);

    EXPECT_EQ(requireField(fields, "status"), "error");
}

TEST(NetworkIntegrationTests, IngestQuotesRejectsNegativeRate) {
    TestAppContext context;
    RunningServer server(24108, context);

    boost::asio::io_context clientIo;
    auto socket = connectClient(clientIo, 24108);

    writeLine(
        socket,
        "command=ingest_quotes;"
        "provider=ECB;"
        "batch_timestamp=2026-03-19T10%3A00%3A00Z;"
        "quote_count=1;"
        "quote0_base_code=EUR;"
        "quote0_base_name=Euro;"
        "quote0_quote_code=USD;"
        "quote0_quote_name=US%20Dollar;"
        "quote0_rate=-1.250000;"
        "quote0_source_timestamp=2026-03-19T10%3A00%3A00Z"
    );

    const auto response = readLine(socket);
    const auto fields = parseEnvelopePayload(context.serializer, response);

    EXPECT_EQ(requireField(fields, "status"), "error");
}

TEST(NetworkIntegrationTests, GetHistoryRejectsInvalidRange) {
    TestAppContext context;
    RunningServer server(24109, context);

    boost::asio::io_context clientIo;
    auto socket = connectClient(clientIo, 24109);

    writeLine(
        socket,
        "command=get_history;"
        "provider=ECB;"
        "base_currency=EUR;"
        "quote_currency=USD;"
        "from=2026-03-19T10%3A10%3A00Z;"
        "to=2026-03-19T10%3A00%3A00Z;"
        "step=1m"
    );

    const auto response = readLine(socket);
    const auto fields = parseEnvelopePayload(context.serializer, response);

    EXPECT_EQ(requireField(fields, "status"), "error");
}

TEST(NetworkIntegrationTests, UnknownCommandReturnsProtocolError) {
    TestAppContext context;
    RunningServer server(24110, context);

    boost::asio::io_context clientIo;
    auto socket = connectClient(clientIo, 24110);

    writeLine(socket, "command=unknown_command");

    const auto response = readLine(socket);
    const auto fields = parseEnvelopePayload(context.serializer, response);

    EXPECT_EQ(requireField(fields, "status"), "error");
    EXPECT_EQ(requireField(fields, "code"), "unknown_command");
}

TEST(NetworkIntegrationTests, MalformedPayloadReturnsProtocolError) {
    TestAppContext context;
    RunningServer server(24111, context);

    boost::asio::io_context clientIo;
    auto socket = connectClient(clientIo, 24111);

    writeLine(socket, "this_is_not_a_valid_payload");

    const auto response = readLine(socket);
    const auto fields = parseEnvelopePayload(context.serializer, response);

    EXPECT_EQ(requireField(fields, "status"), "error");
    EXPECT_EQ(requireField(fields, "code"), "protocol_error");
}

TEST(NetworkIntegrationTests, SingleConnectionCanHandleFullScenario) {
    TestAppContext context;
    RunningServer server(24112, context);

    boost::asio::io_context clientIo;
    auto socket = connectClient(clientIo, 24112);

    writeLine(socket, "command=get_currencies");
    {
        const auto response = readLine(socket);
        const auto fields = parseEnvelopePayload(context.serializer, response);
        EXPECT_EQ(requireField(fields, "status"), "ok");
        EXPECT_EQ(requireField(fields, "currency_count"), "7");
    }

    writeLine(
        socket,
        "command=ingest_quotes;"
        "provider=ECB;"
        "batch_timestamp=2026-03-19T10%3A00%3A00Z;"
        "quote_count=2;"
        "quote0_base_code=EUR;"
        "quote0_base_name=Euro;"
        "quote0_quote_code=USD;"
        "quote0_quote_name=US%20Dollar;"
        "quote0_rate=1.300000;"
        "quote0_source_timestamp=2026-03-19T10%3A00%3A00Z;"
        "quote1_base_code=EUR;"
        "quote1_base_name=Euro;"
        "quote1_quote_code=RUB;"
        "quote1_quote_name=Russian%20Ruble;"
        "quote1_rate=101.000000;"
        "quote1_source_timestamp=2026-03-19T10%3A00%3A00Z"
    );
    {
        const auto response = readLine(socket);
        const auto fields = parseEnvelopePayload(context.serializer, response);
        EXPECT_EQ(requireField(fields, "status"), "ok");
        EXPECT_EQ(requireField(fields, "accepted_count"), "2");
    }

    writeLine(
        socket,
        "command=get_rates;"
        "provider=ECB;"
        "base_code=EUR;"
        "quote_codes=USD,RUB"
    );
    {
        const auto response = readLine(socket);
        const auto fields = parseEnvelopePayload(context.serializer, response);
        EXPECT_EQ(requireField(fields, "status"), "ok");
        EXPECT_EQ(requireField(fields, "rate_count"), "2");
    }

    writeLine(
        socket,
        "command=convert;"
        "provider=ECB;"
        "from_currency=EUR;"
        "to_currency=USD;"
        "amount=5.000000"
    );
    {
        const auto response = readLine(socket);
        const auto fields = parseEnvelopePayload(context.serializer, response);
        EXPECT_EQ(requireField(fields, "status"), "ok");
        EXPECT_EQ(requireField(fields, "result"), "6.500000");
    }

    writeLine(
        socket,
        "command=get_history;"
        "provider=ECB;"
        "base_code=EUR;"
        "quote_code=USD;"
        "from=2026-03-19T09%3A59%3A00Z;"
        "to=2026-03-19T10%3A01%3A00Z;"
        "step=1m"
    );
    {
        const auto response = readLine(socket);
        const auto fields = parseEnvelopePayload(context.serializer, response);
        EXPECT_EQ(requireField(fields, "status"), "ok");
        EXPECT_EQ(requireField(fields, "point_count"), "1");
        EXPECT_EQ(requireField(fields, "point0_rate"), "1.300000");
    }
}

} 