// Copyright (c) 2026 Sebastian Palencsar
// SPDX-License-Identifier: GPL-3.0-or-later

#include "bearplayer.h"
#include "streamurl.h"

#include <QDebug>
#include <QMediaMetaData>
#include <QUrl>

BearPlayer::BearPlayer(QObject *parent) : QObject(parent) {
  m_audioOutput = new QAudioOutput(this);
  m_mediaPlayer = new QMediaPlayer(this);
  m_mediaPlayer->setAudioOutput(m_audioOutput);

  m_coverArtFetcher = new CoverArtFetcher(this);
  connect(m_coverArtFetcher, &CoverArtFetcher::coverUrlReady, this,
          &BearPlayer::onCoverUrlReady);

  m_icyReader = new IcyReader(this);
  connect(m_icyReader, &IcyReader::metaDataReceived, this,
          &BearPlayer::onIcyMetaDataReceived);

  connect(m_mediaPlayer, &QMediaPlayer::playbackStateChanged, this,
          &BearPlayer::onPlaybackStateChanged);
  connect(m_mediaPlayer, &QMediaPlayer::mediaStatusChanged, this,
          &BearPlayer::onMediaStatusChanged);
  connect(m_mediaPlayer, &QMediaPlayer::metaDataChanged, this,
          &BearPlayer::onMetaDataChanged);
  connect(m_mediaPlayer, &QMediaPlayer::errorOccurred, this,
          [this](QMediaPlayer::Error error, const QString &) {
    if (error != QMediaPlayer::NoError && m_hasActiveSource) {
      setPlaying(false);
      scheduleRetry();
    }
  });

  m_retryTimer.setSingleShot(true);
  m_retryTimer.setInterval(1200);
  connect(&m_retryTimer, &QTimer::timeout, this, [this]() {
    if (m_lastUrl.isEmpty() || m_playing || m_retryAttempts >= 2) {
      if (m_hasActiveSource && !m_playing && m_retryAttempts >= 2) {
        setConnectionState(QStringLiteral("error"));
      }
      return;
    }
    ++m_retryAttempts;
    setConnectionState(QStringLiteral("connecting"));
    m_changingSource = true;
    m_mediaPlayer->setSource(QUrl(m_lastUrl));
    m_mediaPlayer->play();
    m_changingSource = false;
    qDebug() << "Retry stream" << m_retryAttempts << m_lastName;
  });
}

BearPlayer::~BearPlayer() { stop(); }

void BearPlayer::playUrl(const QString &url, const QString &name) {
  if (url.isEmpty() || !isAllowedStreamUrl(url)) {
    qWarning() << "BearPlayer: rejected stream URL with disallowed scheme";
    return;
  }

  m_retryTimer.stop();
  m_currentStationName = name;
  m_lastName = name;
  m_lastUrl = url;
  m_retryAttempts = 0;
  m_hasActiveSource = true;
  emit currentStationChanged(m_currentStationName);
  clearTrackInfo();

  setConnectionState(QStringLiteral("connecting"));
  m_changingSource = true;
  m_mediaPlayer->setSource(QUrl(url));
  m_mediaPlayer->play();
  m_changingSource = false;
  m_icyReader->start(url, m_nowPlayingState.sourceGeneration());

  qDebug() << "Playing:" << name << url;
}

void BearPlayer::stop() {
  m_retryTimer.stop();
  m_hasActiveSource = false;
  m_currentStationName.clear();
  m_lastName.clear();
  m_lastUrl.clear();
  m_retryAttempts = 0;
  m_mediaPlayer->stop();
  m_icyReader->stop();
  setPlaying(false);
  setConnectionState(QStringLiteral("idle"));
  emit currentStationChanged(QString());
  clearTrackInfo();
}

void BearPlayer::togglePlayPause() {
  if (!m_hasActiveSource) {
    return;
  }
  if (m_playing) {
    m_mediaPlayer->pause();
  } else {
    if (m_connectionState == QStringLiteral("error")) {
      m_retryAttempts = 0;
    }
    setConnectionState(QStringLiteral("connecting"));
    m_mediaPlayer->play();
  }
}

void BearPlayer::setVolume(qreal vol) {
  m_audioOutput->setVolume(qBound(0.0, vol, 1.0));
  emit volumeChanged(m_audioOutput->volume());
}

void BearPlayer::onPlaybackStateChanged(QMediaPlayer::PlaybackState state) {
  if (state == QMediaPlayer::PlayingState) {
    setPlaying(true);
    setConnectionState(QStringLiteral("playing"));
  } else if (state == QMediaPlayer::PausedState) {
    setPlaying(false);
    setConnectionState(QStringLiteral("paused"));
  } else {
    setPlaying(false);
  }

  if (state == QMediaPlayer::StoppedState && !m_changingSource) {
    scheduleRetry();
  }
}

