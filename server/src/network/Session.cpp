#include "network/Session.hpp"

#include <utility>

namespace currency::network {

Session::Session(
    boost::asio::ip::tcp::socket socket,
    const RequestRouter& router,
    logging::Logger& logger)
    : socket_(std::move(socket)),
      timer_(socket_.get_executor()),
      router_(router),
      logger_(logger) {}

void Session::start() {
    logger_.info("network.session", "Client connected", {{"remote", remoteEndpoint()}});
    read();
}

std::string Session::remoteEndpoint() const {
    boost::system::error_code errorCode;
    const auto endpoint = socket_.remote_endpoint(errorCode);
    if (errorCode) {
        return "unknown";
    }

    return endpoint.address().to_string() + ":" + std::to_string(endpoint.port());
}

void Session::armTimeout() {
    timer_.expires_after(std::chrono::seconds(120));
    auto self = shared_from_this();
    timer_.async_wait([this, self](const boost::system::error_code& errorCode) {
        if (errorCode) {
            return;
        }

        logger_.warning("network.session", "Session timed out", {{"remote", remoteEndpoint()}});
        stop();
    });
}

void Session::cancelTimeout() {
    timer_.cancel();
}

void Session::stop() {
    if (stopped_) {
        return;
    }
    stopped_ = true;

    cancelTimeout();
    const auto remote = remoteEndpoint();

    boost::system::error_code ignored;
    socket_.shutdown(boost::asio::ip::tcp::socket::shutdown_both, ignored);
    socket_.close(ignored);
    logger_.info("network.session", "Client disconnected", {{"remote", remote}});
}

void Session::read() {
    auto self = shared_from_this();
    armTimeout();
    boost::asio::async_read_until(
        socket_,
        buffer_,
        '\n',
        [this, self](const boost::system::error_code& errorCode, const std::size_t bytesTransferred) {
            if (errorCode) {
                cancelTimeout();
                if (errorCode != boost::asio::error::operation_aborted &&
                    errorCode != boost::asio::error::eof &&
                    errorCode != boost::asio::error::connection_reset) {
                    logger_.warning(
                        "network.session",
                        "Read failed",
                        {
                            {"remote", remoteEndpoint()},
                            {"code", std::to_string(errorCode.value())},
                            {"message", "socket read failure"},
                        });
                }
                if (errorCode != boost::asio::error::operation_aborted) {
                    stop();
                }
                return;
            }

            cancelTimeout();
            std::istream input(&buffer_);
            std::string line;
            std::getline(input, line);
            logger_.info(
                "network.session",
                "Request received",
                {
                    {"remote", remoteEndpoint()},
                    {"bytes", std::to_string(bytesTransferred)},
                });
            auto response = router_.handle(line);
            write(std::move(response));
        });
}

void Session::write(std::string response) {
    auto self = shared_from_this();
    auto outbound = std::make_shared<std::string>(std::move(response));
    outbound->push_back('\n');
    armTimeout();
    boost::asio::async_write(
        socket_,
        boost::asio::buffer(*outbound),
        [this, self, outbound](const boost::system::error_code& errorCode, const std::size_t) {
            if (errorCode) {
                cancelTimeout();
                if (errorCode != boost::asio::error::operation_aborted) {
                    logger_.warning(
                        "network.session",
                        "Write failed",
                        {
                            {"remote", remoteEndpoint()},
                            {"code", std::to_string(errorCode.value())},
                            {"message", "socket write failure"},
                        });
                    stop();
                }
                return;
            }
            cancelTimeout();
            read();
        });
}

}
