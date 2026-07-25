// Copyright (c) 2026 Sebastian Palencsar
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QHash>
#include <QList>
#include <QObject>
#include <QQueue>
#include <QSet>
#include <QStringList>
#include <QUrl>

class QNetworkAccessManager;
class QNetworkReply;
class StationImageCache;

class StationArtworkService : public QObject
{
    Q_OBJECT

public:
    explicit StationArtworkService(StationImageCache *imageCache,
                                   QObject *parent = nullptr);
    StationArtworkService(StationImageCache *imageCache, bool allowHttpForTests,
                          QObject *parent = nullptr);

    QString sourceForStation(const QString &stationKey, const QString &faviconUrl,
                             const QString &homepageUrl);
    int revision() const { return m_revision; }

    static QStringList htmlCandidates(const QByteArray &html, const QUrl &pageUrl,
                                      QUrl *manifestUrl = nullptr,
                                      QStringList *openGraphImages = nullptr);
    static QStringList manifestCandidates(const QByteArray &json,
                                          const QUrl &manifestUrl);
    static bool isSafeNetworkUrl(const QUrl &url, bool allowHttp);

signals:
    void revisionChanged();

private:
    struct Candidate {
        QString url;
        bool requireSquare = false;
    };
    struct StationState {
        QString favicon;
        QString homepage;
        QString source;
        QList<Candidate> candidates;
        int nextCandidate = 0;
        bool started = false;
        bool complete = false;
        bool discovering = false;
        bool discoveryAttempted = false;
    };
    struct DiscoveryJob {
        QString homepage;
        QUrl url;
        QUrl pageUrl;
        QUrl manifestUrl;
        QStringList primaryCandidates;
        QStringList openGraphCandidates;
        int redirectCount = 0;
        bool manifestPhase = false;
    };

    void tryNextCandidate(const QString &stationKey);
    void requestCandidate(const QString &stationKey, const Candidate &candidate);
    void handleImageReady(const QString &remoteUrl, const QString &localUrl);
    void handleImageRejected(const QString &remoteUrl);
    void startDiscovery(const QString &stationKey);
    void pumpDiscoveryQueue();
    void fetchDiscovery(const DiscoveryJob &job);
    void finishDiscoveryReply(QNetworkReply *reply);
    void completeDiscovery(const DiscoveryJob &job,
                           const QStringList &manifestIcons = {});
    void assignDiscoveryCandidates(const QString &homepage,
                                   const QList<Candidate> &candidates);
    void completeWithoutLogo(const QString &stationKey);
    void advanceRevision();
    static bool logoIsUsable(const QString &localUrl, bool requireSquare);

    StationImageCache *m_imageCache = nullptr;
    QNetworkAccessManager *m_networkManager = nullptr;
    bool m_allowHttpForTests = false;
    int m_revision = 0;
    bool m_revisionUpdatePending = false;
    int m_activeDiscoveries = 0;
    QHash<QString, StationState> m_stations;
    QHash<QString, QHash<QString, bool>> m_imageWaiters;
    QHash<QString, QList<Candidate>> m_discoveryCache;
    QHash<QString, QSet<QString>> m_discoveryWaiters;
    QQueue<QString> m_discoveryQueue;
};
