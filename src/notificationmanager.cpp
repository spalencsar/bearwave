#include "notificationmanager.h"
#include "radiobackend.h"
#include "bearplayer.h"

#include <QDBusInterface>
#include <QDBusReply>
#include <QDBusMessage>
#include <QDBusConnection>
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
            m_currentReply->abort();
            m_currentReply->deleteLater();
            m_currentReply = nullptr;
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
            m_currentReply->abort();
            m_currentReply->deleteLater();
            m_currentReply = nullptr;
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

    if (coverUrl.isEmpty()) {
        if (!m_notifyTimer.isActive()) {
            m_notifyTimer.start(800);
        }
    } else {
        m_notifiedCoverUrl = coverUrl;
        const QString hash = QString(QCryptographicHash::hash(coverUrl.toUtf8(), QCryptographicHash::Md5).toHex());
        const QString filePath = m_coversDir + "/" + hash + ".jpg";

        if (QFile::exists(filePath)) {
            m_activeCoverPath = filePath;
            triggerNotification();
        } else {
            downloadCover(coverUrl);
            if (!m_notifyTimer.isActive()) {
                m_notifyTimer.start(800);
            }
        }
    }
}

void NotificationManager::downloadCover(const QString &url)
{
    if (m_currentReply) {
        m_currentReply->abort();
        m_currentReply->deleteLater();
        m_currentReply = nullptr;
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
    hints.insert(QStringLiteral("desktop-entry"), QStringLiteral("org.kde.bearwave"));
    if (!coverPath.isEmpty()) {
        hints.insert(QStringLiteral("image-path"), coverPath);
        hints.insert(QStringLiteral("image_path"), coverPath);
    }

    QList<QVariant> args;
    args << QStringLiteral("BearWave")
         << m_lastNotificationId
         << QStringLiteral("org.kde.bearwave")
         << summary
         << body
         << QStringList()
         << hints
         << -1;

    QDBusReply<uint> dbusReply = notifyInterface.callWithArgumentList(
        QDBus::Block,
        QStringLiteral("Notify"),
        args
    );

    if (dbusReply.isValid()) {
        m_lastNotificationId = dbusReply.value();
    } else {
        qWarning() << "Failed to send notification via DBus:" << dbusReply.error().message();
    }
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
