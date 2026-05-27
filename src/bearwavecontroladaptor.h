// Copyright (c) 2026 Sebastian Palencsar
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef BEARWAVECONTROLADAPTOR_H
#define BEARWAVECONTROLADAPTOR_H

#include <QDBusAbstractAdaptor>
#include <QVariantList>

class RadioBackend;

class BearWaveControlAdaptor : public QDBusAbstractAdaptor
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "org.kde.BearWave.Control")

public:
    explicit BearWaveControlAdaptor(RadioBackend *backend);

public slots:
    QVariantList GetFavorites() const;
    QVariantList GetRecentStations() const;
    bool PlayFavoriteByUuid(const QString &uuid, const QString &urlResolved = QString());
    bool PlayRecentByUuid(const QString &uuid, const QString &urlResolved = QString());
    bool HasNext() const;
    bool HasPrevious() const;
    void PlayNext();
    void PlayPrevious();
    void PlayPause();
    void Stop();

signals:
    void StationsChanged();

private:
    RadioBackend *m_backend;
};

#endif
