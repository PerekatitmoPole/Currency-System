#pragma once

#include "network/RequestRouter.hpp"

#include <boost/asio.hpp>

#include <array>
#include <memory>
#include <string>

namespace currency::network {

class Session : public std::enable_shared_from_this<Session> {
public:
    Session(boost::asio::ip::tcp::socket socket, const RequestRouter& router);

    void start();

private:
    void read();
    void write(std::string response);

    boost::asio::ip::tcp::socket socket_;
    boost::asio::streambuf buffer_;
    const RequestRouter& router_;
};

}
