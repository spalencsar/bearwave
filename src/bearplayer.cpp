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

  m_retryTimer.setSingleShot(true);
  m_retryTimer.setInterval(1200);
  connect(&m_retryTimer, &QTimer::timeout, this, [this]() {
    if (m_lastUrl.isEmpty() || m_playing || m_retryAttempts >= 2) {
      return;
    }
    ++m_retryAttempts;
    m_mediaPlayer->setSource(QUrl(m_lastUrl));
    m_mediaPlayer->play();
    qDebug() << "Retry stream" << m_retryAttempts << m_lastName;
  });
}

BearPlayer::~BearPlayer() { stop(); }

void BearPlayer::playUrl(const QString &url, const QString &name) {
  if (url.isEmpty() || !isAllowedStreamUrl(url)) {
    qWarning() << "BearPlayer: rejected stream URL with disallowed scheme";
    return;
  }

  m_currentStationName = name;
  m_lastName = name;
  m_lastUrl = url;
  m_retryAttempts = 0;
  emit currentStationChanged(m_currentStationName);
  clearTrackInfo();

  m_mediaPlayer->setSource(QUrl(url));
  m_mediaPlayer->play();
  m_icyReader->start(url);

  qDebug() << "Playing:" << name << url;
}

void BearPlayer::stop() {
  m_mediaPlayer->stop();
  m_icyReader->stop();
  m_currentStationName.clear();
  m_lastName.clear();
  m_lastUrl.clear();
  m_retryAttempts = 0;
  emit currentStationChanged(QString());
  clearTrackInfo();
}

void BearPlayer::togglePlayPause() {
  if (m_playing) {
    m_mediaPlayer->pause();
  } else {
    m_mediaPlayer->play();
  }
}

void BearPlayer::setVolume(qreal vol) {
  m_audioOutput->setVolume(qBound(0.0, vol, 1.0));
  emit volumeChanged(m_audioOutput->volume());
}

void BearPlayer::onPlaybackStateChanged(QMediaPlayer::PlaybackState state) {
  m_playing = (state == QMediaPlayer::PlayingState);
  emit playingChanged(m_playing);

  if (state == QMediaPlayer::StoppedState) {
    scheduleRetry();
  }
}

void BearPlayer::onMediaStatusChanged(QMediaPlayer::MediaStatus status) {
  if (status == QMediaPlayer::InvalidMedia) {
    m_playing = false;
    emit playingChanged(m_playing);
    scheduleRetry();
  }
}

void BearPlayer::scheduleRetry() {
  if (m_lastUrl.isEmpty() || m_retryAttempts >= 2 || m_playing) {
    return;
  }
  if (!m_retryTimer.isActive()) {
    m_retryTimer.start();
  }
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

  if (m_currentTrackArtist == artist && m_currentTrackTitle == title) {
    return;
  }

  m_currentTrackArtist = artist;
  m_currentTrackTitle = title;
  m_currentCoverArtUrl.clear();
  m_coverArtFetcher->fetch(artist, title);
  emit trackInfoChanged();
}

void BearPlayer::onCoverUrlReady(const QString &url) {
  if (m_currentCoverArtUrl != url) {
    m_currentCoverArtUrl = url;
    emit trackInfoChanged();
  }
}

void BearPlayer::onIcyMetaDataReceived(const QString &artist,
                                       const QString &title) {
  if (m_currentTrackArtist == artist && m_currentTrackTitle == title) {
    return;
  }

  m_currentTrackArtist = artist;
  m_currentTrackTitle = title;
  m_currentCoverArtUrl.clear();
  m_coverArtFetcher->fetch(artist, title);
  emit trackInfoChanged();
}

QString BearPlayer::currentNowPlaying() const {
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

void BearPlayer::clearTrackInfo() {
  if (m_currentTrackArtist.isEmpty() && m_currentTrackTitle.isEmpty()) {
    return;
  }
  m_currentTrackArtist.clear();
  m_currentTrackTitle.clear();
  m_currentCoverArtUrl.clear();
  m_coverArtFetcher->fetch(QString(), QString());
  emit trackInfoChanged();
}
