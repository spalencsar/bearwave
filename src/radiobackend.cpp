// Copyright (c) 2026 Sebastian Palencsar
// SPDX-License-Identifier: GPL-3.0-or-later

#include "radiobackend.h"
#include "countrynames.h"
#include "stationartworkservice.h"
#include "stationimagecache.h"
#include "stationlogostyle.h"
#include "streamurl.h"

#include <QDebug>
#include <algorithm>
#include <QDir>
#include <QFile>
#include <QGuiApplication>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QClipboard>
#include <QStandardPaths>
#include <QLocale>
#include <QUrl>

namespace {
constexpr int kRecentLimit = 20;

QString appConfigDir()
{
    return QDir::homePath() + QStringLiteral("/.config/bearwave");
}

}

RadioBackend::RadioBackend(QObject *parent)
    : QObject(parent)
{
    m_radioBrowser = new RadioBrowser(this);
    m_stationImageCache = new StationImageCache(this);
    m_stationArtworkService =
        new StationArtworkService(m_stationImageCache, this);
    m_player = new BearPlayer(this);
    setupConnections();
    loadFavorites();
    loadState();
}

int RadioBackend::stationImageRevision() const
{
    return (m_stationImageCache ? m_stationImageCache->revision() : 0)
           + (m_stationArtworkService ? m_stationArtworkService->revision() : 0);
}

QString RadioBackend::stationImageSource(const QString &remoteUrl)
{
    return m_stationImageCache
               ? m_stationImageCache->sourceForUrl(remoteUrl)
               : QStringLiteral("qrc:/assets/app/bearwave.svg");
}

QString RadioBackend::stationLogoSource(const QString &stationKey,
                                        const QString &faviconUrl,
                                        const QString &homepageUrl)
{
    return m_stationArtworkService
               ? m_stationArtworkService->sourceForStation(
                     stationKey, faviconUrl, homepageUrl)
               : QString();
}

QString RadioBackend::stationInitials(const QString &stationName) const
{
    return StationLogoStyle::initials(stationName);
}

int RadioBackend::stationLogoPaletteIndex(const QString &stationKey,
                                          int paletteSize) const
{
    return StationLogoStyle::paletteIndex(stationKey, paletteSize);
}

QList<QObject*> RadioBackend::stations() const
{
    QList<QObject*> list;
    for (RadioStation *s : m_filteredStations) {
        list.append(s);
    }
    return list;
}

QList<QObject*> RadioBackend::favoriteStations() const
{
    QList<QObject*> list;
    for (RadioStation *s : m_favorites) {
        list.append(s);
    }
    return list;
}

void RadioBackend::setupConnections()
{
    connect(m_radioBrowser, &RadioBrowser::stationsLoaded,
            this, &RadioBackend::onStationsLoaded);
    connect(m_radioBrowser, &RadioBrowser::countriesLoaded,
            this, &RadioBackend::onCountriesLoaded);
    connect(m_radioBrowser, &RadioBrowser::error,
            this, [this](const QString &error) {
        qWarning() << "RadioBrowser error:" << error;
        setLoading(false);
        if (!m_currentLoadSatisfied) {
            setLastError(error);
        } else {
            setLastError(QString());
        }
    });
    connect(m_stationImageCache, &StationImageCache::revisionChanged,
            this, &RadioBackend::stationImageRevisionChanged);
    connect(m_stationArtworkService, &StationArtworkService::revisionChanged,
            this, &RadioBackend::stationImageRevisionChanged);

    connect(m_player, &BearPlayer::currentStationChanged, this, [this](const QString &name) {
        if (!name.isEmpty()) {
            m_lastStationName = name;
            emit resumeStateChanged();
            saveState();
        } else {
            m_currentStationUuid.clear();
            m_currentStationUrl.clear();
            emit currentStationChanged();
        }
    });

    connect(m_player, &BearPlayer::volumeChanged, this, [this]() {
        saveState();
    });
}

void RadioBackend::onStationsLoaded(const QList<RadioStation*> &stations)
{
    qDeleteAll(m_stations);
    m_stations.clear();
    m_filteredStations.clear();

    m_stations = stations;
    for (RadioStation *s : m_stations) {
        s->setParent(this);
        for (RadioStation *f : m_favorites) {
            if ((!s->uuid().isEmpty() && s->uuid() == f->uuid()) ||
                (s->uuid().isEmpty() && s->urlResolved() == f->urlResolved())) {
                s->setIsFavorite(true);
                break;
            }
        }
    }
    std::sort(m_stations.begin(), m_stations.end(), [](RadioStation *a, RadioStation *b) {
        return QString::localeAwareCompare(a->name(), b->name()) < 0;
    });

    rebuildFilteredStations(false);
    if (!m_filteredStations.isEmpty()) {
        setSelectedStation(toVariantMap(m_filteredStations.first()));
    } else {
        setSelectedStation(QVariantMap());
    }
    syncStationListIndex();
    m_currentLoadSatisfied = true;
    setLastError(QString());
    setLoading(false);
    qDebug() << "Loaded" << stations.size() << "stations";
}

