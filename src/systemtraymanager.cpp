// Copyright (c) 2026 Sebastian Palencsar
// SPDX-License-Identifier: GPL-3.0-or-later

#include "systemtraymanager.h"

#include "radiobackend.h"
#include "bearplayer.h"

#include <QApplication>
#include <QAction>
#include <QIcon>
#include <QMenu>
#include <QSystemTrayIcon>
#include <QWindow>

SystemTrayManager::SystemTrayManager(RadioBackend *backend, QWindow *window, QApplication *app, QObject *parent)
    : QObject(parent)
    , m_backend(backend)
    , m_window(window)
    , m_app(app)
{
    if (!QSystemTrayIcon::isSystemTrayAvailable() || !m_backend || !m_window || !m_app) {
        return;
    }

    setupTrayIcon();
}

SystemTrayManager::~SystemTrayManager()
{
    if (m_trayIcon) {
        m_trayIcon->hide();
    }
}

bool SystemTrayManager::isAvailable() const
{
    return m_trayIcon != nullptr;
}

void SystemTrayManager::setupTrayIcon()
{
    m_trayIcon = new QSystemTrayIcon(this);

    QIcon icon = QIcon::fromTheme(QStringLiteral("de.nerdbear.bearwave"));
    if (icon.isNull()) {
        icon = QIcon::fromTheme(QStringLiteral("multimedia-player"));
    }
    m_trayIcon->setIcon(icon);
    m_trayIcon->setToolTip(tr("BearWave"));

    m_menu = new QMenu();

    m_playPauseAction = m_menu->addAction(QString());
    connect(m_playPauseAction, &QAction::triggered, this, &SystemTrayManager::togglePlayback);

    m_visibilityAction = m_menu->addAction(QString());
    connect(m_visibilityAction, &QAction::triggered, this, &SystemTrayManager::toggleWindowVisibility);

    m_menu->addSeparator();

    m_quitAction = m_menu->addAction(tr("Quit"));
    connect(m_quitAction, &QAction::triggered, this, &SystemTrayManager::quitApplication);

    m_trayIcon->setContextMenu(m_menu);

    connect(m_trayIcon, &QSystemTrayIcon::activated, this, &SystemTrayManager::onTrayActivated);

    BearPlayer *player = m_backend->player();
    if (player) {
        connect(player, &BearPlayer::playingChanged, this, &SystemTrayManager::updatePlayPauseAction);
    }

    connect(m_window, &QWindow::visibilityChanged, this, &SystemTrayManager::updateVisibilityAction);
    connect(m_backend, &RadioBackend::raiseRequested, this, &SystemTrayManager::showAndActivateWindow);

    updatePlayPauseAction();
    updateVisibilityAction();

    m_trayIcon->show();
}

void SystemTrayManager::updatePlayPauseAction()
{
    if (!m_playPauseAction || !m_backend || !m_backend->player()) {
        return;
    }

    m_playPauseAction->setText(m_backend->player()->playing() ? tr("Pause") : tr("Play"));
}

void SystemTrayManager::updateVisibilityAction()
{
    if (!m_visibilityAction || !m_window) {
        return;
    }

    m_visibilityAction->setText(m_window->isVisible() ? tr("Hide") : tr("Show"));
}

void SystemTrayManager::togglePlayback()
{
    if (m_backend && m_backend->player()) {
        m_backend->player()->togglePlayPause();
    }
}

void SystemTrayManager::toggleWindowVisibility()
{
    if (!m_window) {
        return;
    }

    if (m_window->isVisible()) {
        m_window->hide();
    } else {
        showAndActivateWindow();
    }
}

void SystemTrayManager::showAndActivateWindow()
{
    if (!m_window) {
        return;
    }

    m_window->show();
    m_window->raise();
    m_window->requestActivate();
}

void SystemTrayManager::quitApplication()
{
    if (m_app) {
        m_app->quit();
    }
}

void SystemTrayManager::onTrayActivated(QSystemTrayIcon::ActivationReason reason)
{
    if (reason == QSystemTrayIcon::Trigger) {
        toggleWindowVisibility();
    }
}