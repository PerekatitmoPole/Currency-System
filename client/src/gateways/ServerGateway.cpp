#include "gateways/ServerGateway.hpp"

#include "common/Errors.hpp"

namespace currency::client::gateways {

ServerGateway::ServerGateway(serialization::TextProtocolCodec& codec, QObject* parent)
    : QObject(parent),
      codec_(codec) {}

common::Result<void> ServerGateway::connectToServer(const QString& host, const quint16 port) {
    host_ = host.trimmed();
    port_ = port;

    if (socket_.state() == QAbstractSocket::ConnectedState) {
        socket_.disconnectFromHost();
        socket_.waitForDisconnected(1000);
    }

    socket_.connectToHost(host_, port_);
    if (!socket_.waitForConnected(3000)) {
        return common::Result<void>::failure(common::Errors::networkError(
            QString("Failed to connect to server %1:%2").arg(host_).arg(port_), socket_.errorString()));
    }

    return common::Result<void>::success();
}

void ServerGateway::disconnectFromServer() {
    if (socket_.state() == QAbstractSocket::ConnectedState) {
        socket_.disconnectFromHost();
        socket_.waitForDisconnected(1000);
    }
}

bool ServerGateway::isConnected() const {
    return socket_.state() == QAbstractSocket::ConnectedState;
}

common::Result<dto::ServerEnvelopeDto> ServerGateway::sendUpdateQuotes(const dto::UpdateQuotesRequestDto& request) {
    return sendEnvelope("ingest_quotes", codec_.toFields(request));
}

common::Result<dto::ServerEnvelopeDto> ServerGateway::sendGetRates(const dto::GetRatesRequestDto& request) {
    return sendEnvelope("get_rates", codec_.toFields(request));
}

common::Result<dto::ServerEnvelopeDto> ServerGateway::sendGetHistory(const dto::GetHistoryRequestDto& request) {
    return sendEnvelope("get_history", codec_.toFields(request));
}

common::Result<dto::ServerEnvelopeDto> ServerGateway::sendConvert(const dto::ConvertRequestDto& request) {
    return sendEnvelope("convert", codec_.toFields(request));
}

common::Result<dto::ServerEnvelopeDto> ServerGateway::sendEnvelope(const QString& command, const dto::FieldMap& fields) {
    if (!isConnected()) {
        const auto reconnect = connectToServer(host_, port_);
        if (!reconnect.ok()) {
            return common::Result<dto::ServerEnvelopeDto>::failure(reconnect.error());
        }
    }

    const auto payload = codec_.encodeRequest(command, fields) + '\n';
    if (socket_.write(payload.toUtf8()) == -1 || !socket_.waitForBytesWritten(3000)) {
        return common::Result<dto::ServerEnvelopeDto>::failure(common::Errors::networkError(
            "Failed to send request to server", socket_.errorString()));
    }

    if (!socket_.waitForReadyRead(5000)) {
        return common::Result<dto::ServerEnvelopeDto>::failure(common::Errors::networkError(
            "Timed out while waiting for server response", socket_.errorString()));
    }

    while (!socket_.canReadLine()) {
        if (!socket_.waitForReadyRead(2000)) {
            break;
        }
    }

    if (!socket_.canReadLine()) {
        return common::Result<dto::ServerEnvelopeDto>::failure(common::Errors::protocolError(
            "Server response does not contain a line terminator", {}));
    }

    return codec_.decodeEnvelope(QString::fromUtf8(socket_.readLine()).trimmed());
}

}