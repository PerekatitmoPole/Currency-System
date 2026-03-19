#include "network/Session.hpp"

#include <utility>

namespace currency::network {

Session::Session(boost::asio::ip::tcp::socket socket, const RequestRouter& router)
    : socket_(std::move(socket)), router_(router) {}

void Session::start() {
    read();
}

void Session::read() {
    auto self = shared_from_this();
    boost::asio::async_read_until(
        socket_,
        buffer_,
        '\n',
        [this, self](const boost::system::error_code& errorCode, const std::size_t /*bytesTransferred*/) {
            if (errorCode) {
                return;
            }

            std::istream input(&buffer_);
            std::string line;
            std::getline(input, line);
            auto response = router_.handle(line);
            write(std::move(response));
        });
}

void Session::write(std::string response) {
    auto self = shared_from_this();
    auto outbound = std::make_shared<std::string>(std::move(response));
    outbound->push_back('\n');
    boost::asio::async_write(
        socket_,
        boost::asio::buffer(*outbound),
        [this, self, outbound](const boost::system::error_code& errorCode, const std::size_t) {
            if (errorCode) {
                return;
            }
            read();
        });
}

}
