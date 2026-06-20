// Copyright (c) 2026 Sebastian Palencsar
// SPDX-License-Identifier: GPL-3.0-or-later

#include "radiobackend.h"

#include <QDebug>
#include <algorithm>
#include <QDir>
#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QStandardPaths>
#include <QLocale>
#include <QUrl>

namespace {
constexpr int kRecentLimit = 20;

QString appConfigDir()
{
    return QDir::homePath() + QStringLiteral("/.config/bearwave");
}

bool isAllowedStreamUrl(const QString &urlString)
{
    const QUrl url(urlString.trimmed());
    if (!url.isValid() || url.isEmpty()) {
        return false;
    }
    const QString scheme = url.scheme().toLower();
    return scheme == QStringLiteral("http") || scheme == QStringLiteral("https");
}
}

RadioBackend::RadioBackend(QObject *parent)
    : QObject(parent)
{
    m_radioBrowser = new RadioBrowser(this);
    m_player = new BearPlayer(this);
    setupConnections();
    loadFavorites();
    loadState();
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

void RadioBackend::onCountriesLoaded(const QVariantList &countries)
{
    m_countries.clear();
    for (const QVariant &item : countries) {
        QVariantMap map = item.toMap();
        QString code = map["code"].toString();
        QString englishName = map["name"].toString();
        map["name"] = localizeCountry(code, englishName);
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
            saveFavorites();
            emit favoritesChanged();
            emit stationsChanged();
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
                }
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

    m_player->playUrl(m_lastStationUrl, m_lastStationName.isEmpty() ? tr("Last played") : m_lastStationName);
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
        RadioStation *s = new RadioStation(this);
        s->setUuid(o.value("uuid").toString());
        s->setName(o.value("name").toString());
        s->setUrl(o.value("url").toString());
        s->setUrlResolved(o.value("urlResolved").toString());
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
    const double volume = obj.value("volume").toDouble(0.5);
    const QJsonArray recentArray = obj.value("recentStations").toArray();
    m_recentStations.clear();
    for (const QJsonValue &value : recentArray) {
        if (!value.isObject()) {
            continue;
        }
        const QVariantMap recentStation = value.toObject().toVariantMap();
        if (recentStation.value(QStringLiteral("urlResolved")).toString().isEmpty()) {
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
}

void RadioBackend::recordRecentStation(const QVariantMap &stationData)
{
    const QString url = stationData.value(QStringLiteral("urlResolved")).toString();
    if (url.isEmpty()) {
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
    if (url.isEmpty()) {
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

    saveState();
    m_currentStationUuid = station.value(QStringLiteral("uuid")).toString();
    m_currentStationUrl = url;
    emit currentStationChanged();
    m_player->playUrl(url, m_lastStationName.isEmpty() ? tr("Last played") : m_lastStationName);
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

void RadioBackend::playCurrentSelection()
{
    const QList<RadioStation*> list = currentList();
    if (list.isEmpty() || m_currentIndex < 0 || m_currentIndex >= list.size()) {
        return;
    }

    m_currentFromHistory = false;
    m_standalonePlayback = false;

    RadioStation *station = list[m_currentIndex];
    QString url = station->urlResolved().isEmpty()
                  ? station->url()
                  : station->urlResolved();
    m_lastStationName = station->name();
    m_lastStationUrl = url;
    emit resumeStateChanged();
    recordRecentStation(toVariantMap(station));
    saveState();
    m_currentStationUuid = station->uuid();
    m_currentStationUrl = url;
    emit currentStationChanged();
    m_player->playUrl(url, station->name());
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
    map.insert(QStringLiteral("isFavorite"), station->isFavorite());
    map.insert(QStringLiteral("favicon"), station->favicon());
    map.insert(QStringLiteral("urlResolved"), station->urlResolved());
    map.insert(QStringLiteral("codec"), station->codec());
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

QString RadioBackend::localizeCountry(const QString &code, const QString &englishName)
{
    QLocale locale;
    if (locale.language() != QLocale::German) {
        return englishName;
    }

    static const QHash<QString, QString> germanCountries = {
        {"AD", "Andorra"}, {"AE", "Vereinigte Arabische Emirate"}, {"AF", "Afghanistan"},
        {"AG", "Antigua und Barbuda"}, {"AI", "Anguilla"}, {"AL", "Albanien"},
        {"AM", "Armenien"}, {"AO", "Angola"}, {"AQ", "Antarktis"}, {"AR", "Argentinien"},
        {"AS", "Amerikanisch-Samoa"}, {"AT", "Österreich"}, {"AU", "Australien"},
        {"AW", "Aruba"}, {"AX", "Åland-Inseln"}, {"AZ", "Aserbaidschan"},
        {"BA", "Bosnien und Herzegowina"}, {"BB", "Barbados"}, {"BD", "Bangladesch"},
        {"BE", "Belgien"}, {"BF", "Burkina Faso"}, {"BG", "Bulgarien"},
        {"BH", "Bahrain"}, {"BI", "Burundi"}, {"BJ", "Benin"},
        {"BL", "St. Bartholomäus"}, {"BM", "Bermuda"}, {"BN", "Brunei"},
        {"BO", "Bolivien"}, {"BQ", "Bonaire"}, {"BR", "Brasilien"},
        {"BS", "Bahamas"}, {"BT", "Bhutan"}, {"BV", "Bouvetinsel"},
        {"BW", "Botswana"}, {"BY", "Belarus"}, {"BZ", "Belize"},
        {"CA", "Kanada"}, {"CC", "Kokosinseln"}, {"CD", "Kongo-Kinshasa"},
        {"CF", "Zentralafrikanische Republik"}, {"CG", "Kongo-Brazzaville"}, {"CH", "Schweiz"},
        {"CI", "Elfenbeinküste"}, {"CK", "Cookinseln"}, {"CL", "Chile"},
        {"CM", "Kamerun"}, {"CN", "China"}, {"CO", "Kolumbien"},
        {"CR", "Costa Rica"}, {"CU", "Kuba"}, {"CV", "Cabo Verde"},
        {"CW", "Curaçao"}, {"CX", "Weihnachtsinsel"}, {"CY", "Zypern"},
        {"CZ", "Tschechien"}, {"DE", "Deutschland"}, {"DJ", "Dschibuti"},
        {"DK", "Dänemark"}, {"DM", "Dominica"}, {"DO", "Dominikanische Republik"},
        {"DZ", "Algerien"}, {"EC", "Ecuador"}, {"EE", "Estland"},
        {"EG", "Ägypten"}, {"EH", "Westsahara"}, {"ER", "Eritrea"},
        {"ES", "Spanien"}, {"ET", "Äthiopien"}, {"FI", "Finnland"},
        {"FJ", "Fidschi"}, {"FK", "Falklandinseln"}, {"FM", "Mikronesien"},
        {"FO", "Färöer"}, {"FR", "Frankreich"}, {"GA", "Gabun"},
        {"GB", "Großbritannien"}, {"GD", "Grenada"}, {"GE", "Georgien"},
        {"GF", "Französisch-Guayana"}, {"GG", "Guernsey"}, {"GH", "Ghana"},
        {"GI", "Gibraltar"}, {"GL", "Grönland"}, {"GM", "Gambia"},
        {"GN", "Guinea"}, {"GP", "Guadeloupe"}, {"GQ", "Äquatorialguinea"},
        {"GR", "Griechenland"}, {"GS", "Südgeorgien"}, {"GT", "Guatemala"},
        {"GU", "Guam"}, {"GW", "Guinea-Bissau"}, {"GY", "Guyana"},
        {"HK", "Hongkong"}, {"HM", "Heard und McDonaldinseln"}, {"HN", "Honduras"},
        {"HR", "Kroatien"}, {"HT", "Haiti"}, {"HU", "Ungarn"},
        {"ID", "Indonesien"}, {"IE", "Irland"}, {"IL", "Israel"},
        {"IM", "Isle of Man"}, {"IN", "Indien"}, {"IO", "Britisches Territorium im Indischen Ozean"},
        {"IQ", "Irak"}, {"IR", "Iran"}, {"IS", "Island"},
        {"IT", "Italien"}, {"JE", "Jersey"}, {"JM", "Jamaika"},
        {"JO", "Jordanien"}, {"JP", "Japan"}, {"KE", "Kenia"},
        {"KG", "Kirgisistan"}, {"KH", "Kambodscha"}, {"KI", "Kiribati"},
        {"KM", "Komoren"}, {"KN", "St. Kitts und Nevis"}, {"KP", "Nordkorea"},
        {"KR", "Südkorea"}, {"KW", "Kuwait"}, {"KY", "Kaimaninseln"},
        {"KZ", "Kasachstan"}, {"LA", "Laos"}, {"LB", "Libanon"},
        {"LC", "St. Lucia"}, {"LI", "Liechtenstein"}, {"LK", "Sri Lanka"},
        {"LR", "Liberia"}, {"LS", "Lesotho"}, {"LT", "Litauen"},
        {"LU", "Luxemburg"}, {"LV", "Lettland"}, {"LY", "Libyen"},
        {"MA", "Marokko"}, {"MC", "Monaco"}, {"MD", "Moldawien"},
        {"ME", "Montenegro"}, {"MF", "St. Martin"}, {"MG", "Madagaskar"},
        {"MH", "Marshallinseln"}, {"MK", "Nordmazedonien"}, {"ML", "Mali"},
        {"MM", "Myanmar"}, {"MN", "Mongolei"}, {"MO", "Macau"},
        {"MP", "Nördliche Marianen"}, {"MQ", "Martinique"}, {"MR", "Mauretanien"},
        {"MS", "Montserrat"}, {"MT", "Malta"}, {"MU", "Mauritius"},
        {"MV", "Malediven"}, {"MW", "Malawi"}, {"MX", "Mexiko"},
        {"MY", "Malaysia"}, {"MZ", "Mosambik"}, {"NA", "Namibia"},
        {"NC", "Neukaledonien"}, {"NE", "Niger"}, {"NF", "Norfolkinsel"},
        {"NG", "Nigeria"}, {"NI", "Nicaragua"}, {"NL", "Niederlande"},
        {"NO", "Norwegen"}, {"NP", "Nepal"}, {"NR", "Nauru"},
        {"NU", "Niue"}, {"NZ", "Neuseeland"}, {"OM", "Oman"},
        {"PA", "Panama"}, {"PE", "Peru"}, {"PF", "Französisch-Polynesien"},
        {"PG", "Papua-Neuguinea"}, {"PH", "Philippinen"}, {"PK", "Pakistan"},
        {"PL", "Polen"}, {"PM", "St. Pierre und Miquelon"}, {"PN", "Pitcairninseln"},
        {"PR", "Puerto Rico"}, {"PS", "Palästina"}, {"PT", "Portugal"},
        {"PW", "Palau"}, {"PY", "Paraguay"}, {"QA", "Katar"},
        {"RE", "Réunion"}, {"RO", "Rumänien"}, {"RS", "Serbien"},
        {"RU", "Russland"}, {"RW", "Ruanda"}, {"SA", "Saudi-Arabien"},
        {"SB", "Salomonen"}, {"SC", "Seychellen"}, {"SD", "Sudan"},
        {"SE", "Schweden"}, {"SG", "Singapur"}, {"SH", "St. Helena"},
        {"SI", "Slowenien"}, {"SJ", "Svalbard und Jan Mayen"}, {"SK", "Slowakei"},
        {"SL", "Sierra Leone"}, {"SM", "San Marino"}, {"SN", "Senegal"},
        {"SO", "Somalia"}, {"SR", "Suriname"}, {"SS", "Südsudan"},
        {"ST", "São Tomé und Príncipe"}, {"SV", "El Salvador"}, {"SX", "Sint Maarten"},
        {"SY", "Syrien"}, {"SZ", "Eswatini"}, {"TC", "Turks- und Caicosinseln"},
        {"TD", "Tschad"}, {"TF", "Französische Süd- und Antarktisgebiete"}, {"TG", "Togo"},
        {"TH", "Thailand"}, {"TJ", "Tadschikistan"}, {"TK", "Tokelau"},
        {"TL", "Osttimor"}, {"TM", "Turkmenistan"}, {"TN", "Tunesien"},
        {"TO", "Tonga"}, {"TR", "Türkei"}, {"TT", "Trinidad und Tobago"},
        {"TV", "Tuvalu"}, {"TW", "Taiwan"}, {"TZ", "Tansania"},
        {"UA", "Ukraine"}, {"UG", "Uganda"}, {"UM", "Amerikanische Überseeinseln"},
        {"US", "Vereinigte Staaten"}, {"UY", "Uruguay"}, {"UZ", "Usbekistan"},
        {"VA", "Vatikanstadt"}, {"VC", "St. Vincent und die Grenadinen"}, {"VE", "Venezuela"},
        {"VG", "Britische Jungferninseln"}, {"VI", "Amerikanische Jungferninseln"}, {"VN", "Vietnam"},
        {"VU", "Vanuatu"}, {"WF", "Wallis und Futuna"}, {"WS", "Samoa"},
        {"YE", "Jemen"}, {"YT", "Mayotte"}, {"ZA", "Südafrika"},
        {"ZM", "Sambia"}, {"ZW", "Simbabwe"}
    };

    QString result = germanCountries.value(code.toUpper());
    return result.isEmpty() ? englishName : result;
}
