// Copyright (c) 2026 Sebastian Palencsar
// SPDX-License-Identifier: GPL-3.0-or-later

#include <QtTest>
#include <QApplication>
#include <QDir>
#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QTemporaryDir>
#include <QCryptographicHash>
#include <QEventLoop>
#include <QTimer>

#include "radiobackend.h"
#include "bearplayer.h"
#include "nowplayingstate.h"

class RadioBackendPlaybackTest : public QObject
{
    Q_OBJECT

private slots:
    void initTestCase();
    void init();
    void history_next_navigates_older_entries();
    void resume_disables_list_navigation();
    void station_list_next_uses_filtered_index();
    void history_next_does_not_jump_to_station_list();
    void cached_load_keeps_error_clear_after_network_failure();
    void list_reload_resyncs_index_for_playing_station();
    void list_next_disabled_when_playing_station_not_in_new_list();
    void manual_station_rejects_unsafe_url_scheme();
    void manual_station_accepts_http_and_https();
    void playback_rejects_unsafe_url_from_station_list();
    void favorites_and_state_skip_unsafe_urls();
    void selected_station_tracks_visible_list_selection();
    void play_selected_station_uses_selection_without_prior_playback();
    void source_change_clears_cover_and_rejects_stale_results();
    void player_exposes_connection_and_retry_states();
};

namespace {
QTemporaryDir *s_tempHome = nullptr;

QString cachePathForEndpoint(const QString &endpoint)
{
    const QString cacheDir = QDir::homePath() + QStringLiteral("/.cache/bearwave/api_cache");
    const QString hash = QString(QCryptographicHash::hash(endpoint.toUtf8(), QCryptographicHash::Md5).toHex());
    return cacheDir + QStringLiteral("/") + hash + QStringLiteral(".json");
}

void writeStationCache(const QString &endpoint, const QString &name,
                       const QString &url = QString(), const QString &uuid = QString())
{
    QDir().mkpath(QDir::homePath() + QStringLiteral("/.cache/bearwave/api_cache"));

    QJsonObject station;
    station.insert(QStringLiteral("name"), name);
    station.insert(QStringLiteral("url_resolved"),
                   url.isEmpty() ? QStringLiteral("http://example.com/") + name : url);
    station.insert(QStringLiteral("country"), QStringLiteral("Germany"));
    station.insert(QStringLiteral("stationuuid"), uuid.isEmpty() ? name : uuid);

    QJsonArray array;
    array.append(station);

    QFile file(cachePathForEndpoint(endpoint));
    if (!file.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
        return;
    }
    file.write(QJsonDocument(array).toJson());
    file.close();
}

void writeStationListCache(const QString &endpoint, const QJsonArray &stations)
{
    QDir().mkpath(QDir::homePath() + QStringLiteral("/.cache/bearwave/api_cache"));
    QFile file(cachePathForEndpoint(endpoint));
    if (!file.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
        return;
    }
    file.write(QJsonDocument(stations).toJson());
    file.close();
}

int visibleIndexForUrl(RadioBackend &backend, const QString &url)
{
    const QList<QObject*> stations = backend.stations();
    for (int i = 0; i < stations.size(); ++i) {
        QObject *station = stations[i];
        const QString resolved = station->property("urlResolved").toString();
        if (resolved == url) {
            return i;
        }
    }
    return -1;
}
}

void RadioBackendPlaybackTest::initTestCase()
{
    s_tempHome = new QTemporaryDir();
    QVERIFY(s_tempHome->isValid());
    qputenv("HOME", s_tempHome->path().toUtf8());

    const QString configDir = QDir::homePath() + QStringLiteral("/.config/bearwave");
    QDir().mkpath(configDir);

    QJsonObject state;
    state.insert(QStringLiteral("lastStationName"), QStringLiteral("Station A"));
    state.insert(QStringLiteral("lastStationUrl"), QStringLiteral("http://example.com/a"));
    state.insert(QStringLiteral("volume"), 0.5);

    QJsonArray recent;
    recent.append(QJsonObject{
        {QStringLiteral("name"), QStringLiteral("Station A")},
        {QStringLiteral("uuid"), QStringLiteral("uuid-a")},
        {QStringLiteral("urlResolved"), QStringLiteral("http://example.com/a")}
    });
    recent.append(QJsonObject{
        {QStringLiteral("name"), QStringLiteral("Station B")},
        {QStringLiteral("uuid"), QStringLiteral("uuid-b")},
        {QStringLiteral("urlResolved"), QStringLiteral("http://example.com/b")}
    });
    recent.append(QJsonObject{
        {QStringLiteral("name"), QStringLiteral("Station C")},
        {QStringLiteral("uuid"), QStringLiteral("uuid-c")},
        {QStringLiteral("urlResolved"), QStringLiteral("http://example.com/c")}
    });
    state.insert(QStringLiteral("recentStations"), recent);

    QFile file(configDir + QStringLiteral("/state.json"));
    QVERIFY(file.open(QIODevice::WriteOnly | QIODevice::Truncate));
    file.write(QJsonDocument(state).toJson());
    file.close();
}

