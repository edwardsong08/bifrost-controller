// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once

#include <QCryptographicHash>
#include <QDir>
#include <QStandardPaths>
#include <QString>

namespace Troa {
inline QString name() { return QStringLiteral("TROA PC Controller Mapper"); }
inline QString slug() { return QStringLiteral("troa-pc-controller-mapper"); }
inline QString projectUrl() { return QStringLiteral("https://github.com/edwardsong08/troa-pc-controller-mapper"); }
inline QString dataDirectory()
{
    return QDir(QStandardPaths::writableLocation(QStandardPaths::GenericDataLocation)).filePath(name());
}
inline QString profileDirectory() { return QDir(dataDirectory()).filePath(QStringLiteral("profiles")); }
inline QString socketName()
{
    const auto suffix = QCryptographicHash::hash(dataDirectory().toUtf8(), QCryptographicHash::Sha256).toHex().left(16);
    return QStringLiteral("troa-controller-mapper-api-") + QString::fromLatin1(suffix);
}
} // namespace Troa