void RadioBackend::beginLoad()
{
    m_currentLoadSatisfied = false;
    setLoading(true);
}

void RadioBackend::loadGermanStations()
{
    beginLoad();
    m_radioBrowser->getGermanStations();
}

void RadioBackend::loadDutchStations()
{
    beginLoad();
    m_radioBrowser->getDutchStations();
}

void RadioBackend::loadTopStations()
{
    beginLoad();
    m_radioBrowser->getTopStations(100);
}

void RadioBackend::loadWorldStations()
{
    beginLoad();
    m_radioBrowser->getWorldStations(200);
}

void RadioBackend::loadCountries()
{
    beginLoad();
    m_radioBrowser->getCountries();
}

bool RadioBackend::countryMatches(const QVariantMap &country, const QString &query) const
{
    return CountryNames::matches(country.value(QStringLiteral("code")).toString(),
                                 country.value(QStringLiteral("englishName")).toString(),
                                 country.value(QStringLiteral("name")).toString(),
                                 query);
}

void RadioBackend::onCountriesLoaded(const QVariantList &countries)
{
    m_countries.clear();
    for (const QVariant &item : countries) {
        QVariantMap map = item.toMap();
        QString code = map["code"].toString();
        QString englishName = map["name"].toString();
        map["englishName"] = englishName;
        map["name"] = CountryNames::displayName(code, englishName);
        m_countries.append(map);
    }

    std::sort(m_countries.begin(), m_countries.end(), [](const QVariant &a, const QVariant &b) {
        return QString::localeAwareCompare(a.toMap()["name"].toString(), b.toMap()["name"].toString()) < 0;
    });
    emit countriesChanged();
    m_currentLoadSatisfied = true;
    setLastError(QString());
    setLoading(false);
}

void RadioBackend::loadByTag(const QString &tag)
{
    if (tag.trimmed().isEmpty()) {
        return;
    }
    beginLoad();
    m_radioBrowser->getByTag(tag.trimmed());
}

void RadioBackend::loadByCountryCode(const QString &countryCode)
{
    if (countryCode.trimmed().isEmpty()) {
        return;
    }
    beginLoad();
    m_radioBrowser->getByCountry(countryCode.trimmed().toUpper());
}

void RadioBackend::searchStations(const QString &query)
{
    if (query.isEmpty()) {
        return;
    }
    qDebug() << "RadioBackend: searchStations called with query:" << query;
    beginLoad();
    m_radioBrowser->search(query);
}

void RadioBackend::playStation(int index)
{
    if (!stationForVisibleIndex(index)) {
        return;
    }

    m_currentFromFavorites = false;
    m_currentFromHistory = false;
    m_standalonePlayback = false;
    m_currentIndex = index;

    playCurrentSelection();
}

void RadioBackend::playFavoriteStation(int index)
{
    if (index < 0 || index >= m_favorites.size()) {
        return;
    }

    m_currentFromFavorites = true;
    m_currentFromHistory = false;
    m_standalonePlayback = false;
    m_currentIndex = index;

    playCurrentSelection();
}

void RadioBackend::selectStation(int index)
{
    RadioStation *station = stationForVisibleIndex(index);
    if (!station) {
        return;
    }
    setSelectedStation(toVariantMap(station));
}

void RadioBackend::selectFavoriteStation(int index)
{
    if (index < 0 || index >= m_favorites.size()) {
        return;
    }
    setSelectedStation(toVariantMap(m_favorites[index]));
}

bool RadioBackend::selectRecentByUuid(const QString &uuid, const QString &urlResolved)
{
    const QVariantList recent = recentStations();
    const int recentIndex = recentStationIndex(recent, uuid, urlResolved);
    if (recentIndex < 0) {
        return false;
    }
    setSelectedStation(recent.at(recentIndex).toMap());
    return true;
}

