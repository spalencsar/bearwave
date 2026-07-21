// Copyright (c) 2026 Sebastian Palencsar
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef STREAMURL_H
#define STREAMURL_H

#include <QString>
#include <QUrl>

// Only plain internet stream schemes are accepted for playback and storage.
inline bool isAllowedStreamUrl(const QString &urlString)
{
    const QUrl url(urlString.trimmed());
    if (!url.isValid() || url.isEmpty()) {
        return false;
    }
    const QString scheme = url.scheme().toLower();
    return scheme == QStringLiteral("http") || scheme == QStringLiteral("https");
}

#endif // STREAMURL_H
