// Copyright (c) 2026 Sebastian Palencsar
// SPDX-License-Identifier: GPL-3.0-or-later

#include "colorschemecontroller.h"

#include <QCoreApplication>
#include <QDBusConnection>
#include <QDBusMessage>
#include <QDBusReply>
#include <QDBusVariant>
#include <QDir>
#include <QFile>
#include <QGuiApplication>
#include <QJsonDocument>
#include <QJsonObject>
#include <QProcess>
#include <QStandardPaths>
#include <QStyleHints>
#include <QVariant>

ColorSchemeController::ColorSchemeController(QObject *parent)
    : QObject(parent)
{
    const QString sessionPath = dmsSessionPath();
    if (!sessionPath.isEmpty()) {
        const QString dir = QFileInfo(sessionPath).absolutePath();
        m_watcher.addPath(dir);
        if (QFile::exists(sessionPath)) {
            m_watcher.addPath(sessionPath);
        }
        connect(&m_watcher, &QFileSystemWatcher::fileChanged, this, [this, sessionPath](const QString &) {
            // Editors often replace the file; re-add path after change.
            if (QFile::exists(sessionPath) && !m_watcher.files().contains(sessionPath)) {
                m_watcher.addPath(sessionPath);
            }
            refresh();
        });
        connect(&m_watcher, &QFileSystemWatcher::directoryChanged, this, [this, sessionPath](const QString &) {
            if (QFile::exists(sessionPath) && !m_watcher.files().contains(sessionPath)) {
                m_watcher.addPath(sessionPath);
            }
            refresh();
        });
    }

    // Portal SettingChanged (org.freedesktop.appearance / color-scheme)
    QDBusConnection::sessionBus().connect(
        QStringLiteral("org.freedesktop.portal.Desktop"),
        QStringLiteral("/org/freedesktop/portal/desktop"),
        QStringLiteral("org.freedesktop.portal.Settings"),
        QStringLiteral("SettingChanged"),
        this,
        SLOT(refresh()));

    if (QGuiApplication::styleHints()) {
        connect(QGuiApplication::styleHints(), &QStyleHints::colorSchemeChanged,
                this, &ColorSchemeController::refresh);
    }

    refresh();
}

QString ColorSchemeController::dmsSessionPath()
{
    const QString stateHome = QStandardPaths::writableLocation(QStandardPaths::GenericStateLocation);
    if (stateHome.isEmpty()) {
        return QDir::homePath() + QStringLiteral("/.local/state/DankMaterialShell/session.json");
    }
    return stateHome + QStringLiteral("/DankMaterialShell/session.json");
}

bool ColorSchemeController::readDmsIsLightMode(bool *ok) const
{
    if (ok) {
        *ok = false;
    }
    const QString path = dmsSessionPath();
    QFile file(path);
    if (!file.exists() || !file.open(QIODevice::ReadOnly)) {
        return false;
    }
    const QJsonDocument doc = QJsonDocument::fromJson(file.readAll());
    if (!doc.isObject() || !doc.object().contains(QStringLiteral("isLightMode"))) {
        return false;
    }
    if (ok) {
        *ok = true;
    }
    return doc.object().value(QStringLiteral("isLightMode")).toBool();
}

int ColorSchemeController::readPortalColorScheme() const
{
    // xdg-desktop-portal: 0 = no preference, 1 = prefer dark, 2 = prefer light
    QDBusMessage msg = QDBusMessage::createMethodCall(
        QStringLiteral("org.freedesktop.portal.Desktop"),
        QStringLiteral("/org/freedesktop/portal/desktop"),
        QStringLiteral("org.freedesktop.portal.Settings"),
        QStringLiteral("Read"));
    msg << QStringLiteral("org.freedesktop.appearance") << QStringLiteral("color-scheme");
    const QDBusMessage reply = QDBusConnection::sessionBus().call(msg);
    if (reply.type() == QDBusMessage::ErrorMessage || reply.arguments().isEmpty()) {
        return -1;
    }

    QVariant value = reply.arguments().constFirst();
    // Nested QDBusVariant unwrap
    while (value.canConvert<QDBusVariant>()) {
        value = value.value<QDBusVariant>().variant();
    }
    bool ok = false;
    const uint scheme = value.toUInt(&ok);
    if (!ok) {
        return -1;
    }
    return static_cast<int>(scheme);
}

QString ColorSchemeController::readGsettingsColorScheme() const
{
    QProcess proc;
    proc.start(QStringLiteral("gsettings"),
               {QStringLiteral("get"),
                QStringLiteral("org.gnome.desktop.interface"),
                QStringLiteral("color-scheme")});
    if (!proc.waitForFinished(800)) {
        return {};
    }
    QString out = QString::fromUtf8(proc.readAllStandardOutput()).trimmed();
    // gsettings prints 'prefer-dark' with quotes
    if (out.startsWith(QLatin1Char('\'')) && out.endsWith(QLatin1Char('\''))) {
        out = out.mid(1, out.size() - 2);
    }
    return out;
}

bool ColorSchemeController::resolveLightMode() const
{
    // 1) Explicit override for debugging / packaging
    const QString envTheme = qEnvironmentVariable("BEARWAVE_THEME").trimmed().toLower();
    if (envTheme == QLatin1String("light") || envTheme == QLatin1String("lightmode")) {
        return true;
    }
    if (envTheme == QLatin1String("dark") || envTheme == QLatin1String("darkmode")) {
        return false;
    }

    // 2) Optional shell session file (e.g. DankMaterialShell) when present
    bool dmsOk = false;
    const bool dmsLight = readDmsIsLightMode(&dmsOk);
    if (dmsOk) {
        return dmsLight;
    }

    // 3) xdg-desktop-portal appearance
    const int portal = readPortalColorScheme();
    if (portal == 2) {
        return true;
    }
    if (portal == 1) {
        return false;
    }

    // 4) GSettings color-scheme
    const QString gset = readGsettingsColorScheme();
    if (gset == QLatin1String("prefer-light")) {
        return true;
    }
    if (gset == QLatin1String("prefer-dark")) {
        return false;
    }

    // 5) Qt styleHints (may stay Unknown/Dark when portal is 0)
    if (QGuiApplication::styleHints()) {
        const auto scheme = QGuiApplication::styleHints()->colorScheme();
        if (scheme == Qt::ColorScheme::Light) {
            return true;
        }
        if (scheme == Qt::ColorScheme::Dark) {
            return false;
        }
    }

    // Default pure dark when no preference is available
    return false;
}

void ColorSchemeController::refresh()
{
    const bool next = resolveLightMode();
    if (next == m_lightMode) {
        return;
    }
    m_lightMode = next;
    // Keep Qt/QML styleHints in sync so any Qt.colorScheme bindings update too.
    if (QGuiApplication::styleHints()) {
        QGuiApplication::styleHints()->setColorScheme(
            m_lightMode ? Qt::ColorScheme::Light : Qt::ColorScheme::Dark);
    }
    emit lightModeChanged();
    qInfo().noquote() << QStringLiteral("Color scheme: %1 (session/portal/gsettings/Qt)")
                             .arg(m_lightMode ? QStringLiteral("light") : QStringLiteral("dark"));
}