bool RadioBackend::playSelectedStation()
{
    const QString uuid = m_selectedStation.value(QStringLiteral("uuid")).toString();
    const QString urlResolved = m_selectedStation.value(QStringLiteral("urlResolved")).toString();
    if (uuid.isEmpty() && urlResolved.isEmpty()) {
        return false;
    }

    for (int i = 0; i < m_filteredStations.size(); ++i) {
        if (matchesStation(m_filteredStations[i], uuid, urlResolved)) {
            playStation(i);
            return true;
        }
    }

    for (int i = 0; i < m_favorites.size(); ++i) {
        if (matchesStation(m_favorites[i], uuid, urlResolved)) {
            playFavoriteStation(i);
            return true;
        }
    }

    return playRecentByUuid(uuid, urlResolved);
}

void RadioBackend::playNextStation()
{
    if (m_standalonePlayback) {
        return;
    }

    if (m_currentFromHistory) {
        if (m_recentStations.isEmpty()) {
            return;
        }
        const int nextIndex = m_currentIndex < 0 ? 0 : m_currentIndex + 1;
        if (nextIndex >= m_recentStations.size()) {
            return;
        }
        playHistoryAtIndex(nextIndex, false);
        return;
    }

    const QList<RadioStation*> list = currentList();
    if (list.isEmpty()) {
        return;
    }
    if (m_currentIndex < 0) {
        return;
    }
    if (m_currentIndex < list.size() - 1) {
        ++m_currentIndex;
    }
    playCurrentSelection();
}

void RadioBackend::playPreviousStation()
{
    if (m_standalonePlayback) {
        return;
    }

    if (m_currentFromHistory) {
        if (m_currentIndex <= 0) {
            return;
        }
        playHistoryAtIndex(m_currentIndex - 1, false);
        return;
    }

    const QList<RadioStation*> list = currentList();
    if (list.isEmpty()) {
        return;
    }
    if (m_currentIndex <= 0) {
        return;
    }
    --m_currentIndex;
    playCurrentSelection();
}

bool RadioBackend::hasNextStation() const
{
    if (m_standalonePlayback) {
        return false;
    }

    if (m_currentFromHistory) {
        if (m_recentStations.isEmpty()) {
            return false;
        }
        if (m_currentIndex < 0) {
            return true;
        }
        return m_currentIndex < m_recentStations.size() - 1;
    }

    const QList<RadioStation*> list = currentList();
    if (list.isEmpty()) {
        return false;
    }
    if (m_currentIndex < 0) {
        return false;
    }
    return m_currentIndex < list.size() - 1;
}

bool RadioBackend::hasPreviousStation() const
{
    if (m_standalonePlayback) {
        return false;
    }

    if (m_currentFromHistory) {
        return m_currentIndex > 0;
    }

    const QList<RadioStation*> list = currentList();
    if (list.isEmpty()) {
        return false;
    }
    return m_currentIndex > 0;
}

void RadioBackend::toggleFavorite(int index)
{
    if (index < 0 || index >= m_stations.size()) {
        return;
    }

    RadioStation *station = m_stations[index];
    for (int i = 0; i < m_favorites.size(); ++i) {
        if ((!station->uuid().isEmpty() && m_favorites[i]->uuid() == station->uuid()) ||
            (station->uuid().isEmpty() && m_favorites[i]->urlResolved() == station->urlResolved())) {
            m_favorites.removeAt(i);
            station->setIsFavorite(false);
            setSelectedStation(toVariantMap(station));
            saveFavorites();
            emit favoritesChanged();
            emit stationsChanged();
            emit listsChanged();
            return;
        }
    }

    RadioStation *copy = new RadioStation(this);
    copy->setUuid(station->uuid());
    copy->setName(station->name());
    copy->setUrl(station->url());
    copy->setUrlResolved(station->urlResolved());
    copy->setHomepage(station->homepage());
    copy->setFavicon(station->favicon());
    copy->setCountry(station->country());
    copy->setTags(station->tags());
    copy->setCodec(station->codec());
    copy->setBitrate(station->bitrate());
    copy->setVotes(station->votes());
    copy->setIsOnline(station->isOnline());
    copy->setIsFavorite(true);
    m_favorites.append(copy);
    std::sort(m_favorites.begin(), m_favorites.end(), [](RadioStation *a, RadioStation *b) {
        return QString::localeAwareCompare(a->name(), b->name()) < 0;
    });
    station->setIsFavorite(true);
    setSelectedStation(toVariantMap(station));

    saveFavorites();
    emit favoritesChanged();
    emit stationsChanged();
    emit listsChanged();
}

