// Copyright (c) 2026 Sebastian Palencsar
// SPDX-License-Identifier: GPL-3.0-or-later

#include "nowplayingstate.h"

quint64 NowPlayingState::resetSource()
{
    ++m_sourceGeneration;
    m_artist.clear();
    m_title.clear();
    m_coverUrl.clear();
    return m_sourceGeneration;
}

bool NowPlayingState::updateMetadata(quint64 sourceGeneration, const QString &artist, const QString &title)
{
    if (sourceGeneration != m_sourceGeneration
        || (m_artist == artist && m_title == title)) {
        return false;
    }

    m_artist = artist;
    m_title = title;
    m_coverUrl.clear();
    return true;
}

bool NowPlayingState::updateCover(quint64 sourceGeneration, const QString &url)
{
    if (sourceGeneration != m_sourceGeneration || m_coverUrl == url) {
        return false;
    }

    m_coverUrl = url;
    return true;
}

bool NowPlayingState::hasTrackInfo() const
{
    return !m_artist.isEmpty() || !m_title.isEmpty() || !m_coverUrl.isEmpty();
}
