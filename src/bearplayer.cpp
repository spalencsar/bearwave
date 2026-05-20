#include "bearplayer.h"

#include <QUrl>
#include <QDebug>

namespace {
QString firstMetaValue(const QMultiMap<QString, QString> &meta, const QStringList &keys)
{
    for (const QString &key : keys) {
        if (meta.contains(key)) {
            const QString value = meta.value(key).trimmed();
            if (!value.isEmpty()) {
                return value;
            }
        }
    }
    return QString();
}
}

BearPlayer::BearPlayer(QObject *parent)
    : QObject(parent)
{
    m_mediaObject = new Phonon::MediaObject(this);
    m_audioOutput = new Phonon::AudioOutput(Phonon::MusicCategory, this);

    m_coverArtFetcher = new CoverArtFetcher(this);
    connect(m_coverArtFetcher, &CoverArtFetcher::coverUrlReady, this, &BearPlayer::onCoverUrlReady);

    Phonon::createPath(m_mediaObject, m_audioOutput);

    connect(m_mediaObject, &Phonon::MediaObject::stateChanged,
            this, &BearPlayer::onStateChanged);
    connect(m_mediaObject, &Phonon::MediaObject::metaDataChanged,
            this, &BearPlayer::onMetaDataChanged);

    m_retryTimer.setSingleShot(true);
    m_retryTimer.setInterval(1200);
    connect(&m_retryTimer, &QTimer::timeout, this, [this]() {
        if (m_lastUrl.isEmpty() || m_playing || m_retryAttempts >= 2) {
            return;
        }
        ++m_retryAttempts;
        m_mediaObject->setCurrentSource(QUrl(m_lastUrl));
        m_mediaObject->play();
        qDebug() << "Retry stream" << m_retryAttempts << m_lastName;
    });
}

BearPlayer::~BearPlayer()
{
    stop();
}

void BearPlayer::playUrl(const QString &url, const QString &name)
{
    if (url.isEmpty()) {
        return;
    }

    m_currentStationName = name;
    m_lastName = name;
    m_lastUrl = url;
    m_retryAttempts = 0;
    emit currentStationChanged(m_currentStationName);
    clearTrackInfo();

    m_mediaObject->setCurrentSource(QUrl(url));
    m_mediaObject->play();

    qDebug() << "Playing:" << name << url;
}

void BearPlayer::stop()
{
    m_mediaObject->stop();
    m_currentStationName.clear();
    m_lastName.clear();
    m_lastUrl.clear();
    m_retryAttempts = 0;
    emit currentStationChanged(QString());
    clearTrackInfo();
}

void BearPlayer::togglePlayPause()
{
    if (m_playing) {
        m_mediaObject->pause();
    } else {
        m_mediaObject->play();
    }
}

void BearPlayer::setVolume(qreal vol)
{
    m_audioOutput->setVolume(qBound(0.0, vol, 1.0));
    emit volumeChanged(m_audioOutput->volume());
}

void BearPlayer::onStateChanged(int state)
{
    switch (state) {
    case 2:
        m_playing = true;
        break;
    default:
        m_playing = false;
        scheduleRetry();
        break;
    }
    emit playingChanged(m_playing);
}

void BearPlayer::scheduleRetry()
{
    if (m_lastUrl.isEmpty() || m_retryAttempts >= 2 || m_playing) {
        return;
    }
    if (!m_retryTimer.isActive()) {
        m_retryTimer.start();
    }
}

void BearPlayer::onMetaDataChanged()
{
    const QMultiMap<QString, QString> meta = m_mediaObject->metaData();

    const QString artist = firstMetaValue(meta, {
        QStringLiteral("ARTIST"),
        QStringLiteral("artist"),
        QStringLiteral("Artist")
    });

    QString title = firstMetaValue(meta, {
        QStringLiteral("TITLE"),
        QStringLiteral("title"),
        QStringLiteral("Title")
    });

    if (title.isEmpty()) {
        title = firstMetaValue(meta, {
            QStringLiteral("icy-title"),
            QStringLiteral("Icy-Title"),
            QStringLiteral("StreamTitle")
        });
    }

    if (m_currentTrackArtist == artist && m_currentTrackTitle == title) {
        return;
    }

    m_currentTrackArtist = artist;
    m_currentTrackTitle = title;
    m_currentCoverArtUrl.clear();
    m_coverArtFetcher->fetch(artist, title);
    emit trackInfoChanged();
}

void BearPlayer::onCoverUrlReady(const QString &url)
{
    if (m_currentCoverArtUrl != url) {
        m_currentCoverArtUrl = url;
        emit trackInfoChanged();
    }
}

QString BearPlayer::currentNowPlaying() const
{
    if (!m_currentTrackArtist.isEmpty() && !m_currentTrackTitle.isEmpty()) {
        return m_currentTrackArtist + QStringLiteral(" - ") + m_currentTrackTitle;
    }
    if (!m_currentTrackTitle.isEmpty()) {
        return m_currentTrackTitle;
    }
    if (!m_currentTrackArtist.isEmpty()) {
        return m_currentTrackArtist;
    }
    return QString();
}

void BearPlayer::clearTrackInfo()
{
    if (m_currentTrackArtist.isEmpty() && m_currentTrackTitle.isEmpty()) {
        return;
    }
    m_currentTrackArtist.clear();
    m_currentTrackTitle.clear();
    m_currentCoverArtUrl.clear();
    m_coverArtFetcher->fetch(QString(), QString());
    emit trackInfoChanged();
}
