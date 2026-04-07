#pragma once

#include "gateways/IApiSourceClient.hpp"

#include <QNetworkAccessManager>
#include <QObject>
#include <QUrl>

namespace currency::client::gateways {

class AbstractApiSourceClient : public QObject, public IApiSourceClient {
    Q_OBJECT

public:
    AbstractApiSourceClient(dto::ApiSource source, QNetworkAccessManager& networkManager, QObject* parent = nullptr);

    dto::ApiSource source() const override;
    QString displayName() const override;
    void setApiKey(const QString& apiKey) override;
    bool supportsHistory() const override;

protected:
    common::Result<QByteArray> performGet(
        const QUrl& url,
        const QList<QPair<QByteArray, QByteArray>>& headers = {}) const;

    common::Result<QByteArray> requireApiKeyAndGet(
        const QUrl& url,
        const QList<QPair<QByteArray, QByteArray>>& headers = {}) const;

    common::Error missingApiKeyError() const;
    QString apiKey() const;

private:
    dto::ApiSource source_;
    QNetworkAccessManager& networkManager_;
    QString apiKey_;
};

}