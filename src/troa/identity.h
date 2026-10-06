// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once

#include <QCryptographicHash>
#include <QDir>
#include <QStandardPaths>
#include <QString>

namespace Troa {
inline QString name() { return QStringLiteral("Bifrost Controller"); }
inline QString slug() { return QStringLiteral("bifrost-controller"); }
inline QString projectUrl() { return QStringLiteral("https://github.com/edwardsong08/bifrost-controller"); }
inline QString dataDirectory()
{
    return QDir(QStandardPaths::writableLocation(QStandardPaths::GenericDataLocation))
        .filePath(QStringLiteral("TROA PC Controller Mapper")); // Preserve preview user data across rebranding.
}
inline QString profileDirectory() { return QDir(dataDirectory()).filePath(QStringLiteral("profiles")); }
inline QString socketName()
{
    const auto suffix = QCryptographicHash::hash(dataDirectory().toUtf8(), QCryptographicHash::Sha256).toHex().left(16);
    return QStringLiteral("bifrost-controller-api-") + QString::fromLatin1(suffix);
}
} // namespace Troa