void RadioBackend::toggleFavoriteById(const QString &uuid, const QString &urlResolved)
{
    int stationIndex = -1;
    for (int i = 0; i < m_stations.size(); ++i) {
        if ((!uuid.isEmpty() && m_stations[i]->uuid() == uuid) ||
            (uuid.isEmpty() && !urlResolved.isEmpty() && m_stations[i]->urlResolved() == urlResolved)) {
            stationIndex = i;
            break;
        }
    }

    if (stationIndex >= 0) {
        toggleFavorite(stationIndex);
        return;
    }

    for (int i = 0; i < m_favorites.size(); ++i) {
        if ((!uuid.isEmpty() && m_favorites[i]->uuid() == uuid) ||
            (uuid.isEmpty() && !urlResolved.isEmpty() && m_favorites[i]->urlResolved() == urlResolved)) {
            m_favorites.removeAt(i);
            for (RadioStation *s : m_stations) {
                if ((!uuid.isEmpty() && s->uuid() == uuid) ||
                    (uuid.isEmpty() && !urlResolved.isEmpty() && s->urlResolved() == urlResolved)) {
                    s->setIsFavorite(false);
                    setSelectedStation(toVariantMap(s));
                }
            }
            if (m_selectedStation.value(QStringLiteral("uuid")).toString() == uuid ||
                (uuid.isEmpty() && m_selectedStation.value(QStringLiteral("urlResolved")).toString() == urlResolved)) {
                QVariantMap station = m_selectedStation;
                station.insert(QStringLiteral("isFavorite"), false);
                setSelectedStation(station);
            }
            saveFavorites();
            emit favoritesChanged();
            emit stationsChanged();
            emit listsChanged();
            return;
        }
    }
}

void RadioBackend::addManualStation(const QString &name, const QString &url, const QString &country)
{
    if (name.trimmed().isEmpty() || url.trimmed().isEmpty()) {
        return;
    }
    if (!isAllowedStreamUrl(url)) {
        setLastError(tr("Stream URL must use http:// or https://"));
        return;
    }

    setLastError(QString());
    RadioStation *station = new RadioStation(this);
    station->setName(name.trimmed());
    station->setUrl(url.trimmed());
    station->setUrlResolved(url.trimmed());
    station->setCountry(country.trimmed().isEmpty() ? tr("Manual") : country.trimmed());
    station->setCodec(QStringLiteral("unknown"));
    station->setBitrate(0);
    station->setVotes(0);
    station->setIsOnline(true);

    m_stations.prepend(station);
    rebuildFilteredStations(false);
    setSelectedStation(toVariantMap(station));
}

void RadioBackend::sortStations(const QString &mode)
{
    if (mode == QStringLiteral("name")) {
        std::sort(m_stations.begin(), m_stations.end(), [](RadioStation *a, RadioStation *b) {
            return QString::localeAwareCompare(a->name(), b->name()) < 0;
        });
    } else if (mode == QStringLiteral("bitrate")) {
        std::sort(m_stations.begin(), m_stations.end(), [](RadioStation *a, RadioStation *b) {
            return a->bitrate() > b->bitrate();
        });
    } else if (mode == QStringLiteral("votes")) {
        std::sort(m_stations.begin(), m_stations.end(), [](RadioStation *a, RadioStation *b) {
            return a->votes() > b->votes();
        });
    }
    rebuildFilteredStations(false);
}

QVariantList RadioBackend::getRecentStations() const
{
    return m_recentStations;
}

QVariantList RadioBackend::recentStations() const
{
    QVariantList updatedList;
    for (const QVariant &entry : m_recentStations) {
        QVariantMap station = entry.toMap();
        const QString uuid = station.value(QStringLiteral("uuid")).toString();
        const QString urlResolved = station.value(QStringLiteral("urlResolved")).toString();

        bool isFav = false;
        for (const RadioStation *fav : m_favorites) {
            if ((!uuid.isEmpty() && fav->uuid() == uuid) ||
                (uuid.isEmpty() && !urlResolved.isEmpty() && fav->urlResolved() == urlResolved)) {
                isFav = true;
                break;
            }
        }
        station.insert(QStringLiteral("isFavorite"), isFav);
        updatedList.append(station);
    }
    return updatedList;
}

QVariantList RadioBackend::getFavoriteStations() const
{
    QVariantList result;
    for (RadioStation *station : m_favorites) {
        result.push_back(toVariantMap(station));
    }
    return result;
}

bool RadioBackend::playRecentByUuid(const QString &uuid, const QString &urlResolved)
{
    const int recentIndex = recentStationIndex(m_recentStations, uuid, urlResolved);
    if (recentIndex >= 0) {
        playHistoryAtIndex(recentIndex, true);
        return true;
    }

    for (RadioStation *station : m_stations) {
        if (matchesStation(station, uuid, urlResolved)) {
            const QVariantMap stationData = toVariantMap(station);
            recordRecentStation(stationData);
            playHistoryAtIndex(0, false);
            return true;
        }
    }

    return false;
}

