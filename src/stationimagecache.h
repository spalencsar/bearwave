// Copyright (c) 2026 Sebastian Palencsar
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QObject>
#include <QHash>
#include <QSet>
#include <QString>

class QNetworkAccessManager;
class QNetworkReply;

class StationImageCache : public QObject
{
    Q_OBJECT
    Q_PROPERTY(int revision READ revision NOTIFY revisionChanged)

public:
    explicit StationImageCache(QObject *parent = nullptr);
    StationImageCache(const QString &cacheDirectory, bool allowHttpForTests,
                      QObject *parent = nullptr);

    Q_INVOKABLE QString sourceForUrl(const QString &remoteUrl);

    int revision() const;
    bool isBlocked(const QString &remoteUrl) const;
    static void pruneCacheDirectory(const QString &cacheDirectory,
                                    qint64 maximumBytes = 50 * 1024 * 1024,
                                    int maximumAgeDays = 30);

signals:
    void revisionChanged();
    void imageReady(const QString &remoteUrl, const QString &localUrl);
    void imageRejected(const QString &remoteUrl);

private:
    static constexpr qint64 maximumDownloadBytes = 2 * 1024 * 1024;
    static constexpr int maximumDimension = 2048;
    static constexpr int cachedDimension = 512;

    QString cachePathForUrl(const QString &remoteUrl) const;
    QString localSourceForPath(const QString &path) const;
    bool cachedFileIsValid(const QString &path) const;
    bool isAllowedRemoteUrl(const QString &remoteUrl) const;
    void startDownload(const QString &remoteUrl);
    void finishDownload(QNetworkReply *reply);
    void reject(const QString &remoteUrl);
    void advanceRevision();
    void touchCacheFile(const QString &path) const;

    QNetworkAccessManager *m_networkManager = nullptr;
    QString m_cacheDirectory;
    bool m_allowHttpForTests = false;
    int m_revision = 0;
    QHash<QString, QString> m_sources;
    QSet<QString> m_pending;
    QSet<QString> m_blocked;
};
