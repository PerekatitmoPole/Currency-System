#pragma once

#include "dto/ApiDtos.hpp"
#include "dto/UiDtos.hpp"
#include "gateways/ServerGateway.hpp"
#include "models/viewmodels/HistoryPointViewModel.hpp"
#include "models/viewmodels/RateViewModel.hpp"

#include <QDateTime>
#include <QCoreApplication>
#include <QHostAddress>
#include <QTcpServer>
#include <QTcpSocket>
#include <QUrl>

#include <atomic>
#include <deque>
#include <future>
#include <functional>
#include <memory>
#include <mutex>
#include <thread>
#include <utility>
#include <chrono>

namespace currency::client::tests {

inline QString encodeValue(const QString& value) {
    return QString::fromLatin1(QUrl::toPercentEncoding(value, QByteArray("-_.~,")));
}

inline QString makeEnvelope(
    const QString& status,
    dto::FieldMap fields = {},
    const QString& errorCode = {},
    const QString& errorMessage = {}) {
    dto::FieldMap payload = std::move(fields);
    payload.insert("status", status);
    if (!errorCode.isEmpty()) {
        payload.insert("code", errorCode);
    }
    if (!errorMessage.isEmpty()) {
        payload.insert("message", errorMessage);
    }

    QStringList tokens;
    for (auto iterator = payload.cbegin(); iterator != payload.cend(); ++iterator) {
        tokens.push_back(iterator.key() + '=' + encodeValue(iterator.value()));
    }
    return tokens.join(';');
}

inline QList<models::RateViewModel> sampleRates() {
    return {
        models::RateViewModel{
            .sourceName = "ECB",
            .baseCurrency = "EUR",
            .quoteCurrency = "USD",
            .rate = 1.081234,
            .timestamp = QDateTime(QDate(2026, 4, 18), QTime(10, 30), Qt::UTC),
            .note = "fresh",
        },
        models::RateViewModel{
            .sourceName = "Frankfurter",
            .baseCurrency = "EUR",
            .quoteCurrency = "GBP",
            .rate = 0.853210,
            .timestamp = QDateTime(QDate(2026, 4, 18), QTime(10, 30), Qt::UTC),
            .note = "cached",
        },
    };
}

inline QList<models::HistoryPointViewModel> sampleHistory() {
    return {
        models::HistoryPointViewModel{
            .sourceName = "ECB",
            .baseCurrency = "EUR",
            .quoteCurrency = "USD",
            .rate = 1.071234,
            .timestamp = QDateTime(QDate(2026, 4, 17), QTime(10, 30), Qt::UTC),
        },
        models::HistoryPointViewModel{
            .sourceName = "ECB",
            .baseCurrency = "EUR",
            .quoteCurrency = "USD",
            .rate = 1.081234,
            .timestamp = QDateTime(QDate(2026, 4, 18), QTime(10, 30), Qt::UTC),
        },
    };
}

inline QList<dto::ChartPointDto> sampleChartPoints() {
    return {
        dto::ChartPointDto{
            .timestamp = QDateTime(QDate(2026, 4, 17), QTime(10, 30), Qt::UTC),
            .value = 1.071234,
        },
        dto::ChartPointDto{
            .timestamp = QDateTime(QDate(2026, 4, 18), QTime(10, 30), Qt::UTC),
            .value = 1.081234,
        },
    };
}

class MockLineServer {
public:
    explicit MockLineServer(std::deque<QString> responses)
        : responses_(std::move(responses)) {
        std::promise<quint16> readyPromise;
        auto readyFuture = readyPromise.get_future();
        thread_ = std::thread([this, promise = std::move(readyPromise)]() mutable {
            run(std::move(promise));
        });
        port_ = readyFuture.get();
    }

    ~MockLineServer() {
        stop();
    }

    quint16 port() const {
        return port_;
    }

    QStringList receivedLines() const {
        std::scoped_lock lock(mutex_);
        return receivedLines_;
    }

private:
    void run(std::promise<quint16> readyPromise) {
        QTcpServer server;
        if (!server.listen(QHostAddress::LocalHost, 0)) {
            readyPromise.set_value(0);
            return;
        }

        readyPromise.set_value(server.serverPort());

        while (!stopping_.load()) {
            if (!server.waitForNewConnection(100)) {
                continue;
            }

            std::unique_ptr<QTcpSocket> socket(server.nextPendingConnection());
            if (!socket) {
                continue;
            }

            while (!stopping_.load() && socket->state() == QAbstractSocket::ConnectedState) {
                if (!socket->waitForReadyRead(100)) {
                    continue;
                }

                while (socket->canReadLine()) {
                    const auto line = QString::fromUtf8(socket->readLine()).trimmed();
                    {
                        std::scoped_lock lock(mutex_);
                        receivedLines_.push_back(line);
                    }

                    QString response;
                    {
                        std::scoped_lock lock(mutex_);
                        if (!responses_.empty()) {
                            response = responses_.front();
                            responses_.pop_front();
                        }
                    }

                    if (!response.isEmpty()) {
                        socket->write(response.toUtf8());
                        socket->write("\n");
                        socket->flush();
                        socket->waitForBytesWritten(1000);
                    }
                }
            }
        }

        server.close();
    }

    void stop() {
        if (!thread_.joinable()) {
            return;
        }

        stopping_.store(true);

        if (port_ != 0) {
            QTcpSocket wakeSocket;
            wakeSocket.connectToHost(QHostAddress::LocalHost, port_);
            wakeSocket.waitForConnected(100);
            wakeSocket.write("\n");
            wakeSocket.flush();
            wakeSocket.disconnectFromHost();
            wakeSocket.waitForDisconnected(100);
        }

        thread_.join();
    }

    mutable std::mutex mutex_;
    std::deque<QString> responses_;
    QStringList receivedLines_;
    std::atomic_bool stopping_{false};
    std::thread thread_;
    quint16 port_{0};
};

inline common::Result<void> connectGateway(gateways::ServerGateway& gateway, const quint16 port) {
    return gateway.connectToServer("127.0.0.1", port);
}

inline bool waitUntil(std::function<bool()> predicate, const int timeoutMs = 1000, const int pollIntervalMs = 10) {
    const auto deadline = std::chrono::steady_clock::now() + std::chrono::milliseconds(timeoutMs);
    while (std::chrono::steady_clock::now() < deadline) {
        QCoreApplication::processEvents();
        if (predicate()) {
            return true;
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(pollIntervalMs));
    }
    QCoreApplication::processEvents();
    return predicate();
}

}