bool RadioBackend::playFavoriteByUuid(const QString &uuid, const QString &urlResolved)
{
    for (int i = 0; i < m_favorites.size(); ++i) {
        if (matchesStation(m_favorites[i], uuid, urlResolved)) {
            playFavoriteStation(i);
            return true;
        }
    }
    return false;
}

void RadioBackend::resumeLastStation()
{
    if (m_lastStationUrl.isEmpty()) {
        return;
    }

    m_currentFromFavorites = false;
    m_currentFromHistory = false;
    m_standalonePlayback = true;
    m_currentIndex = -1;

    QVariantMap station;
    station.insert(QStringLiteral("name"), m_lastStationName.isEmpty() ? tr("Last played") : m_lastStationName);
    station.insert(QStringLiteral("urlResolved"), m_lastStationUrl);
    recordRecentStation(station);
    saveState();

    QString resolvedUuid;
    for (const RadioStation *s : m_stations) {
        if (s->urlResolved() == m_lastStationUrl || s->url() == m_lastStationUrl) {
            resolvedUuid = s->uuid();
            break;
        }
    }
    if (resolvedUuid.isEmpty()) {
        for (const RadioStation *s : m_favorites) {
            if (s->urlResolved() == m_lastStationUrl || s->url() == m_lastStationUrl) {
                resolvedUuid = s->uuid();
                break;
            }
        }
    }
    m_currentStationUuid = resolvedUuid;
    m_currentStationUrl = m_lastStationUrl;
    emit currentStationChanged();

    if (!startPlayback(m_lastStationUrl,
                       m_lastStationName.isEmpty() ? tr("Last played")
                                                   : m_lastStationName)) {
        m_lastStationUrl.clear();
        m_lastStationName.clear();
        m_currentStationUrl.clear();
        m_currentStationUuid.clear();
        emit resumeStateChanged();
        emit currentStationChanged();
    }
}

void RadioBackend::copyToClipboard(const QString &text) const
{
    if (text.isEmpty() || !QGuiApplication::clipboard()) {
        return;
    }
    QGuiApplication::clipboard()->setText(text);
}

void RadioBackend::loadFavorites()
{
    const QString configDir = appConfigDir();
    QDir().mkpath(configDir);
    QFile file(configDir + "/favorites.json");
    if (!file.open(QIODevice::ReadOnly)) {
        return;
    }

    const QJsonDocument doc = QJsonDocument::fromJson(file.readAll());
    if (!doc.isArray()) {
        return;
    }

    for (const QJsonValue &v : doc.array()) {
        if (!v.isObject()) {
            continue;
        }
        QJsonObject o = v.toObject();
        const QString url = o.value("url").toString();
        const QString urlResolved = o.value("urlResolved").toString();
        const QString streamUrl = !urlResolved.isEmpty() ? urlResolved : url;
        if (!isAllowedStreamUrl(streamUrl)
            || (!url.isEmpty() && !isAllowedStreamUrl(url))
            || (!urlResolved.isEmpty() && !isAllowedStreamUrl(urlResolved))) {
            qWarning() << "Skipping favorite with disallowed stream URL scheme";
            continue;
        }
        RadioStation *s = new RadioStation(this);
        s->setUuid(o.value("uuid").toString());
        s->setName(o.value("name").toString());
        s->setUrl(url);
        s->setUrlResolved(urlResolved);
        s->setCountry(o.value("country").toString());
        s->setIsFavorite(true);
        m_favorites.append(s);
    }
    std::sort(m_favorites.begin(), m_favorites.end(), [](RadioStation *a, RadioStation *b) {
        return QString::localeAwareCompare(a->name(), b->name()) < 0;
    });
}

void RadioBackend::saveFavorites() const
{
    const QString configDir = appConfigDir();
    QDir().mkpath(configDir);
    QFile file(configDir + "/favorites.json");
    if (!file.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
        return;
    }

    QJsonArray arr;
    for (RadioStation *s : m_favorites) {
        QJsonObject o;
        o.insert("uuid", s->uuid());
        o.insert("name", s->name());
        o.insert("url", s->url());
        o.insert("urlResolved", s->urlResolved());
        o.insert("country", s->country());
        arr.append(o);
    }
    file.write(QJsonDocument(arr).toJson(QJsonDocument::Indented));
}

