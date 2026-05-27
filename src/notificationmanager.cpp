// Copyright (c) 2026 Sebastian Palencsar
// SPDX-License-Identifier: GPL-3.0-or-later

#include "notificationmanager.h"
#include "radiobackend.h"
#include "bearplayer.h"

#include <QDBusInterface>
#include <QDBusReply>
#include <QDBusMessage>
#include <QDBusConnection>
#include <QDBusPendingCallWatcher>
#include <QDBusPendingReply>
#include <QDir>
#include <QFile>
#include <QCryptographicHash>
#include <QStandardPaths>
#include <QDebug>

NotificationManager::NotificationManager(RadioBackend *backend, QObject *parent)
    : QObject(parent)
    , m_backend(backend)
    , m_player(backend->player())
    , m_notificationShown(false)
{
    m_coversDir = QDir::homePath() + QStringLiteral("/.cache/bearwave/covers");
    QDir().mkpath(m_coversDir);

    m_networkManager = new QNetworkAccessManager(this);

    m_notifyTimer.setSingleShot(true);
    connect(&m_notifyTimer, &QTimer::timeout, this, &NotificationManager::onNotifyTimeout);

    connect(m_player, &BearPlayer::trackInfoChanged, this, &NotificationManager::onTrackInfoChanged);
    connect(m_player, &BearPlayer::currentStationChanged, this, &NotificationManager::onTrackInfoChanged);
}

NotificationManager::~NotificationManager()
{
    closeNotification();
}

void NotificationManager::onTrackInfoChanged()
{
    const QString station = m_player->currentStationName();
    const QString artist = m_player->currentTrackArtist();
    const QString title = m_player->currentTrackTitle();
    const QString coverUrl = m_player->currentCoverArtUrl();

    if (artist.isEmpty() && title.isEmpty()) {
        m_notifyTimer.stop();
        if (m_currentReply) {
            QNetworkReply *reply = m_currentReply;
            m_currentReply = nullptr;
            reply->disconnect(this);
            reply->abort();
            reply->deleteLater();
        }
        closeNotification();
        m_lastArtist.clear();
        m_lastTitle.clear();
        m_lastStation.clear();
        m_notifiedCoverUrl.clear();
        m_activeCoverPath.clear();
        m_notificationShown = false;
        return;
    }

    bool trackChanged = (artist != m_lastArtist || title != m_lastTitle || station != m_lastStation);

    if (trackChanged) {
        m_notifyTimer.stop();
        if (m_currentReply) {
            QNetworkReply *reply = m_currentReply;
            m_currentReply = nullptr;
            reply->disconnect(this);
            reply->abort();
            reply->deleteLater();
        }
        m_lastArtist = artist;
        m_lastTitle = title;
        m_lastStation = station;
        m_notifiedCoverUrl.clear();
        m_activeCoverPath.clear();
        m_notificationShown = false;
    }

    if (m_notificationShown) {
        return;
    }

    QString targetUrl = coverUrl;
    if (targetUrl.isEmpty() && m_backend) {
        QObject *stationObj = m_backend->currentStation();
        if (stationObj) {
            targetUrl = stationObj->property("favicon").toString();
            if (!targetUrl.startsWith(QLatin1String("https://"))) {
                targetUrl.clear();
            }
        }
    }

    if (targetUrl.isEmpty()) {
        if (!m_notifyTimer.isActive()) {
            m_notifyTimer.start(800);
        }
    } else {
        m_notifiedCoverUrl = targetUrl;
        const QString hash = QString(QCryptographicHash::hash(targetUrl.toUtf8(), QCryptographicHash::Md5).toHex());
        const QString filePath = m_coversDir + "/" + hash + ".jpg";

        if (QFile::exists(filePath)) {
            m_activeCoverPath = filePath;
            triggerNotification();
        } else {
            downloadCover(targetUrl);
            if (!m_notifyTimer.isActive()) {
                m_notifyTimer.start(800);
            }
        }
    }
}

void NotificationManager::downloadCover(const QString &url)
{
    if (m_currentReply) {
        QNetworkReply *reply = m_currentReply;
        m_currentReply = nullptr;
        reply->disconnect(this);
        reply->abort();
        reply->deleteLater();
    }

    m_pendingCoverUrl = url;
    QNetworkRequest request(url);
    m_currentReply = m_networkManager->get(request);
    connect(m_currentReply, &QNetworkReply::finished, this, &NotificationManager::onDownloadFinished);
}

