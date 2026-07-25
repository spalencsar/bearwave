// Copyright (c) 2026 Sebastian Palencsar
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef NOWPLAYINGSTATE_H
#define NOWPLAYINGSTATE_H

#include <QString>
#include <QtGlobal>

class NowPlayingState
{
public:
    quint64 resetSource();
    bool updateMetadata(quint64 sourceGeneration, const QString &artist, const QString &title);
    bool updateCover(quint64 sourceGeneration, const QString &url);

    quint64 sourceGeneration() const { return m_sourceGeneration; }
    QString artist() const { return m_artist; }
    QString title() const { return m_title; }
    QString coverUrl() const { return m_coverUrl; }
    bool hasTrackInfo() const;

private:
    quint64 m_sourceGeneration = 0;
    QString m_artist;
    QString m_title;
    QString m_coverUrl;
};

#endif // NOWPLAYINGSTATE_H
