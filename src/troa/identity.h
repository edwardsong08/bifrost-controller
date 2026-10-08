// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once

#include <QCoreApplication>
#include <QCryptographicHash>
#include <QDir>
#include <QFileInfo>
#include <QStandardPaths>
#include <QString>

namespace Troa {
inline QString name() { return QStringLiteral("Bifrost Controller"); }
inline QString slug() { return QStringLiteral("bifrost-controller"); }
inline QString projectUrl() { return QStringLiteral("https://github.com/edwardsong08/bifrost-controller"); }
inline QString applicationCommand()
{
#ifdef Q_OS_WIN
    const QDir root(QDir(QCoreApplication::applicationDirPath()).filePath("../../.."));
    const QString launcher = root.filePath(QStringLiteral("bifrost-launcher.exe"));
    if (QFileInfo::exists(root.filePath(QStringLiteral("current.txt"))) && QFileInfo::exists(launcher))
        return QDir::cleanPath(launcher);
#endif
    return QCoreApplication::applicationFilePath();
}
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