void RadioBackendPlaybackTest::init()
{
    initTestCase();
}

void RadioBackendPlaybackTest::history_next_navigates_older_entries()
{
    RadioBackend backend;

    QVERIFY(backend.playRecentByUuid(QStringLiteral("uuid-b"), QString()));
    QCOMPARE(backend.currentStationUrl(), QStringLiteral("http://example.com/b"));
    QVERIFY(backend.hasNextStation());
    QVERIFY(!backend.hasPreviousStation());

    backend.playNextStation();
    QCOMPARE(backend.currentStationUrl(), QStringLiteral("http://example.com/a"));
    QVERIFY(backend.hasPreviousStation());
}

void RadioBackendPlaybackTest::resume_disables_list_navigation()
{
    RadioBackend backend;

    backend.resumeLastStation();
    QCOMPARE(backend.currentStationUrl(), QStringLiteral("http://example.com/a"));
    QVERIFY(!backend.hasNextStation());
    QVERIFY(!backend.hasPreviousStation());

    backend.playNextStation();
    QCOMPARE(backend.currentStationUrl(), QStringLiteral("http://example.com/a"));
}

void RadioBackendPlaybackTest::station_list_next_uses_filtered_index()
{
    RadioBackend backend;

    backend.addManualStation(QStringLiteral("Station One"), QStringLiteral("http://example.com/one"), QStringLiteral("DE"));
    backend.addManualStation(QStringLiteral("Station Two"), QStringLiteral("http://example.com/two"), QStringLiteral("DE"));

    backend.playStation(0);
    QCOMPARE(backend.currentStationUrl(), QStringLiteral("http://example.com/two"));
    QVERIFY(backend.hasNextStation());

    backend.playNextStation();
    QCOMPARE(backend.currentStationUrl(), QStringLiteral("http://example.com/one"));
}

void RadioBackendPlaybackTest::history_next_does_not_jump_to_station_list()
{
    RadioBackend backend;

    // Loaded station list present, but playback started from history (not playStation()).
    backend.addManualStation(QStringLiteral("Station X"), QStringLiteral("http://example.com/x"), QStringLiteral("DE"));
    backend.addManualStation(QStringLiteral("Station Y"), QStringLiteral("http://example.com/y"), QStringLiteral("DE"));

    QVERIFY(backend.playRecentByUuid(QStringLiteral("uuid-c"), QString()));
    QCOMPARE(backend.currentStationUrl(), QStringLiteral("http://example.com/c"));

    backend.playNextStation();
    QCOMPARE(backend.currentStationUrl(), QStringLiteral("http://example.com/a"));
    QVERIFY(backend.currentStationUrl() != QStringLiteral("http://example.com/x"));
    QVERIFY(backend.currentStationUrl() != QStringLiteral("http://example.com/y"));
}

void RadioBackendPlaybackTest::cached_load_keeps_error_clear_after_network_failure()
{
    writeStationCache(QStringLiteral("/stations/topvote/100"), QStringLiteral("Cached Top Station"));

    RadioBackend backend;
    QVERIFY(QFile::exists(QDir::homePath() + QStringLiteral("/.cache/bearwave/api_cache")));
    backend.loadTopStations();

    QTRY_VERIFY_WITH_TIMEOUT(backend.stations().size() > 0, 2000);
    QCOMPARE(backend.lastError(), QString());

    QEventLoop loop;
    QTimer::singleShot(11000, &loop, &QEventLoop::quit);
    loop.exec();

    QCOMPARE(backend.lastError(), QString());
    QVERIFY(backend.stations().size() > 0);
}

