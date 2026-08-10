// Copyright (c) 2026 Sebastian Palencsar
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef COLORSCHEMECONTROLLER_H
#define COLORSCHEMECONTROLLER_H

#include <QObject>
#include <QFileSystemWatcher>
#include <QString>

// Resolves light/dark for Linux desktops.
// Priority: BEARWAVE_THEME env → optional shell session.json isLightMode →
// xdg portal color-scheme → gsettings → Qt styleHints.
class ColorSchemeController : public QObject
{
    Q_OBJECT
    Q_PROPERTY(bool lightMode READ lightMode NOTIFY lightModeChanged)

public:
    explicit ColorSchemeController(QObject *parent = nullptr);

    bool lightMode() const { return m_lightMode; }

public slots:
    void refresh();

signals:
    void lightModeChanged();

private:
    static QString dmsSessionPath();
    bool readDmsIsLightMode(bool *ok) const;
    int readPortalColorScheme() const; // 0 none, 1 dark, 2 light, -1 error
    QString readGsettingsColorScheme() const;
    bool resolveLightMode() const;

    bool m_lightMode = false;
    QFileSystemWatcher m_watcher;
};

#endif
