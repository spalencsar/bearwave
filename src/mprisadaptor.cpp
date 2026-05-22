#include "mprisadaptor.h"

#include "radiobackend.h"
#include "bearplayer.h"

#include <QApplication>
#include <QDBusConnection>
#include <QDBusMessage>
#include <QDBusObjectPath>
#include <QStandardPaths>

namespace {
void emitPlayerPropertiesChanged(const QStringList &changedProps, const QVariantMap &changedValues)
{
    QDBusMessage signal = QDBusMessage::createSignal(
        QStringLiteral("/org/mpris/MediaPlayer2"),
        QStringLiteral("org.freedesktop.DBus.Properties"),
        QStringLiteral("PropertiesChanged"));

    signal << QStringLiteral("org.mpris.MediaPlayer2.Player")
           << changedValues
           << QStringList();

    QDBusConnection::sessionBus().send(signal);
    Q_UNUSED(changedProps)
}
}

MprisRootAdaptor::MprisRootAdaptor(RadioBackend *backend, QApplication *app, QObject *parent)
    : QDBusAbstractAdaptor(backend)
    , m_backend(backend)
    , m_app(app)
{
}

bool MprisRootAdaptor::canQuit() const
{
    return true;
}

bool MprisRootAdaptor::fullscreen() const
{
    return false;
}

void MprisRootAdaptor::setFullscreen(bool value)
{
    Q_UNUSED(value)
}

bool MprisRootAdaptor::canSetFullscreen() const
{
    return false;
}

bool MprisRootAdaptor::canRaise() const
{
    return false;
}

bool MprisRootAdaptor::hasTrackList() const
{
    return false;
}

QString MprisRootAdaptor::identity() const
{
    return QStringLiteral("BearWave");
}

QString MprisRootAdaptor::desktopEntry() const
{
    return QStringLiteral("org.kde.bearwave");
}

QStringList MprisRootAdaptor::supportedUriSchemes() const
{
    return {QStringLiteral("http"), QStringLiteral("https")};
}

QStringList MprisRootAdaptor::supportedMimeTypes() const
{
    return {
        QStringLiteral("audio/mpeg"),
        QStringLiteral("audio/aac"),
        QStringLiteral("audio/ogg"),
        QStringLiteral("application/x-mpegURL"),
        QStringLiteral("application/vnd.apple.mpegurl")
    };
}

void MprisRootAdaptor::Raise()
{
}

void MprisRootAdaptor::Quit()
{
    if (m_app) {
        m_app->quit();
    }
}

MprisPlayerAdaptor::MprisPlayerAdaptor(RadioBackend *backend)
    : QDBusAbstractAdaptor(backend)
    , m_backend(backend)
{
    BearPlayer *p = player();
    if (!p) {
        return;
    }

    connect(p, &BearPlayer::playingChanged, this, [this]() {
        QVariantMap changed;
        changed.insert(QStringLiteral("PlaybackStatus"), playbackStatus());
        emitPlayerPropertiesChanged({QStringLiteral("PlaybackStatus")}, changed);
    });

    connect(p, &BearPlayer::volumeChanged, this, [this]() {
        QVariantMap changed;
        changed.insert(QStringLiteral("Volume"), volume());
        emitPlayerPropertiesChanged({QStringLiteral("Volume")}, changed);
    });

    connect(p, &BearPlayer::currentStationChanged, this, [this]() {
        QVariantMap changed;
        changed.insert(QStringLiteral("Metadata"), metadata());
        changed.insert(QStringLiteral("PlaybackStatus"), playbackStatus());
        changed.insert(QStringLiteral("CanGoNext"), canGoNext());
        changed.insert(QStringLiteral("CanGoPrevious"), canGoPrevious());
        emitPlayerPropertiesChanged({QStringLiteral("Metadata"), QStringLiteral("PlaybackStatus")}, changed);
    });

    connect(p, &BearPlayer::trackInfoChanged, this, [this]() {
        QVariantMap changed;
        changed.insert(QStringLiteral("Metadata"), metadata());
        emitPlayerPropertiesChanged({QStringLiteral("Metadata")}, changed);
    });
}

BearPlayer *MprisPlayerAdaptor::player() const
{
    return m_backend ? m_backend->player() : nullptr;
}