void RadioBackendPlaybackTest::list_reload_resyncs_index_for_playing_station()
{
    const QString topEndpoint = QStringLiteral("/stations/topvote/100");
    const QString deEndpoint = QStringLiteral("/stations/bycountrycodeexact/DE?limit=50&order=votes&reverse=true");
    const QString sharedUrl = QStringLiteral("http://example.com/shared");

    QJsonArray topStations;
    topStations.append(QJsonObject{
        {QStringLiteral("name"), QStringLiteral("Alpha Top")},
        {QStringLiteral("url_resolved"), QStringLiteral("http://example.com/alpha")},
        {QStringLiteral("country"), QStringLiteral("Global")},
        {QStringLiteral("stationuuid"), QStringLiteral("uuid-alpha")}
    });
    topStations.append(QJsonObject{
        {QStringLiteral("name"), QStringLiteral("Shared Station")},
        {QStringLiteral("url_resolved"), sharedUrl},
        {QStringLiteral("country"), QStringLiteral("Global")},
        {QStringLiteral("stationuuid"), QStringLiteral("uuid-shared")}
    });
    topStations.append(QJsonObject{
        {QStringLiteral("name"), QStringLiteral("Zulu Top")},
        {QStringLiteral("url_resolved"), QStringLiteral("http://example.com/zulu")},
        {QStringLiteral("country"), QStringLiteral("Global")},
        {QStringLiteral("stationuuid"), QStringLiteral("uuid-zulu")}
    });
    writeStationListCache(topEndpoint, topStations);

    QJsonArray deStations;
    deStations.append(QJsonObject{
        {QStringLiteral("name"), QStringLiteral("DE Only")},
        {QStringLiteral("url_resolved"), QStringLiteral("http://example.com/deonly")},
        {QStringLiteral("country"), QStringLiteral("Germany")},
        {QStringLiteral("stationuuid"), QStringLiteral("uuid-deonly")}
    });
    deStations.append(QJsonObject{
        {QStringLiteral("name"), QStringLiteral("Shared Station")},
        {QStringLiteral("url_resolved"), sharedUrl},
        {QStringLiteral("country"), QStringLiteral("Germany")},
        {QStringLiteral("stationuuid"), QStringLiteral("uuid-shared")}
    });
    writeStationListCache(deEndpoint, deStations);

    RadioBackend backend;
    backend.loadTopStations();
    QTRY_VERIFY_WITH_TIMEOUT(backend.stations().size() == 3, 2000);

    const int sharedTopIndex = visibleIndexForUrl(backend, sharedUrl);
    QVERIFY(sharedTopIndex >= 0);
    backend.playStation(sharedTopIndex);
    QCOMPARE(backend.currentStationUrl(), sharedUrl);
    QVERIFY(backend.hasNextStation());

    backend.loadGermanStations();
    QTRY_VERIFY_WITH_TIMEOUT(backend.stations().size() == 2, 2000);
    QCOMPARE(backend.currentStationUrl(), sharedUrl);
    QVERIFY(!backend.hasNextStation());
    QVERIFY(backend.hasPreviousStation());

    backend.playNextStation();
    QCOMPARE(backend.currentStationUrl(), sharedUrl);

    backend.playPreviousStation();
    QCOMPARE(backend.currentStationUrl(), QStringLiteral("http://example.com/deonly"));
}

void RadioBackendPlaybackTest::list_next_disabled_when_playing_station_not_in_new_list()
{
    const QString topEndpoint = QStringLiteral("/stations/topvote/100");
    const QString deEndpoint = QStringLiteral("/stations/bycountrycodeexact/DE?limit=50&order=votes&reverse=true");
    const QString topUrl = QStringLiteral("http://example.com/toponly");

    writeStationCache(topEndpoint, QStringLiteral("Top Only"), topUrl, QStringLiteral("uuid-top"));
    writeStationCache(deEndpoint, QStringLiteral("DE Only"), QStringLiteral("http://example.com/deonly"),
                      QStringLiteral("uuid-de"));

    RadioBackend backend;
    backend.loadTopStations();
    QTRY_VERIFY_WITH_TIMEOUT(backend.stations().size() == 1, 2000);
    backend.playStation(0);
    QCOMPARE(backend.currentStationUrl(), topUrl);
    QVERIFY(!backend.hasNextStation());

    backend.loadGermanStations();
    QTRY_VERIFY_WITH_TIMEOUT(backend.stations().size() == 1, 2000);
    QCOMPARE(backend.currentStationUrl(), topUrl);
    QVERIFY(!backend.hasNextStation());
    QVERIFY(!backend.hasPreviousStation());

    backend.playNextStation();
    QCOMPARE(backend.currentStationUrl(), topUrl);
}

void RadioBackendPlaybackTest::manual_station_rejects_unsafe_url_scheme()
{
    RadioBackend backend;

    backend.addManualStation(QStringLiteral("Unsafe"), QStringLiteral("file:///etc/passwd"), QStringLiteral("DE"));
    QCOMPARE(backend.stations().size(), 0);
    QVERIFY(!backend.lastError().isEmpty());

    backend.addManualStation(QStringLiteral("FTP"), QStringLiteral("ftp://example.com/stream"), QStringLiteral("DE"));
    QCOMPARE(backend.stations().size(), 0);
}

