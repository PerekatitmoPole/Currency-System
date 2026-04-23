#pragma once

#include "common/Result.hpp"
#include "dto/ServerDtos.hpp"
#include "serialization/TextProtocolCodec.hpp"

#include <QObject>
#include <QTcpSocket>

namespace currency::client::gateways {

class ServerGateway : public QObject {
    Q_OBJECT

public:
    ServerGateway(serialization::TextProtocolCodec& codec, QObject* parent = nullptr);

    common::Result<void> connectToServer(const QString& host, quint16 port);
    void disconnectFromServer();
    bool isConnected() const;

    common::Result<dto::ServerEnvelopeDto> sendGetCurrencies(const QString& provider = {});
    common::Result<dto::ServerEnvelopeDto> sendGetRates(const dto::GetRatesRequestDto& request);
    common::Result<dto::ServerEnvelopeDto> sendGetHistory(const dto::GetHistoryRequestDto& request);
    common::Result<dto::ServerEnvelopeDto> sendConvert(const dto::ConvertRequestDto& request);

private:
    common::Result<dto::ServerEnvelopeDto> sendEnvelope(const QString& command, const dto::FieldMap& fields);

    serialization::TextProtocolCodec& codec_;
    QTcpSocket socket_;
    QString host_;
    quint16 port_{0};
};

}