void RadioBackend::loadState()
{
    const QString configDir = appConfigDir();
    QDir().mkpath(configDir);
    QFile file(configDir + "/state.json");
    if (!file.open(QIODevice::ReadOnly)) {
        return;
    }

    const QJsonDocument doc = QJsonDocument::fromJson(file.readAll());
    if (!doc.isObject()) {
        return;
    }

    const QJsonObject obj = doc.object();
    m_lastStationName = obj.value("lastStationName").toString();
    m_lastStationUrl = obj.value("lastStationUrl").toString();
    if (!m_lastStationUrl.isEmpty() && !isAllowedStreamUrl(m_lastStationUrl)) {
        qWarning() << "Discarding last station URL with disallowed scheme";
        m_lastStationUrl.clear();
        m_lastStationName.clear();
    }
    const double volume = obj.value("volume").toDouble(0.5);
    const QJsonArray recentArray = obj.value("recentStations").toArray();
    m_recentStations.clear();
    for (const QJsonValue &value : recentArray) {
        if (!value.isObject()) {
            continue;
        }
        const QVariantMap recentStation = value.toObject().toVariantMap();
        const QString recentUrl =
            recentStation.value(QStringLiteral("urlResolved")).toString();
        if (recentUrl.isEmpty() || !isAllowedStreamUrl(recentUrl)) {
            continue;
        }
        m_recentStations.append(recentStation);
        if (m_recentStations.size() >= kRecentLimit) {
            break;
        }
    }
    m_player->setVolume(volume);
    emit resumeStateChanged();
}

void RadioBackend::saveState() const
{
    const QString configDir = appConfigDir();
    QDir().mkpath(configDir);
    QFile file(configDir + "/state.json");
    if (!file.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
        return;
    }

    QJsonObject obj;
    obj.insert("lastStationName", m_lastStationName);
    obj.insert("lastStationUrl", m_lastStationUrl);
    obj.insert("volume", m_player->volume());
    obj.insert("recentStations", QJsonArray::fromVariantList(m_recentStations));
    file.write(QJsonDocument(obj).toJson(QJsonDocument::Indented));
}

void RadioBackend::setLoading(bool loading)
{
    if (m_loading == loading) {
        return;
    }
    m_loading = loading;
    emit loadingChanged();
}

void RadioBackend::setLastError(const QString &error)
{
    if (m_lastError == error) {
        return;
    }
    m_lastError = error;
    emit lastErrorChanged();
}

QList<RadioStation*> RadioBackend::currentList() const
{
    return m_currentFromFavorites ? m_favorites : m_filteredStations;
}

RadioStation *RadioBackend::stationForVisibleIndex(int index) const
{
    if (index < 0 || index >= m_filteredStations.size()) {
        return nullptr;
    }
    return m_filteredStations[index];
}

void RadioBackend::setFilterQuery(const QString &query)
{
    if (m_filterQuery == query && !m_filteredStations.isEmpty() && !m_stations.isEmpty()) {
        return;
    }

    m_filterQuery = query;
    rebuildFilteredStations();
}

void RadioBackend::rebuildFilteredStations(bool emitFilterSignal)
{
    m_filteredStations.clear();

    if (m_filterQuery.trimmed().isEmpty()) {
        m_filteredStations = m_stations;
    } else {
        const QString lowerQuery = m_filterQuery.toLower();
        for (RadioStation *s : m_stations) {
            if (s->name().toLower().contains(lowerQuery) ||
                s->tags().toLower().contains(lowerQuery) ||
                s->country().toLower().contains(lowerQuery)) {
                m_filteredStations.append(s);
            }
        }
    }

    if (emitFilterSignal) {
        emit filterQueryChanged();
    }
    emit stationsChanged();
    emit listsChanged();
    syncStationListIndex();

    if (!m_selectedStation.isEmpty()) {
        const QString selectedUuid = m_selectedStation.value(QStringLiteral("uuid")).toString();
        const QString selectedUrl = m_selectedStation.value(QStringLiteral("urlResolved")).toString();
        bool stillVisible = false;
        for (RadioStation *station : m_filteredStations) {
            if (matchesStation(station, selectedUuid, selectedUrl)) {
                stillVisible = true;
                break;
            }
        }
        if (!stillVisible) {
            setSelectedStation(m_filteredStations.isEmpty() ? QVariantMap() : toVariantMap(m_filteredStations.first()));
        }
    } else if (!m_filteredStations.isEmpty()) {
        setSelectedStation(toVariantMap(m_filteredStations.first()));
    }
}