void NotificationManager::onDownloadFinished()
{
    QNetworkReply *reply = qobject_cast<QNetworkReply*>(sender());
    if (!reply) return;

    if (reply == m_currentReply) {
        m_currentReply = nullptr;
    }

    if (reply->error() == QNetworkReply::NoError) {
        QByteArray data = reply->readAll();
        const QString hash = QString(QCryptographicHash::hash(m_pendingCoverUrl.toUtf8(), QCryptographicHash::Md5).toHex());
        const QString filePath = m_coversDir + "/" + hash + ".jpg";

        QFile file(filePath);
        if (file.open(QIODevice::WriteOnly)) {
            file.write(data);
            file.close();
            
            m_activeCoverPath = filePath;
            triggerNotification();
        }
    } else {
        if (reply->error() != QNetworkReply::OperationCanceledError) {
            qDebug() << "Cover download failed:" << reply->errorString();
        }
    }

    reply->deleteLater();
}

void NotificationManager::onNotifyTimeout()
{
    triggerNotification();
}

void NotificationManager::triggerNotification()
{
    m_notifyTimer.stop();
    qDebug() << "Triggering notification - station:" << m_lastStation << "artist:" << m_lastArtist << "title:" << m_lastTitle << "cover:" << m_activeCoverPath;
    showNotification(m_lastStation, m_lastArtist, m_lastTitle, m_activeCoverPath);
    m_notificationShown = true;
}

void NotificationManager::showNotification(const QString &station, const QString &artist, const QString &title, const QString &coverPath)
{
    QDBusInterface notifyInterface(
        QStringLiteral("org.freedesktop.Notifications"),
        QStringLiteral("/org/freedesktop/Notifications"),
        QStringLiteral("org.freedesktop.Notifications"),
        QDBusConnection::sessionBus()
    );

    if (!notifyInterface.isValid()) {
        qWarning() << "DBus Notification interface is invalid";
        return;
    }

    QString summary;
    QString body;

    if (!title.isEmpty()) {
        summary = title;
        if (!artist.isEmpty()) {
            body = artist;
        }
        if (!station.isEmpty()) {
            if (!body.isEmpty()) body += QStringLiteral("\n");
            body += tr("Station: %1").arg(station);
        }
    } else {
        if (!station.isEmpty()) {
            summary = station;
        } else {
            summary = QStringLiteral("BearWave");
        }
        if (!artist.isEmpty()) {
            body = artist;
        }
    }

    QVariantMap hints;
    hints.insert(QStringLiteral("desktop-entry"), QStringLiteral("de.nerdbear.bearwave"));
    if (!coverPath.isEmpty()) {
        hints.insert(QStringLiteral("image-path"), coverPath);
        hints.insert(QStringLiteral("image_path"), coverPath);
    }

    QList<QVariant> args;
    args << QStringLiteral("BearWave")
         << m_lastNotificationId
         << QStringLiteral("de.nerdbear.bearwave")
         << summary
         << body
         << QStringList()
         << hints
         << -1;

    QDBusPendingCall pcall = notifyInterface.asyncCallWithArgumentList(
        QStringLiteral("Notify"),
        args
    );

    QDBusPendingCallWatcher *watcher = new QDBusPendingCallWatcher(pcall, this);
    connect(watcher, &QDBusPendingCallWatcher::finished, this, [this](QDBusPendingCallWatcher *watcher) {
        watcher->deleteLater();
        QDBusPendingReply<uint> reply = *watcher;
        if (reply.isValid()) {
            m_lastNotificationId = reply.value();
        } else {
            qWarning() << "Failed to send notification via DBus:" << reply.error().message();
        }
    });
}

void NotificationManager::closeNotification()
{
    if (m_lastNotificationId == 0) {
        return;
    }
    qDebug() << "Closing notification with ID:" << m_lastNotificationId;

    QDBusInterface notifyInterface(
        QStringLiteral("org.freedesktop.Notifications"),
        QStringLiteral("/org/freedesktop/Notifications"),
        QStringLiteral("org.freedesktop.Notifications"),
        QDBusConnection::sessionBus()
    );

    if (notifyInterface.isValid()) {
        notifyInterface.call(QStringLiteral("CloseNotification"), m_lastNotificationId);
    }

    m_lastNotificationId = 0;
}