void BearPlayer::onMediaStatusChanged(QMediaPlayer::MediaStatus status) {
  switch (status) {
  case QMediaPlayer::LoadingMedia:
    if (m_hasActiveSource) {
      setConnectionState(QStringLiteral("connecting"));
    }
    break;
  case QMediaPlayer::BufferingMedia:
  case QMediaPlayer::StalledMedia:
    if (m_hasActiveSource) {
      setConnectionState(
          m_mediaPlayer->playbackState() == QMediaPlayer::PausedState
              ? QStringLiteral("paused")
              : QStringLiteral("buffering"));
    }
    break;
  case QMediaPlayer::LoadedMedia:
  case QMediaPlayer::BufferedMedia:
    if (m_hasActiveSource) {
      if (m_mediaPlayer->playbackState() == QMediaPlayer::PlayingState) {
        setConnectionState(QStringLiteral("playing"));
      } else if (m_mediaPlayer->playbackState() == QMediaPlayer::PausedState) {
        setConnectionState(QStringLiteral("paused"));
      } else {
        setConnectionState(QStringLiteral("connecting"));
      }
    }
    break;
  case QMediaPlayer::InvalidMedia:
  case QMediaPlayer::EndOfMedia:
    setPlaying(false);
    scheduleRetry();
    break;
  case QMediaPlayer::NoMedia:
    if (!m_hasActiveSource) {
      setConnectionState(QStringLiteral("idle"));
    }
    break;
  default:
    break;
  }
}

void BearPlayer::scheduleRetry() {
  if (!m_hasActiveSource || m_lastUrl.isEmpty() || m_playing) {
    return;
  }
  if (m_retryAttempts >= 2) {
    setConnectionState(QStringLiteral("error"));
    return;
  }
  if (!m_retryTimer.isActive()) {
    setConnectionState(QStringLiteral("retrying"));
    m_retryTimer.start();
  }
}

void BearPlayer::setConnectionState(const QString &state) {
  if (m_connectionState == state) {
    return;
  }
  m_connectionState = state;
  emit connectionStateChanged();
}

void BearPlayer::setPlaying(bool playing) {
  if (m_playing == playing) {
    return;
  }
  m_playing = playing;
  emit playingChanged(m_playing);
}

void BearPlayer::onMetaDataChanged() {
  QMediaMetaData meta = m_mediaPlayer->metaData();

  QString artist = meta.stringValue(QMediaMetaData::ContributingArtist);
  if (artist.isEmpty()) {
    artist = meta.stringValue(QMediaMetaData::Author);
  }

  QString title = meta.stringValue(QMediaMetaData::Title);

  // Ignore completely empty metadata from QMediaPlayer so we don't overwrite
  // IcyReader
  if (artist.isEmpty() && title.isEmpty()) {
    return;
  }

  const quint64 sourceGeneration = m_nowPlayingState.sourceGeneration();
  if (!m_nowPlayingState.updateMetadata(sourceGeneration, artist, title)) {
    return;
  }

  m_coverArtFetcher->fetch(artist, title, sourceGeneration);
  emit trackInfoChanged();
}

void BearPlayer::onCoverUrlReady(const QString &url, quint64 sourceGeneration) {
  if (m_nowPlayingState.updateCover(sourceGeneration, url)) {
    emit trackInfoChanged();
  }
}

void BearPlayer::onIcyMetaDataReceived(const QString &artist,
                                       const QString &title,
                                       quint64 sourceGeneration) {
  if (!m_nowPlayingState.updateMetadata(sourceGeneration, artist, title)) {
    return;
  }

  m_coverArtFetcher->fetch(artist, title, sourceGeneration);
  emit trackInfoChanged();
}

QString BearPlayer::currentNowPlaying() const {
  const QString artist = m_nowPlayingState.artist();
  const QString title = m_nowPlayingState.title();
  if (!artist.isEmpty() && !title.isEmpty()) {
    return artist + QStringLiteral(" - ") + title;
  }
  if (!title.isEmpty()) {
    return title;
  }
  if (!artist.isEmpty()) {
    return artist;
  }
  return QString();
}

void BearPlayer::clearTrackInfo() {
  m_coverArtFetcher->cancel();

  const bool hadTrackInfo = m_nowPlayingState.hasTrackInfo();
  m_nowPlayingState.resetSource();
  if (hadTrackInfo) {
    emit trackInfoChanged();
  }
}
