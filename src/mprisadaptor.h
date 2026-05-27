// Copyright (c) 2026 Sebastian Palencsar
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef MPRISADAPTOR_H
#define MPRISADAPTOR_H

#include <QDBusAbstractAdaptor>
#include <QVariantMap>
#include <QObject>
#include <QString>
#include <QStringList>

class QApplication;
class RadioBackend;
class BearPlayer;

class MprisRootAdaptor : public QDBusAbstractAdaptor
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "org.mpris.MediaPlayer2")

    Q_PROPERTY(bool CanQuit READ canQuit CONSTANT)
    Q_PROPERTY(bool Fullscreen READ fullscreen WRITE setFullscreen)
    Q_PROPERTY(bool CanSetFullscreen READ canSetFullscreen CONSTANT)
    Q_PROPERTY(bool CanRaise READ canRaise CONSTANT)
    Q_PROPERTY(bool HasTrackList READ hasTrackList CONSTANT)
    Q_PROPERTY(QString Identity READ identity CONSTANT)
    Q_PROPERTY(QString DesktopEntry READ desktopEntry CONSTANT)
    Q_PROPERTY(QStringList SupportedUriSchemes READ supportedUriSchemes CONSTANT)
    Q_PROPERTY(QStringList SupportedMimeTypes READ supportedMimeTypes CONSTANT)

public:
    explicit MprisRootAdaptor(RadioBackend *backend, QApplication *app, QObject *parent = nullptr);

    bool canQuit() const;
    bool fullscreen() const;
    void setFullscreen(bool value);
    bool canSetFullscreen() const;
    bool canRaise() const;
    bool hasTrackList() const;
    QString identity() const;
    QString desktopEntry() const;
    QStringList supportedUriSchemes() const;
    QStringList supportedMimeTypes() const;

public slots:
    void Raise();
    void Quit();

private:
    RadioBackend *m_backend;
    QApplication *m_app;
};

class MprisPlayerAdaptor : public QDBusAbstractAdaptor
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "org.mpris.MediaPlayer2.Player")

    Q_PROPERTY(QString PlaybackStatus READ playbackStatus)
    Q_PROPERTY(double Rate READ rate WRITE setRate)
    Q_PROPERTY(double MinimumRate READ minimumRate CONSTANT)
    Q_PROPERTY(double MaximumRate READ maximumRate CONSTANT)
    Q_PROPERTY(double Volume READ volume WRITE setVolume)
    Q_PROPERTY(QVariantMap Metadata READ metadata)
    Q_PROPERTY(qlonglong Position READ position)
    Q_PROPERTY(bool CanSeek READ canSeek CONSTANT)
    Q_PROPERTY(bool CanControl READ canControl CONSTANT)
    Q_PROPERTY(bool CanPlay READ canPlay CONSTANT)
    Q_PROPERTY(bool CanPause READ canPause CONSTANT)
    Q_PROPERTY(bool CanGoNext READ canGoNext CONSTANT)
    Q_PROPERTY(bool CanGoPrevious READ canGoPrevious CONSTANT)

public:
    explicit MprisPlayerAdaptor(RadioBackend *backend);

    QString playbackStatus() const;
    double rate() const;
    void setRate(double value);
    double minimumRate() const;
    double maximumRate() const;
    double volume() const;
    void setVolume(double value);
    QVariantMap metadata() const;
    qlonglong position() const;
    bool canSeek() const;
    bool canControl() const;
    bool canPlay() const;
    bool canPause() const;
    bool canGoNext() const;
    bool canGoPrevious() const;

public slots:
    void Play();
    void Pause();
    void PlayPause();
    void Stop();
    void Next();
    void Previous();

private:
    BearPlayer *player() const;

    RadioBackend *m_backend;
};

#endif
