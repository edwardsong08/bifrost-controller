// SPDX-License-Identifier: GPL-3.0-or-later
#include "localapi.h"
#include "identity.h"
#include "profilestore.h"

#include <QJsonDocument>
#include <QLocalSocket>
#include <QTimer>
#include <QVariant>

namespace Troa {
LocalApi::LocalApi(std::function<QJsonObject(const QJsonObject &)> handler, QObject *parent)
    : QObject(parent), m_handler(std::move(handler))
{
    m_server.setSocketOptions(QLocalServer::UserAccessOption);
    m_server.setMaxPendingConnections(4);
    connect(&m_server, &QLocalServer::newConnection, this, [this]() {
        while (m_server.hasPendingConnections()) {
            auto socket = m_server.nextPendingConnection();
            if (!socket) continue;
            socket->setReadBufferSize(256 * 1024 + 1);
            connect(socket, &QLocalSocket::disconnected, socket, &QObject::deleteLater);
            auto timeout = new QTimer(socket);
            timeout->setSingleShot(true);
            connect(timeout, &QTimer::timeout, socket, &QLocalSocket::abort);
            timeout->start(10000);
            const auto respond = [this, socket, timeout]() {
                if (socket->property("answered").toBool()) return;
                auto buffer = socket->property("request").toByteArray() + socket->readAll();
                if (buffer.size() > 256 * 1024) { socket->abort(); return; }
                socket->setProperty("request", buffer);
                const int newline = buffer.indexOf('\n');
                if (newline < 0) return;
                socket->setProperty("answered", true);
                timeout->stop();
                QJsonParseError error;
                const auto doc = QJsonDocument::fromJson(buffer.left(newline), &error);
                const auto response = error.error == QJsonParseError::NoError && doc.isObject()
                    ? m_handler(doc.object()) : failure("Expected one JSON object.");
                socket->write(QJsonDocument(response).toJson(QJsonDocument::Compact) + '\n');
                socket->disconnectFromServer();
            };
            connect(socket, &QLocalSocket::readyRead, socket, respond);
            if (socket->bytesAvailable()) respond();
        }
    });
}
bool LocalApi::setEnabled(bool enabled)
{
    if (!enabled) {
        m_server.close();
        // Disabling also closes connections accepted before the switch changed.
        for (auto socket : m_server.findChildren<QLocalSocket *>()) socket->abort();
        return true;
    }
    if (m_server.isListening()) return true;
    QLocalSocket probe;
    probe.connectToServer(socketName());
    if (probe.waitForConnected(100)) return false;
    QLocalServer::removeServer(socketName());
    return m_server.listen(socketName());
}
bool LocalApi::isEnabled() const { return m_server.isListening(); }
QString LocalApi::errorString() const { return m_server.errorString(); }
} // namespace Troa
