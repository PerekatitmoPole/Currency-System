#pragma once

#include "logging/Logger.hpp"
#include "network/RequestRouter.hpp"

#include <boost/asio.hpp>

#include <chrono>
#include <memory>
#include <string>

namespace currency::network {

class Session : public std::enable_shared_from_this<Session> {
public:
    Session(
        boost::asio::ip::tcp::socket socket,
        const RequestRouter& router,
        logging::Logger& logger);

    void start();

private:
    std::string remoteEndpoint() const;
    void armTimeout();
    void cancelTimeout();
    void stop();
    void read();
    void write(std::string response);

    boost::asio::ip::tcp::socket socket_;
    boost::asio::streambuf buffer_;
    boost::asio::steady_timer timer_;
    const RequestRouter& router_;
    logging::Logger& logger_;
    bool stopped_{false};
};

}
