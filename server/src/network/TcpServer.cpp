#include "network/TcpServer.hpp"

#include "network/Session.hpp"

namespace currency::network {

TcpServer::TcpServer(
    boost::asio::io_context& ioContext,
    const unsigned short port,
    const RequestRouter& router)
    : ioContext_(ioContext),
      acceptor_(ioContext, boost::asio::ip::tcp::endpoint(boost::asio::ip::tcp::v4(), port)),
      router_(router) {}

void TcpServer::start() {
    accept();
}

void TcpServer::accept() {
    acceptor_.async_accept([this](const boost::system::error_code& errorCode, boost::asio::ip::tcp::socket socket) {
        if (!errorCode) {
            std::make_shared<Session>(std::move(socket), router_)->start();
        }
        accept();
    });
}

void TcpServer::stop() {
    boost::system::error_code errorCode;
    acceptor_.cancel(errorCode);
    errorCode.clear();
    acceptor_.close(errorCode);
}

}
