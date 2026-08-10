// Copyright (c) 2026 Sebastian Palencsar
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef BEARWAVEPATHS_H
#define BEARWAVEPATHS_H

#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QStandardPaths>
#include <QString>

// XDG-aware config/cache roots so Flatpak can persist under
// ~/.var/app/<app-id>/… without --filesystem overrides.
// Native installs keep ~/.config/bearwave and ~/.cache/bearwave when
// XDG_*_HOME are unset.

namespace BearwavePaths {

inline QString configDir()
{
    const QString base =
        QStandardPaths::writableLocation(QStandardPaths::GenericConfigLocation);
    const QString dir = base + QStringLiteral("/bearwave");
    QDir().mkpath(dir);
    return dir;
}

inline QString cacheDir()
{
    const QString base =
        QStandardPaths::writableLocation(QStandardPaths::GenericCacheLocation);
    const QString dir = base + QStringLiteral("/bearwave");
    QDir().mkpath(dir);
    return dir;
}

inline QString coversCacheDir()
{
    const QString dir = cacheDir() + QStringLiteral("/covers");
    QDir().mkpath(dir);
    return dir;
}

inline QString apiCacheDir()
{
    const QString dir = cacheDir() + QStringLiteral("/api_cache");
    QDir().mkpath(dir);
    return dir;
}

// One-shot copy of favorites/state/my_stations from the legacy host path
// (~/.config/bearwave) when the XDG dir is empty and the legacy dir is readable
// (e.g. native upgrade, or Flatpak with an old override).
inline void maybeMigrateLegacyConfig()
{
    const QString target = configDir();
    const QString legacy = QDir::homePath() + QStringLiteral("/.config/bearwave");
    if (QFileInfo::exists(target) && QDir(target).absolutePath() == QDir(legacy).absolutePath()) {
        return;
    }
    if (!QDir(legacy).exists()) {
        return;
    }

    const QStringList files = {
        QStringLiteral("favorites.json"),
        QStringLiteral("my_stations.json"),
        QStringLiteral("state.json"),
    };
    bool targetHasData = false;
    for (const QString &name : files) {
        if (QFile::exists(target + QLatin1Char('/') + name)) {
            targetHasData = true;
            break;
        }
    }
    if (targetHasData) {
        return;
    }

    for (const QString &name : files) {
        const QString src = legacy + QLatin1Char('/') + name;
        const QString dst = target + QLatin1Char('/') + name;
        if (QFile::exists(src) && !QFile::exists(dst)) {
            QFile::copy(src, dst);
        }
    }
}

} // namespace BearwavePaths

#endif