QString MprisPlayerAdaptor::playbackStatus() const
{
    BearPlayer *p = player();
    if (!p) {
        return QStringLiteral("Stopped");
    }
    if (p->currentStationName().isEmpty()) {
        return QStringLiteral("Stopped");
    }
    return p->playing() ? QStringLiteral("Playing") : QStringLiteral("Paused");
}

double MprisPlayerAdaptor::rate() const
{
    return 1.0;
}

void MprisPlayerAdaptor::setRate(double value)
{
    Q_UNUSED(value)
}

double MprisPlayerAdaptor::minimumRate() const
{
    return 1.0;
}

double MprisPlayerAdaptor::maximumRate() const
{
    return 1.0;
}

double MprisPlayerAdaptor::volume() const
{
    BearPlayer *p = player();
    if (!p) {
        return 0.0;
    }
    return p->volume();
}

void MprisPlayerAdaptor::setVolume(double value)
{
    BearPlayer *p = player();
    if (!p) {
        return;
    }
    p->setVolume(value);
}

QVariantMap MprisPlayerAdaptor::metadata() const
{
    QVariantMap map;
    BearPlayer *p = player();
    if (!p) {
        return map;
    }

    const QString station = p->currentStationName();
    const QString title = p->currentTrackTitle();
    const QString artist = p->currentTrackArtist();
    const QString coverUrl = p->currentCoverArtUrl();

    map.insert(QStringLiteral("mpris:trackid"), QVariant::fromValue(QDBusObjectPath(QStringLiteral("/org/kde/bearwave/track/0"))));
    if (!station.isEmpty()) {
        map.insert(QStringLiteral("xesam:album"), station);
    }
    if (!title.isEmpty()) {
        map.insert(QStringLiteral("xesam:title"), title);
    } else if (!station.isEmpty()) {
        map.insert(QStringLiteral("xesam:title"), station);
    }
    if (!artist.isEmpty()) {
        map.insert(QStringLiteral("xesam:artist"), QStringList{artist});
    }
    QString artUrl = coverUrl;
    if (artUrl.isEmpty() && m_backend) {
        QObject *stationObj = m_backend->currentStation();
        if (stationObj) {
            QString fav = stationObj->property("favicon").toString();
            if (fav.startsWith(QLatin1String("https://"))) {
                artUrl = fav;
            }
        }
    }

    if (!artUrl.isEmpty()) {
        map.insert(QStringLiteral("mpris:artUrl"), artUrl);
    } else {
        QString iconPath = QStandardPaths::locate(QStandardPaths::GenericDataLocation, QStringLiteral("icons/hicolor/256x256/apps/org.kde.bearwave.png"));
        if (!iconPath.isEmpty()) {
            map.insert(QStringLiteral("mpris:artUrl"), QStringLiteral("file://") + iconPath);
        }
    }
    map.insert(QStringLiteral("mpris:length"), static_cast<qlonglong>(0));

    return map;
}

qlonglong MprisPlayerAdaptor::position() const
{
    return 0;
}

bool MprisPlayerAdaptor::canSeek() const
{
    return false;
}

bool MprisPlayerAdaptor::canControl() const
{
    return true;
}

bool MprisPlayerAdaptor::canPlay() const
{
    return true;
}

bool MprisPlayerAdaptor::canPause() const
{
    return true;
}

bool MprisPlayerAdaptor::canGoNext() const
{
    return m_backend && m_backend->hasNextStation();
}

bool MprisPlayerAdaptor::canGoPrevious() const
{
    return m_backend && m_backend->hasPreviousStation();
}

void MprisPlayerAdaptor::Play()
{
    BearPlayer *p = player();
    if (!p) {
        return;
    }
    if (!p->playing()) {
        p->togglePlayPause();
    }
}

void MprisPlayerAdaptor::Pause()
{
    BearPlayer *p = player();
    if (!p) {
        return;
    }
    if (p->playing()) {
        p->togglePlayPause();
    }
}

void MprisPlayerAdaptor::PlayPause()
{
    BearPlayer *p = player();
    if (!p) {
        return;
    }
    p->togglePlayPause();
}

void MprisPlayerAdaptor::Stop()
{
    BearPlayer *p = player();
    if (!p) {
        return;
    }
    p->stop();
}

void MprisPlayerAdaptor::Next()
{
    if (m_backend) {
        m_backend->playNextStation();
    }
}

void MprisPlayerAdaptor::Previous()
{
    if (m_backend) {
        m_backend->playPreviousStation();
    }
}
