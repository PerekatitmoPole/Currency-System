#pragma once

#include "common/Result.hpp"
#include "gateways/ServerGateway.hpp"

namespace currency::client::services {

class ServerSyncService {
public:
    ServerSyncService(gateways::ServerGateway& gateway);

    common::Result<void> connect(const QString& host, quint16 port);
    void disconnect();
    bool isConnected() const;

private:
    gateways::ServerGateway& gateway_;
};

}