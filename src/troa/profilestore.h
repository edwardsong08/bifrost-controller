// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once

#include <QJsonArray>
#include <QJsonObject>
#include <QString>
#include <QStringList>

namespace Troa {
class ProfileStore
{
  public:
    QJsonArray list() const;
    QJsonObject read(const QString &id) const;
    QJsonObject save(const QJsonObject &profile, const QString &expectedRevision) const;
    QJsonArray revisions(const QString &id) const;
    QJsonObject restore(const QString &id, const QString &revision, const QString &expectedRevision) const;
    QString exportMapping(const QString &id) const;
    static QString validate(const QJsonObject &profile);
    static QString revision(const QJsonObject &profile);
    static QStringList inputs();
    static QStringList namedKeys();

  private:
    static QString path(const QString &id);
    static bool validId(const QString &id);
    static QJsonArray catalog();
};
QJsonObject failure(const QString &message);
} // namespace Troa
