#include "gateways/ServerGateway.hpp"

#include "common/Errors.hpp"

namespace currency::client::gateways {

namespace {

common::Error mapSocketConnectionError(const QTcpSocket& socket, const QString& host, const quint16 port) {
    switch (socket.error()) {
    case QAbstractSocket::HostNotFoundError:
        return common::Errors::networkError(
            QString("Адрес сервера %1:%2 недоступен или указан неверно.").arg(host).arg(port),
            socket.errorString());
    case QAbstractSocket::ConnectionRefusedError:
        return common::Errors::networkError(
            QString("Сервер %1:%2 отклонил подключение.").arg(host).arg(port),
            socket.errorString());
    case QAbstractSocket::SocketTimeoutError:
        return common::Errors::networkError(
            QString("Время ожидания подключения к серверу %1:%2 истекло.").arg(host).arg(port),
            socket.errorString());
    default:
        return common::Errors::networkError(
            QString("Не удалось подключиться к серверу %1:%2.").arg(host).arg(port),
            socket.errorString());
    }
}

}

ServerGateway::ServerGateway(serialization::TextProtocolCodec& codec, QObject* parent)
    : QObject(parent),
      codec_(codec) {}

common::Result<void> ServerGateway::connectToServer(const QString& host, const quint16 port) {
    host_ = host.trimmed();
    port_ = port;

    if (host_.isEmpty() || port_ == 0) {
        return common::Result<void>::failure(common::Errors::validationError(
            "Укажите хост и порт сервера.", {}));
    }

    if (socket_.state() == QAbstractSocket::ConnectedState) {
        socket_.disconnectFromHost();
        socket_.waitForDisconnected(1000);
    }

    socket_.connectToHost(host_, port_);
    if (!socket_.waitForConnected(3000)) {
        return common::Result<void>::failure(mapSocketConnectionError(socket_, host_, port_));
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

common::Result<dto::ServerEnvelopeDto> ServerGateway::sendGetCurrencies(const QString& provider) {
    dto::FieldMap fields;
    if (!provider.trimmed().isEmpty()) {
        fields.insert("provider", provider.trimmed().toLower());
    }
    return sendEnvelope("get_currencies", fields);
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
    if (socket_.write(payload.toUtf8()) == -1 || !socket_.waitForBytesWritten(30000)) {
        return common::Result<dto::ServerEnvelopeDto>::failure(common::Errors::networkError(
            "Не удалось отправить запрос на сервер.", socket_.errorString()));
    }

    if (!socket_.waitForReadyRead(50000)) {
        return common::Result<dto::ServerEnvelopeDto>::failure(common::Errors::networkError(
            "Сервер не ответил в отведенное время.", socket_.errorString()));
    }

    while (!socket_.canReadLine()) {
        if (!socket_.waitForReadyRead(20000)) {
            break;
        }
    }

    if (!socket_.canReadLine()) {
        return common::Result<dto::ServerEnvelopeDto>::failure(common::Errors::protocolError(
            "Ответ сервера не содержит корректный завершающий перевод строки.", {}));
    }

    return codec_.decodeEnvelope(QString::fromUtf8(socket_.readLine()).trimmed());
}

}