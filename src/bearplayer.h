#ifndef BEARPLAYER_H
#define BEARPLAYER_H

#include <QObject>
#include <QTimer>

#include <phonon4qt6/phonon/MediaObject>
#include <phonon4qt6/phonon/AudioOutput>
#include "coverartfetcher.h"

class BearPlayer : public QObject
{
    Q_OBJECT

    Q_PROPERTY(bool playing READ playing NOTIFY playingChanged)
    Q_PROPERTY(qreal volume READ volume WRITE setVolume NOTIFY volumeChanged)
    Q_PROPERTY(QString currentStationName READ currentStationName NOTIFY currentStationChanged)
    Q_PROPERTY(QString currentTrackTitle READ currentTrackTitle NOTIFY trackInfoChanged)
    Q_PROPERTY(QString currentTrackArtist READ currentTrackArtist NOTIFY trackInfoChanged)
    Q_PROPERTY(QString currentNowPlaying READ currentNowPlaying NOTIFY trackInfoChanged)
    Q_PROPERTY(QString currentCoverArtUrl READ currentCoverArtUrl NOTIFY trackInfoChanged)

public:
    explicit BearPlayer(QObject *parent = nullptr);
    ~BearPlayer();

    bool playing() const { return m_playing; }
    qreal volume() const { return m_audioOutput->volume(); }
    QString currentStationName() const { return m_currentStationName; }
    QString currentTrackTitle() const { return m_currentTrackTitle; }
    QString currentTrackArtist() const { return m_currentTrackArtist; }
    QString currentCoverArtUrl() const { return m_currentCoverArtUrl; }
    QString currentNowPlaying() const;

    Q_INVOKABLE void playUrl(const QString &url, const QString &name);
    Q_INVOKABLE void stop();
    Q_INVOKABLE void togglePlayPause();
    Q_INVOKABLE void setVolume(qreal vol);

signals:
    void playingChanged(bool playing);
    void volumeChanged(qreal volume);
    void currentStationChanged(const QString &stationName);
    void trackInfoChanged();

private slots:
    void onStateChanged(int state);
    void onMetaDataChanged();
    void onCoverUrlReady(const QString &url);

private:
    void clearTrackInfo();
    void scheduleRetry();

    Phonon::MediaObject *m_mediaObject = nullptr;
    Phonon::AudioOutput *m_audioOutput = nullptr;
    QTimer m_retryTimer;
    QString m_currentStationName;
    QString m_currentTrackTitle;
    QString m_currentTrackArtist;
    QString m_lastUrl;
    QString m_lastName;
    QString m_currentCoverArtUrl;
    CoverArtFetcher *m_coverArtFetcher = nullptr;
    int m_retryAttempts = 0;
    bool m_playing = false;
};

#endif
