// Copyright (c) 2026 Sebastian Palencsar
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef APPICON_H
#define APPICON_H

#include <QIcon>
#include <QString>

// Desktop/tray icon: installed theme name first (SVG scalable preferred),
// then bundled SVG/PNG for uninstalled development builds.
inline QIcon bearwaveAppIcon()
{
    QIcon themeIcon = QIcon::fromTheme(QStringLiteral("de.nerdbear.bearwave"));
    if (!themeIcon.isNull()) {
        return themeIcon;
    }

    QIcon svgIcon(QStringLiteral(":/assets/app/bearwave.svg"));
    if (!svgIcon.isNull()) {
        return svgIcon;
    }

    QIcon pngIcon(QStringLiteral(":/assets/app/bearwave.png"));
    if (!pngIcon.isNull()) {
        return pngIcon;
    }

    return QIcon::fromTheme(QStringLiteral("multimedia-player"));
}

#endif // APPICON_H
