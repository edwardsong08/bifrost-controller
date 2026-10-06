// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once

#include <QDateTime>
#include <QJsonObject>
#include <QLocalServer>
#include <QObject>
#include <functional>

namespace Troa {
class LocalApi : public QObject
{
  public:
    explicit LocalApi(std::function<QJsonObject(const QJsonObject &)> handler, QObject *parent = nullptr);
    bool setEnabled(bool enabled);
    bool isEnabled() const;
    QString errorString() const;
    QDateTime lastRequestAt() const;

  private:
    QLocalServer m_server;
    QDateTime m_lastRequestAt;
    std::function<QJsonObject(const QJsonObject &)> m_handler;
};
} // namespace Troa