void RadioBackendPlaybackTest::manual_station_accepts_http_and_https()
{
    RadioBackend backend;

    backend.addManualStation(QStringLiteral("HTTP"), QStringLiteral("http://example.com/stream"), QStringLiteral("DE"));
    QCOMPARE(backend.stations().size(), 1);
    QCOMPARE(backend.lastError(), QString());

    backend.addManualStation(QStringLiteral("HTTPS"), QStringLiteral("https://example.com/stream"), QStringLiteral("DE"));
    QCOMPARE(backend.stations().size(), 2);
}

void RadioBackendPlaybackTest::playback_rejects_unsafe_url_from_station_list()
{
    const QString topEndpoint = QStringLiteral("/stations/topvote/100");
    writeStationCache(topEndpoint, QStringLiteral("Bad File"),
                      QStringLiteral("file:///tmp/x"),
                      QStringLiteral("uuid-file"));

    RadioBackend backend;
    backend.loadTopStations();
    QTRY_VERIFY_WITH_TIMEOUT(backend.stations().size() == 1, 2000);

    backend.playStation(0);
    QVERIFY(!backend.lastError().isEmpty());
    QCOMPARE(backend.currentStationUrl(), QString());
    QVERIFY(!backend.player()->playing());
}

void RadioBackendPlaybackTest::favorites_and_state_skip_unsafe_urls()
{
    const QString configDir =
        QDir::homePath() + QStringLiteral("/.config/bearwave");
    QDir().mkpath(configDir);

    QJsonArray favorites;
    favorites.append(QJsonObject{
        {QStringLiteral("uuid"), QStringLiteral("safe")},
        {QStringLiteral("name"), QStringLiteral("Safe")},
        {QStringLiteral("url"), QStringLiteral("https://example.com/safe")},
        {QStringLiteral("urlResolved"), QStringLiteral("https://example.com/safe")},
        {QStringLiteral("country"), QStringLiteral("DE")}
    });
    favorites.append(QJsonObject{
        {QStringLiteral("uuid"), QStringLiteral("bad")},
        {QStringLiteral("name"), QStringLiteral("Bad")},
        {QStringLiteral("url"), QStringLiteral("file:///etc/passwd")},
        {QStringLiteral("urlResolved"), QStringLiteral("file:///etc/passwd")},
        {QStringLiteral("country"), QStringLiteral("DE")}
    });
    QFile favFile(configDir + QStringLiteral("/favorites.json"));
    QVERIFY(favFile.open(QIODevice::WriteOnly | QIODevice::Truncate));
    favFile.write(QJsonDocument(favorites).toJson());
    favFile.close();

    QJsonObject state;
    state.insert(QStringLiteral("lastStationName"), QStringLiteral("Evil"));
    state.insert(QStringLiteral("lastStationUrl"),
                 QStringLiteral("file:///tmp/evil"));
    state.insert(QStringLiteral("volume"), 0.4);
    QJsonArray recent;
    recent.append(QJsonObject{
        {QStringLiteral("name"), QStringLiteral("Evil Recent")},
        {QStringLiteral("uuid"), QStringLiteral("evil-r")},
        {QStringLiteral("urlResolved"), QStringLiteral("ftp://evil.example/stream")}
    });
    recent.append(QJsonObject{
        {QStringLiteral("name"), QStringLiteral("Good Recent")},
        {QStringLiteral("uuid"), QStringLiteral("good-r")},
        {QStringLiteral("urlResolved"), QStringLiteral("https://example.com/recent")}
    });
    state.insert(QStringLiteral("recentStations"), recent);
    QFile stateFile(configDir + QStringLiteral("/state.json"));
    QVERIFY(stateFile.open(QIODevice::WriteOnly | QIODevice::Truncate));
    stateFile.write(QJsonDocument(state).toJson());
    stateFile.close();

    RadioBackend backend;
    QCOMPARE(backend.favoriteStations().size(), 1);
    QCOMPARE(backend.favoriteStations().first()->property("name").toString(),
             QStringLiteral("Safe"));
    QVERIFY(!backend.canResumeLastStation());
    QCOMPARE(backend.recentStations().size(), 1);
    QCOMPARE(backend.recentStations().first()
                 .toMap()
                 .value(QStringLiteral("uuid"))
                 .toString(),
             QStringLiteral("good-r"));
}

