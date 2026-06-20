// Copyright (c) 2026 Sebastian Palencsar
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef SYSTEMTRAYMANAGER_H
#define SYSTEMTRAYMANAGER_H

#include <QObject>
#include <QSystemTrayIcon>

class QWindow;
class QMenu;
class QAction;
class QApplication;
class RadioBackend;

class SystemTrayManager : public QObject
{
    Q_OBJECT

public:
    SystemTrayManager(RadioBackend *backend, QWindow *window, QApplication *app, QObject *parent = nullptr);
    ~SystemTrayManager() override;

    bool isAvailable() const;

private slots:
    void updatePlayPauseAction();
    void updateVisibilityAction();
    void togglePlayback();
    void toggleWindowVisibility();
    void showAndActivateWindow();
    void quitApplication();
    void onTrayActivated(QSystemTrayIcon::ActivationReason reason);

private:
    void setupTrayIcon();

    RadioBackend *m_backend = nullptr;
    QWindow *m_window = nullptr;
    QApplication *m_app = nullptr;
    QSystemTrayIcon *m_trayIcon = nullptr;
    QMenu *m_menu = nullptr;
    QAction *m_playPauseAction = nullptr;
    QAction *m_visibilityAction = nullptr;
    QAction *m_quitAction = nullptr;
};

#endif