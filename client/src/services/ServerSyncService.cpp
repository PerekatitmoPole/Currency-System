#include "services/ServerSyncService.hpp"

#include "common/Errors.hpp"

#include <QDateTime>
#include <QHash>

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

common::Result<void> ServerSyncService::syncQuotes(const QList<dto::NormalizedQuoteDto>& quotes) {
    if (quotes.isEmpty()) {
        return common::Result<void>::failure(common::Errors::validationError("There are no normalized quotes to sync", {}));
    }

    QHash<QString, QList<dto::NormalizedQuoteDto>> groupedByProvider;
    for (const auto& quote : quotes) {
        groupedByProvider[quote.providerName].push_back(quote);
    }

    for (auto iterator = groupedByProvider.cbegin(); iterator != groupedByProvider.cend(); ++iterator) {
        dto::UpdateQuotesRequestDto request;
        request.provider = iterator.key();
        request.batchTimestamp = QDateTime::currentDateTimeUtc();
        request.quotes = iterator.value();

        const auto response = gateway_.sendUpdateQuotes(request);
        if (!response.ok()) {
            return common::Result<void>::failure(response.error());
        }

        if (response.value().status != "ok") {
            return common::Result<void>::failure(common::Errors::protocolError(
                QString("Server rejected provider sync for %1").arg(iterator.key()),
                response.value().errorMessage));
        }
    }

    return common::Result<void>::success();
}

}