void RadioBackendPlaybackTest::selected_station_tracks_visible_list_selection()
{
    RadioBackend backend;

    backend.addManualStation(QStringLiteral("Station One"), QStringLiteral("http://example.com/one"), QStringLiteral("DE"));
    backend.addManualStation(QStringLiteral("Station Two"), QStringLiteral("http://example.com/two"), QStringLiteral("DE"));

    QCOMPARE(backend.selectedStation().value(QStringLiteral("name")).toString(), QStringLiteral("Station Two"));

    backend.selectStation(1);
    QCOMPARE(backend.selectedStation().value(QStringLiteral("name")).toString(), QStringLiteral("Station One"));
    QCOMPARE(backend.currentStationUrl(), QString());
}

void RadioBackendPlaybackTest::play_selected_station_uses_selection_without_prior_playback()
{
    RadioBackend backend;

    backend.addManualStation(QStringLiteral("Station One"), QStringLiteral("http://example.com/one"), QStringLiteral("DE"));
    backend.addManualStation(QStringLiteral("Station Two"), QStringLiteral("http://example.com/two"), QStringLiteral("DE"));
    backend.selectStation(1);

    QVERIFY(backend.playSelectedStation());
    QCOMPARE(backend.currentStationUrl(), QStringLiteral("http://example.com/one"));
}

void RadioBackendPlaybackTest::source_change_clears_cover_and_rejects_stale_results()
{
    NowPlayingState state;
    const quint64 firstGeneration = state.resetSource();

    QVERIFY(state.updateCover(firstGeneration, QStringLiteral("https://example.com/old-cover.jpg")));
    QCOMPARE(state.coverUrl(), QStringLiteral("https://example.com/old-cover.jpg"));
    QCOMPARE(state.title(), QString());

    const quint64 secondGeneration = state.resetSource();
    QCOMPARE(state.coverUrl(), QString());
    QCOMPARE(state.artist(), QString());
    QCOMPARE(state.title(), QString());

    QVERIFY(!state.updateCover(firstGeneration, QStringLiteral("https://example.com/stale-cover.jpg")));
    QVERIFY(!state.updateMetadata(firstGeneration, QStringLiteral("Old Artist"), QStringLiteral("Old Title")));
    QCOMPARE(state.coverUrl(), QString());
    QCOMPARE(state.artist(), QString());
    QCOMPARE(state.title(), QString());

    QVERIFY(state.updateMetadata(secondGeneration, QStringLiteral("New Artist"), QStringLiteral("New Title")));
    QCOMPARE(state.artist(), QStringLiteral("New Artist"));
    QCOMPARE(state.title(), QStringLiteral("New Title"));
}

void RadioBackendPlaybackTest::player_exposes_connection_and_retry_states()
{
    BearPlayer player;
    QCOMPARE(player.connectionState(), QStringLiteral("idle"));

    player.playUrl(QStringLiteral("http://127.0.0.1:9/stream"),
                   QStringLiteral("Test Station"));
    QCOMPARE(player.connectionState(), QStringLiteral("connecting"));

    QVERIFY(QMetaObject::invokeMethod(
        &player, "onMediaStatusChanged", Qt::DirectConnection,
        Q_ARG(QMediaPlayer::MediaStatus, QMediaPlayer::BufferingMedia)));
    QCOMPARE(player.connectionState(), QStringLiteral("buffering"));

    QVERIFY(QMetaObject::invokeMethod(
        &player, "onPlaybackStateChanged", Qt::DirectConnection,
        Q_ARG(QMediaPlayer::PlaybackState, QMediaPlayer::PlayingState)));
    QCOMPARE(player.connectionState(), QStringLiteral("playing"));

    QVERIFY(QMetaObject::invokeMethod(
        &player, "onPlaybackStateChanged", Qt::DirectConnection,
        Q_ARG(QMediaPlayer::PlaybackState, QMediaPlayer::PausedState)));
    QCOMPARE(player.connectionState(), QStringLiteral("paused"));

    QVERIFY(QMetaObject::invokeMethod(
        &player, "onMediaStatusChanged", Qt::DirectConnection,
        Q_ARG(QMediaPlayer::MediaStatus, QMediaPlayer::InvalidMedia)));
    QCOMPARE(player.connectionState(), QStringLiteral("retrying"));

    player.stop();
    QCOMPARE(player.connectionState(), QStringLiteral("idle"));
    player.togglePlayPause();
    QCOMPARE(player.connectionState(), QStringLiteral("idle"));
}

int main(int argc, char *argv[])
{
    qputenv("QT_QPA_PLATFORM", "offscreen");
    qputenv("QT_MEDIA_BACKEND", "ffmpeg");

    QApplication app(argc, argv);
    RadioBackendPlaybackTest test;
    return QTest::qExec(&test, argc, argv);
}

#include "radiobackend_playback_test.moc"
