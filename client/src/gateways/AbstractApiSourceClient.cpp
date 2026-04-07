#include "gateways/AbstractApiSourceClient.hpp"

#include "common/Errors.hpp"
#include "dto/ApiDtos.hpp"

#include <QEventLoop>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QTimer>

namespace currency::client::gateways {

AbstractApiSourceClient::AbstractApiSourceClient(
    const dto::ApiSource source,
    QNetworkAccessManager& networkManager,
    QObject* parent)
    : QObject(parent),
      source_(source),
      networkManager_(networkManager) {}

dto::ApiSource AbstractApiSourceClient::source() const {
    return source_;
}

QString AbstractApiSourceClient::displayName() const {
    return dto::toDisplayName(source_);
}

void AbstractApiSourceClient::setApiKey(const QString& apiKey) {
    apiKey_ = apiKey.trimmed();
}

bool AbstractApiSourceClient::supportsHistory() const {
    return true;
}

common::Result<QByteArray> AbstractApiSourceClient::performGet(
    const QUrl& url,
    const QList<QPair<QByteArray, QByteArray>>& headers) const {
    QNetworkRequest request(url);
    request.setTransferTimeout(8000);
    for (const auto& header : headers) {
        request.setRawHeader(header.first, header.second);
    }

    QEventLoop loop;
    QTimer timer;
    timer.setSingleShot(true);

    auto* reply = networkManager_.get(request);
    QObject::connect(&timer, &QTimer::timeout, reply, &QNetworkReply::abort);
    QObject::connect(reply, &QNetworkReply::finished, &loop, &QEventLoop::quit);
    timer.start(8000);
    loop.exec();

    const auto statusCode = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
    const auto body = reply->readAll();
    const auto error = reply->error();
    const auto errorString = reply->errorString();
    reply->deleteLater();

    if (error != QNetworkReply::NoError) {
        return common::Result<QByteArray>::failure(
            common::Errors::networkError(QString("Request to %1 failed").arg(url.toString()), errorString));
    }

    if (statusCode >= 400) {
        return common::Result<QByteArray>::failure(common::Errors::networkError(
            QString("Request to %1 returned HTTP %2").arg(url.toString()).arg(statusCode),
            QString::fromUtf8(body.left(512))));
    }

    return common::Result<QByteArray>::success(body);
}

common::Result<QByteArray> AbstractApiSourceClient::requireApiKeyAndGet(
    const QUrl& url,
    const QList<QPair<QByteArray, QByteArray>>& headers) const {
    if (apiKey_.isEmpty()) {
        return common::Result<QByteArray>::failure(missingApiKeyError());
    }

    return performGet(url, headers);
}

common::Error AbstractApiSourceClient::missingApiKeyError() const {
    return common::Errors::validationError(
        QString("API key is required for %1").arg(displayName()),
        QString("Configure an API key for source '%1' in Settings").arg(dto::toStableKey(source_)));
}

QString AbstractApiSourceClient::apiKey() const {
    return apiKey_;
}

}