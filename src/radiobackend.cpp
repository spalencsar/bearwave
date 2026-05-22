#include "radiobackend.h"

#include <QDebug>
#include <algorithm>
#include <QDir>
#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QStandardPaths>

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
    connect(m_radioBrowser, &RadioBrowser::error,
            this, [this](const QString &error) {
        qWarning() << "RadioBrowser error:" << error;
        setLastError(error);
        setLoading(false);
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
    setLastError(QString());
    setLoading(false);
    qDebug() << "Loaded" << stations.size() << "stations";
}

void RadioBackend::loadGermanStations()
{
    setLoading(true);
    m_radioBrowser->getGermanStations();
}

void RadioBackend::loadDutchStations()
{
    setLoading(true);
    m_radioBrowser->getDutchStations();
}

void RadioBackend::loadTopStations()
{
    setLoading(true);
    m_radioBrowser->getTopStations(100);
}

void RadioBackend::loadWorldStations()
{
    setLoading(true);
    m_radioBrowser->getWorldStations(200);
}

void RadioBackend::loadByTag(const QString &tag)
{
    if (tag.trimmed().isEmpty()) {
        return;
    }
    setLoading(true);
    m_radioBrowser->getByTag(tag.trimmed());
}

void RadioBackend::loadByCountryCode(const QString &countryCode)
{
    if (countryCode.trimmed().isEmpty()) {
        return;
    }
    setLoading(true);
    m_radioBrowser->getByCountry(countryCode.trimmed().toUpper());
}

void RadioBackend::searchStations(const QString &query)
{
    if (query.isEmpty()) {
        return;
    }
    qDebug() << "RadioBackend: searchStations called with query:" << query;
    setLoading(true);
    m_radioBrowser->search(query);
}

void RadioBackend::playStation(int index)
{
    if (!stationForVisibleIndex(index)) {
        return;
    }

    m_currentFromFavorites = false;
    m_currentIndex = index;

    playCurrentSelection();
}

void RadioBackend::playFavoriteStation(int index)
{
    if (index < 0 || index >= m_favorites.size()) {
        return;
    }

    m_currentFromFavorites = true;
    m_currentIndex = index;

    playCurrentSelection();
}

void RadioBackend::playNextStation()
{
    const QList<RadioStation*> list = currentList();
    if (list.isEmpty()) {
        return;
    }
    if (m_currentIndex < 0) {
        m_currentIndex = 0;
    } else if (m_currentIndex < list.size() - 1) {
        ++m_currentIndex;
    }
    playCurrentSelection();
}

void RadioBackend::playPreviousStation()
{
    const QList<RadioStation*> list = currentList();
    if (list.isEmpty()) {
        return;
    }
    if (m_currentIndex < 0) {
        m_currentIndex = 0;
    } else if (m_currentIndex > 0) {
        --m_currentIndex;
    }
    playCurrentSelection();
}

bool RadioBackend::hasNextStation() const
{
    const QList<RadioStation*> list = currentList();
    if (list.isEmpty()) {
        return false;
    }
    if (m_currentIndex < 0) {
        return true;
    }
    return m_currentIndex < list.size() - 1;
}

bool RadioBackend::hasPreviousStation() const
{
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
    for (RadioStation *station : m_stations) {
        if (matchesStation(station, uuid, urlResolved)) {
            const QString url = station->urlResolved().isEmpty() ? station->url() : station->urlResolved();
            m_lastStationName = station->name();
            m_lastStationUrl = url;
            emit resumeStateChanged();
            recordRecentStation(toVariantMap(station));
            saveState();
            m_currentStationUuid = station->uuid();
            m_currentStationUrl = url;
            emit currentStationChanged();
            m_player->playUrl(url, station->name());
            return true;
        }
    }

    for (const QVariant &entry : m_recentStations) {
        const QVariantMap station = entry.toMap();
        if ((!uuid.isEmpty() && station.value(QStringLiteral("uuid")).toString() == uuid) ||
            (uuid.isEmpty() && !urlResolved.isEmpty() && station.value(QStringLiteral("urlResolved")).toString() == urlResolved)) {
            const QString url = station.value(QStringLiteral("urlResolved")).toString();
            if (url.isEmpty()) {
                return false;
            }
            m_lastStationName = station.value(QStringLiteral("name")).toString();
            m_lastStationUrl = url;
            emit resumeStateChanged();
            recordRecentStation(station);
            saveState();
            m_currentStationUuid = station.value(QStringLiteral("uuid")).toString();
            m_currentStationUrl = url;
            emit currentStationChanged();
            m_player->playUrl(url, m_lastStationName);
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

void RadioBackend::playCurrentSelection()
{
    const QList<RadioStation*> list = currentList();
    if (list.isEmpty() || m_currentIndex < 0 || m_currentIndex >= list.size()) {
        return;
    }

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
