// Copyright (c) 2026 Sebastian Palencsar
// SPDX-License-Identifier: GPL-3.0-or-later

#include "stationlogostyle.h"

#include <QStringList>

QString StationLogoStyle::initials(const QString &stationName)
{
    const QStringList words =
        stationName.simplified().split(QLatin1Char(' '), Qt::SkipEmptyParts);
    if (words.isEmpty()) {
        return QStringLiteral("♫");
    }
    if (words.size() == 1) {
        return words.first().left(2).toUpper();
    }
    return (words.first().left(1) + words.at(1).left(1)).toUpper();
}

int StationLogoStyle::paletteIndex(const QString &stationKey, int paletteSize)
{
    if (paletteSize <= 0) {
        return 0;
    }
    quint32 hash = 2166136261u;
    for (const QChar character : stationKey) {
        hash ^= character.unicode();
        hash *= 16777619u;
    }
    return static_cast<int>(hash % static_cast<quint32>(paletteSize));
}
