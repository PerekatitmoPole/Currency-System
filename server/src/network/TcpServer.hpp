#pragma once

#include "network/RequestRouter.hpp"

#include <boost/asio.hpp>

namespace currency::network {

class TcpServer {
public:
    TcpServer(boost::asio::io_context& ioContext, unsigned short port, const RequestRouter& router);

    void start();
    void stop();

private:
    void accept();

    boost::asio::io_context& ioContext_;
    boost::asio::ip::tcp::acceptor acceptor_;
    const RequestRouter& router_;
};

}
