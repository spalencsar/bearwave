#ifndef NOTIFICATIONMANAGER_H
#define NOTIFICATIONMANAGER_H

#include <QObject>
#include <QString>
#include <QTimer>
#include <QNetworkAccessManager>
#include <QNetworkReply>

class RadioBackend;
class BearPlayer;

class NotificationManager : public QObject
{
    Q_OBJECT

public:
    explicit NotificationManager(RadioBackend *backend, QObject *parent = nullptr);
    ~NotificationManager();

private slots:
    void onTrackInfoChanged();
    void onDownloadFinished();
    void onNotifyTimeout();

private:
    void showNotification(const QString &station, const QString &artist, const QString &title, const QString &coverPath);
    void closeNotification();
    void downloadCover(const QString &url);
    void triggerNotification();

    RadioBackend *m_backend = nullptr;
    BearPlayer *m_player = nullptr;
    QNetworkAccessManager *m_networkManager = nullptr;
    QNetworkReply *m_currentReply = nullptr;
    QTimer m_notifyTimer;

    quint32 m_lastNotificationId = 0;
    QString m_coversDir;

    QString m_lastArtist;
    QString m_lastTitle;
    QString m_lastStation;
    QString m_notifiedCoverUrl;
    QString m_pendingCoverUrl;
    QString m_activeCoverPath;
    bool m_notificationShown = false;
};

#endif // NOTIFICATIONMANAGER_H
