#ifndef RADIOBACKEND_H
#define RADIOBACKEND_H

#include <QObject>
#include <QQmlEngine>
#include <QList>
#include <QVariantMap>
#include <QVariantList>

#include "radiobrowser.h"
#include "bearplayer.h"

class RadioStation;

class RadioBackend : public QObject
{
    Q_OBJECT

    Q_PROPERTY(QList<QObject*> stations READ stations NOTIFY stationsChanged)
    Q_PROPERTY(QList<QObject*> favoriteStations READ favoriteStations NOTIFY favoritesChanged)
    Q_PROPERTY(BearPlayer* player READ player CONSTANT)
    Q_PROPERTY(bool loading READ loading NOTIFY loadingChanged)
    Q_PROPERTY(QString lastError READ lastError NOTIFY lastErrorChanged)
    Q_PROPERTY(bool canResumeLastStation READ canResumeLastStation NOTIFY resumeStateChanged)
    Q_PROPERTY(QString lastStationName READ lastStationName NOTIFY resumeStateChanged)
    Q_PROPERTY(QString filterQuery READ filterQuery WRITE setFilterQuery NOTIFY filterQueryChanged)

public:
    explicit RadioBackend(QObject *parent = nullptr);

    QList<QObject*> stations() const;
    QList<QObject*> favoriteStations() const;
    BearPlayer* player() const { return m_player; }
    bool loading() const { return m_loading; }
    QString lastError() const { return m_lastError; }
    bool canResumeLastStation() const { return !m_lastStationUrl.isEmpty(); }
    QString lastStationName() const { return m_lastStationName; }
    QString filterQuery() const { return m_filterQuery; }

    Q_INVOKABLE void setFilterQuery(const QString &query);

    Q_INVOKABLE void loadGermanStations();
    Q_INVOKABLE void loadDutchStations();
    Q_INVOKABLE void loadTopStations();
    Q_INVOKABLE void loadWorldStations();
    Q_INVOKABLE void loadByTag(const QString &tag);
    Q_INVOKABLE void loadByCountryCode(const QString &countryCode);
    Q_INVOKABLE void searchStations(const QString &query);
    Q_INVOKABLE void playStation(int index);
    Q_INVOKABLE void playFavoriteStation(int index);
    Q_INVOKABLE void playNextStation();
    Q_INVOKABLE void playPreviousStation();
    Q_INVOKABLE bool hasNextStation() const;
    Q_INVOKABLE bool hasPreviousStation() const;
    Q_INVOKABLE void toggleFavorite(int index);
    Q_INVOKABLE void toggleFavoriteById(const QString &uuid, const QString &urlResolved);
    Q_INVOKABLE void addManualStation(const QString &name, const QString &url, const QString &country);
    Q_INVOKABLE void sortStations(const QString &mode);
    Q_INVOKABLE QVariantList getRecentStations() const;
    Q_INVOKABLE QVariantList getFavoriteStations() const;
    Q_INVOKABLE bool playRecentByUuid(const QString &uuid, const QString &urlResolved = QString());
    Q_INVOKABLE bool playFavoriteByUuid(const QString &uuid, const QString &urlResolved = QString());
    Q_INVOKABLE void resumeLastStation();

signals:
    void stationsChanged();
    void favoritesChanged();
    void listsChanged();
    void loadingChanged();
    void lastErrorChanged();
    void resumeStateChanged();
    void filterQueryChanged();

private slots:
    void onStationsLoaded(const QList<RadioStation*> &stations);

private:
    RadioBrowser *m_radioBrowser = nullptr;
    BearPlayer *m_player = nullptr;
    QList<RadioStation*> m_stations;
    QList<RadioStation*> m_filteredStations;
    QList<RadioStation*> m_favorites;
    int m_currentIndex = -1;
    bool m_currentFromFavorites = false;
    bool m_loading = false;
    QString m_lastError;
    QString m_lastStationName;
    QString m_lastStationUrl;
    QString m_filterQuery;
    QVariantList m_recentStations;

    void setupConnections();
    void loadFavorites();
    void saveFavorites() const;
    void loadState();
    void saveState() const;
    void setLoading(bool loading);
    void setLastError(const QString &error);
    QList<RadioStation*> currentList() const;
    RadioStation *stationForVisibleIndex(int index) const;
    void rebuildFilteredStations(bool emitFilterSignal = true);
    void recordRecentStation(const QVariantMap &stationData);
    void playCurrentSelection();
    static QVariantMap toVariantMap(const RadioStation *station);
    static bool matchesStation(const RadioStation *station, const QString &uuid, const QString &urlResolved);
};

#endif
