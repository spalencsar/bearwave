// Copyright (c) 2026 Sebastian Palencsar
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef APPICON_H
#define APPICON_H

#include <QIcon>
#include <QString>

// Window/tray icon: prefer the bundled SVG so a stale theme PNG in
// ~/.local/share/icons cannot override the current branding. Theme lookup
// remains for desktop launchers (Icon=de.nerdbear.bearwave).
inline QIcon bearwaveAppIcon()
{
    QIcon svgIcon(QStringLiteral(":/assets/app/bearwave.svg"));
    if (!svgIcon.isNull()) {
        return svgIcon;
    }

    QIcon themeIcon = QIcon::fromTheme(QStringLiteral("de.nerdbear.bearwave"));
    if (!themeIcon.isNull()) {
        return themeIcon;
    }

    QIcon pngIcon(QStringLiteral(":/assets/app/bearwave.png"));
    if (!pngIcon.isNull()) {
        return pngIcon;
    }

    return QIcon::fromTheme(QStringLiteral("multimedia-player"));
}

#endif // APPICON_H