void RadioBackend::recordRecentStation(const QVariantMap &stationData)
{
    const QString url = stationData.value(QStringLiteral("urlResolved")).toString();
    if (url.isEmpty() || !isAllowedStreamUrl(url)) {
        return;
    }

    QVariantMap normalizedStation = stationData;
    if (!normalizedStation.contains(QStringLiteral("name"))) {
        normalizedStation.insert(QStringLiteral("name"), QString());
    }

    for (int i = 0; i < m_recentStations.size(); ++i) {
        const QVariantMap existing = m_recentStations[i].toMap();
        const QString existingUuid = existing.value(QStringLiteral("uuid")).toString();
        const QString newUuid = normalizedStation.value(QStringLiteral("uuid")).toString();
        if ((!newUuid.isEmpty() && existingUuid == newUuid) ||
            (newUuid.isEmpty() && existing.value(QStringLiteral("urlResolved")).toString() == url)) {
            m_recentStations.removeAt(i);
            break;
        }
    }

    m_recentStations.prepend(normalizedStation);
    while (m_recentStations.size() > kRecentLimit) {
        m_recentStations.removeLast();
    }

    emit listsChanged();
}

void RadioBackend::playHistoryAtIndex(int index, bool updateRecent)
{
    if (index < 0 || index >= m_recentStations.size()) {
        return;
    }

    const QVariantMap station = m_recentStations.at(index).toMap();
    const QString url = station.value(QStringLiteral("urlResolved")).toString();
    if (url.isEmpty() || !isAllowedStreamUrl(url)) {
        return;
    }

    m_currentFromFavorites = false;
    m_currentFromHistory = true;
    m_standalonePlayback = false;
    m_currentIndex = index;

    m_lastStationName = station.value(QStringLiteral("name")).toString();
    m_lastStationUrl = url;
    emit resumeStateChanged();

    if (updateRecent) {
        recordRecentStation(station);
        m_currentIndex = 0;
    }

    setSelectedStation(station);
    saveState();
    m_currentStationUuid = station.value(QStringLiteral("uuid")).toString();
    m_currentStationUrl = url;
    emit currentStationChanged();
    if (!startPlayback(url, m_lastStationName.isEmpty() ? tr("Last played")
                                                        : m_lastStationName)) {
        m_lastStationUrl.clear();
        m_currentStationUrl.clear();
        emit resumeStateChanged();
        emit currentStationChanged();
    }
}

void RadioBackend::syncStationListIndex()
{
    if (m_currentFromHistory || m_currentFromFavorites) {
        return;
    }

    m_currentIndex = -1;
    for (int i = 0; i < m_filteredStations.size(); ++i) {
        if (matchesStation(m_filteredStations[i], m_currentStationUuid, m_currentStationUrl)) {
            m_currentIndex = i;
            break;
        }
    }
}

bool RadioBackend::startPlayback(const QString &url, const QString &name)
{
    if (!isAllowedStreamUrl(url)) {
        setLastError(tr("Stream URL must use http:// or https://"));
        qWarning() << "Rejected stream playback for disallowed URL scheme";
        return false;
    }
    setLastError(QString());
    m_player->playUrl(url, name);
    return true;
}

void RadioBackend::playCurrentSelection()
{
    const QList<RadioStation*> list = currentList();
    if (list.isEmpty() || m_currentIndex < 0 || m_currentIndex >= list.size()) {
        return;
    }

    m_currentFromHistory = false;
    m_standalonePlayback = false;

    RadioStation *station = list[m_currentIndex];
    setSelectedStation(toVariantMap(station));
    QString url = station->urlResolved().isEmpty()
                  ? station->url()
                  : station->urlResolved();
    if (!isAllowedStreamUrl(url)) {
        setLastError(tr("Stream URL must use http:// or https://"));
        qWarning() << "Rejected station list playback for disallowed URL scheme";
        return;
    }
    m_lastStationName = station->name();
    m_lastStationUrl = url;
    emit resumeStateChanged();
    recordRecentStation(toVariantMap(station));
    saveState();
    m_currentStationUuid = station->uuid();
    m_currentStationUrl = url;
    emit currentStationChanged();
    startPlayback(url, station->name());
}

QObject* RadioBackend::currentStation() const
{
    for (RadioStation *s : m_favorites) {
        if (matchesStation(s, m_currentStationUuid, m_currentStationUrl)) {
            return s;
        }
    }
    for (RadioStation *s : m_stations) {
        if (matchesStation(s, m_currentStationUuid, m_currentStationUrl)) {
            return s;
        }
    }
    return nullptr;
}

void RadioBackend::setSelectedStation(const QVariantMap &station)
{
    if (m_selectedStation == station) {
        return;
    }

    m_selectedStation = station;
    emit selectedStationChanged();
}

