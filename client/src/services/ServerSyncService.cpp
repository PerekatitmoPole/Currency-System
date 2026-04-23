#include "services/ServerSyncService.hpp"

namespace currency::client::services {

ServerSyncService::ServerSyncService(gateways::ServerGateway& gateway)
    : gateway_(gateway) {}

common::Result<void> ServerSyncService::connect(const QString& host, const quint16 port) {
    return gateway_.connectToServer(host, port);
}

void ServerSyncService::disconnect() {
    gateway_.disconnectFromServer();
}

bool ServerSyncService::isConnected() const {
    return gateway_.isConnected();
}

}