void RadioBackend::editManualStation(QObject *stationObj, const QString &name, const QString &url, const QString &country)
{
    RadioStation *station = qobject_cast<RadioStation*>(stationObj);
    if (!station || name.trimmed().isEmpty() || url.trimmed().isEmpty()) {
        return;
    }
    if (!isAllowedStreamUrl(url)) {
        setLastError(tr("Stream URL must use http:// or https://"));
        return;
    }

    setLastError(QString());
    QString oldUrl = station->url();
    QString oldUrlResolved = station->urlResolved();

    station->setName(name.trimmed());
    station->setUrl(url.trimmed());
    station->setUrlResolved(url.trimmed());
    station->setCountry(country.trimmed().isEmpty() ? tr("Manual") : country.trimmed());

    // Update in favorites list if present
    for (RadioStation *fav : m_favorites) {
        if (fav == station || (fav->uuid().isEmpty() && (fav->url() == oldUrl || fav->urlResolved() == oldUrlResolved))) {
            fav->setName(station->name());
            fav->setUrl(station->url());
            fav->setUrlResolved(station->urlResolved());
            fav->setCountry(station->country());
        }
    }

    // Save updated favorites
    saveFavorites();

    // Update in main stations list if present
    for (RadioStation *s : m_stations) {
        if (s == station || (s->uuid().isEmpty() && (s->url() == oldUrl || s->urlResolved() == oldUrlResolved))) {
            s->setName(station->name());
            s->setUrl(station->url());
            s->setUrlResolved(station->urlResolved());
            s->setCountry(station->country());
        }
    }

    // If it's the currently playing station, update player stream info
    if (m_currentStationUrl == oldUrl || m_currentStationUrl == oldUrlResolved) {
        m_currentStationUrl = station->urlResolved();
        m_lastStationUrl = station->urlResolved();
        m_lastStationName = station->name();
        saveState();
        emit resumeStateChanged();
        emit currentStationChanged();
    }

    const QString selectedUrl = m_selectedStation.value(QStringLiteral("urlResolved")).toString();
    if (selectedUrl == oldUrl || selectedUrl == oldUrlResolved) {
        setSelectedStation(toVariantMap(station));
    }

    emit stationsChanged();
    emit favoritesChanged();
    emit listsChanged();
}

QVariantMap RadioBackend::toVariantMap(const RadioStation *station)
{
    QVariantMap map;
    if (!station) {
        return map;
    }
    map.insert(QStringLiteral("uuid"), station->uuid());
    map.insert(QStringLiteral("name"), station->name());
    map.insert(QStringLiteral("country"), station->country());
    map.insert(QStringLiteral("bitrate"), station->bitrate());
    map.insert(QStringLiteral("votes"), station->votes());
    map.insert(QStringLiteral("isFavorite"), station->isFavorite());
    map.insert(QStringLiteral("favicon"), station->favicon());
    map.insert(QStringLiteral("homepage"), station->homepage());
    map.insert(QStringLiteral("url"), station->url());
    map.insert(QStringLiteral("urlResolved"), station->urlResolved());
    map.insert(QStringLiteral("codec"), station->codec());
    map.insert(QStringLiteral("tags"), station->tags());
    map.insert(QStringLiteral("isOnline"), station->isOnline());
    return map;
}

bool RadioBackend::matchesStation(const RadioStation *station, const QString &uuid, const QString &urlResolved)
{
    if (!station) {
        return false;
    }
    if (!uuid.isEmpty() && station->uuid() == uuid) {
        return true;
    }
    if (uuid.isEmpty() && !urlResolved.isEmpty() && station->urlResolved() == urlResolved) {
        return true;
    }
    return false;
}

bool RadioBackend::matchesStationMap(const QVariantMap &station, const QString &uuid, const QString &urlResolved)
{
    const QString stationUuid = station.value(QStringLiteral("uuid")).toString();
    const QString stationUrl = station.value(QStringLiteral("urlResolved")).toString();
    if (!uuid.isEmpty() && stationUuid == uuid) {
        return true;
    }
    if (uuid.isEmpty() && !urlResolved.isEmpty() && stationUrl == urlResolved) {
        return true;
    }
    return false;
}

int RadioBackend::recentStationIndex(const QVariantList &recent, const QString &uuid, const QString &urlResolved)
{
    for (int i = 0; i < recent.size(); ++i) {
        if (matchesStationMap(recent.at(i).toMap(), uuid, urlResolved)) {
            return i;
        }
    }
    return -1;
}
