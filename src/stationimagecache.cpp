// Copyright (c) 2026 Sebastian Palencsar
// SPDX-License-Identifier: GPL-3.0-or-later

#include "stationimagecache.h"
#include "bearwavepaths.h"
#include "networkurlguard.h"

#include <algorithm>
#include <QBuffer>
#include <QCryptographicHash>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QImage>
#include <QImageReader>
#include <QNetworkAccessManager>
#include <QNetworkProxy>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QSaveFile>
#include <QDateTime>
#include <QUrl>

namespace {
const QString fallbackSource = QStringLiteral("qrc:/assets/app/bearwave.svg");
constexpr int maximumRedirects = 5;

QString defaultCacheDirectory()
{
    return BearwavePaths::coversCacheDir();
}
}

StationImageCache::StationImageCache(QObject *parent)
    : StationImageCache(defaultCacheDirectory(), false, parent)
{
}

StationImageCache::StationImageCache(const QString &cacheDirectory, bool allowHttpForTests,
                                     QObject *parent)
    : QObject(parent)
    , m_networkManager(new QNetworkAccessManager(this))
    , m_cacheDirectory(cacheDirectory)
    , m_allowHttpForTests(allowHttpForTests)
{
    m_networkManager->setProxy(QNetworkProxy::NoProxy);
    QDir().mkpath(m_cacheDirectory);
    pruneCacheDirectory(m_cacheDirectory);
}

QString StationImageCache::sourceForUrl(const QString &remoteUrl)
{
    const QString normalizedUrl = remoteUrl.trimmed();
    if (!hasAllowedRemoteUrlShape(normalizedUrl)
        || m_blocked.contains(normalizedUrl)) {
        return fallbackSource;
    }

    const auto knownSource = m_sources.constFind(normalizedUrl);
    if (knownSource != m_sources.constEnd()) {
        const QString path = QUrl(*knownSource).toLocalFile();
        if (QFile::exists(path)) {
            touchCacheFile(path);
            return *knownSource;
        }
        m_sources.remove(normalizedUrl);
    }

    const QString cachePath = cachePathForUrl(normalizedUrl);
    if (QFile::exists(cachePath)) {
        if (cachedFileIsValid(cachePath)) {
            const QString localSource = localSourceForPath(cachePath);
            touchCacheFile(cachePath);
            m_sources.insert(normalizedUrl, localSource);
            return localSource;
        }
        QFile::remove(cachePath);
    }

    if (!m_pending.contains(normalizedUrl)) {
        startDownload(normalizedUrl);
    }
    return fallbackSource;
}

int StationImageCache::revision() const
{
    return m_revision;
}

bool StationImageCache::isBlocked(const QString &remoteUrl) const
{
    return m_blocked.contains(remoteUrl.trimmed());
}

void StationImageCache::pruneCacheDirectory(const QString &cacheDirectory,
                                            qint64 maximumBytes,
                                            int maximumAgeDays)
{
    QDir directory(cacheDirectory);
    if (!directory.exists()) {
        return;
    }

    QFileInfoList files = directory.entryInfoList(
        {QStringLiteral("*.png"), QStringLiteral("*.jpg"),
         QStringLiteral("*.jpeg"), QStringLiteral("*.webp")},
        QDir::Files | QDir::NoSymLinks, QDir::Time);
    const QDateTime cutoff = QDateTime::currentDateTimeUtc().addDays(-maximumAgeDays);
    qint64 totalBytes = 0;
    QFileInfoList retainedFiles;
    for (const QFileInfo &file : files) {
        if (maximumAgeDays >= 0 && file.lastModified().toUTC() < cutoff) {
            QFile::remove(file.absoluteFilePath());
            continue;
        }
        totalBytes += file.size();
        retainedFiles.append(file);
    }

    std::sort(retainedFiles.begin(), retainedFiles.end(),
              [](const QFileInfo &left, const QFileInfo &right) {
        return left.lastModified() < right.lastModified();
    });
    for (const QFileInfo &file : retainedFiles) {
        if (totalBytes <= maximumBytes) {
            break;
        }
        if (QFile::remove(file.absoluteFilePath())) {
            totalBytes -= file.size();
        }
    }
}

QString StationImageCache::cachePathForUrl(const QString &remoteUrl) const
{
    const QByteArray hash =
        QCryptographicHash::hash(remoteUrl.toUtf8(), QCryptographicHash::Sha256).toHex();
    return m_cacheDirectory + QLatin1Char('/') + QString::fromLatin1(hash)
           + QStringLiteral(".png");
}

QString StationImageCache::localSourceForPath(const QString &path) const
{
    return QUrl::fromLocalFile(path).toString();
}

bool StationImageCache::cachedFileIsValid(const QString &path) const
{
    QImageReader reader(path);
    const QSize size = reader.size();
    if (!reader.canRead() || !size.isValid()
        || size.width() > maximumDimension || size.height() > maximumDimension) {
        return false;
    }
    return !reader.read().isNull();
}

bool StationImageCache::hasAllowedRemoteUrlShape(const QString &remoteUrl) const
{
    const QUrl url(remoteUrl);
    if (!url.isValid() || url.host().isEmpty()) {
        return false;
    }
    const QString scheme = url.scheme().toLower();
    if (scheme != QStringLiteral("https") && scheme != QStringLiteral("http")) {
        return false;
    }
    return url.userInfo().isEmpty();
}

void StationImageCache::startDownload(const QString &remoteUrl)
{
    m_pending.insert(remoteUrl);
    resolveDownload(remoteUrl, QUrl(remoteUrl), 0);
}

void StationImageCache::resolveDownload(const QString &remoteUrl,
                                        const QUrl &requestUrl,
                                        int redirectCount)
{
    const bool allowPrivate =
        m_allowHttpForTests
        && requestUrl.scheme().compare(QStringLiteral("http"),
                                       Qt::CaseInsensitive) == 0;
    NetworkUrlGuard::resolve(
        requestUrl, true, allowPrivate, this,
        [this, remoteUrl, requestUrl, redirectCount](
            bool allowed, const QList<QHostAddress> &addresses) {
            if (!allowed) {
                m_pending.remove(remoteUrl);
                reject(remoteUrl);
                return;
            }
            issueDownload(remoteUrl, requestUrl, redirectCount,
                          addresses.constFirst());
        });
}

void StationImageCache::issueDownload(const QString &remoteUrl,
                                      const QUrl &requestUrl,
                                      int redirectCount,
                                      const QHostAddress &resolvedAddress)
{

    QNetworkRequest request =
        NetworkUrlGuard::requestForResolvedUrl(requestUrl, resolvedAddress);
    request.setTransferTimeout(10000);
    request.setAttribute(QNetworkRequest::RedirectPolicyAttribute,
                         QNetworkRequest::ManualRedirectPolicy);
    request.setHeader(QNetworkRequest::UserAgentHeader,
                      QStringLiteral("BearWave/1.0 station image cache"));

    QNetworkReply *reply = m_networkManager->get(request);
    reply->setProperty("stationImageUrl", remoteUrl);
    reply->setProperty("stationImageRequestUrl", requestUrl);
    reply->setProperty("stationImageRedirectCount", redirectCount);
    connect(reply, &QNetworkReply::readyRead, this, [this, reply]() {
        if (reply->bytesAvailable() > maximumDownloadBytes) {
            reply->setProperty("stationImageRejected", true);
            reply->abort();
        }
    });
    connect(reply, &QNetworkReply::metaDataChanged, this, [reply]() {
        const qint64 contentLength =
            reply->header(QNetworkRequest::ContentLengthHeader).toLongLong();
        if (contentLength > StationImageCache::maximumDownloadBytes) {
            reply->setProperty("stationImageRejected", true);
            reply->abort();
        }
    });
    connect(reply, &QNetworkReply::finished, this, [this, reply]() {
        finishDownload(reply);
    });
}

void StationImageCache::finishDownload(QNetworkReply *reply)
{
    const QString remoteUrl = reply->property("stationImageUrl").toString();

    const QVariant redirect =
        reply->attribute(QNetworkRequest::RedirectionTargetAttribute);
    const int redirectCount =
        reply->property("stationImageRedirectCount").toInt();
    if (redirect.isValid() && redirectCount < maximumRedirects
        && reply->error() == QNetworkReply::NoError) {
        const QUrl requestUrl =
            reply->property("stationImageRequestUrl").toUrl();
        const QUrl target = requestUrl.resolved(redirect.toUrl());
        reply->deleteLater();
        resolveDownload(remoteUrl, target, redirectCount + 1);
        return;
    }

    m_pending.remove(remoteUrl);

    if (reply->error() != QNetworkReply::NoError
        || reply->property("stationImageRejected").toBool()
        || redirect.isValid()) {
        reply->deleteLater();
        reject(remoteUrl);
        return;
    }

    const QString contentType =
        reply->header(QNetworkRequest::ContentTypeHeader).toString()
            .section(QLatin1Char(';'), 0, 0).trimmed().toLower();
    const bool acceptableContentType =
        contentType.isEmpty() || contentType.startsWith(QStringLiteral("image/"))
        || contentType == QStringLiteral("application/octet-stream");
    const QByteArray data = reply->readAll();

    if (data.isEmpty() || data.size() > maximumDownloadBytes
        || !acceptableContentType) {
        reply->deleteLater();
        reject(remoteUrl);
        return;
    }

    QBuffer buffer;
    buffer.setData(data);
    buffer.open(QIODevice::ReadOnly);
    QImageReader reader(&buffer);
    reader.setDecideFormatFromContent(true);

    const QSize decodedSize = reader.size();
    if (!decodedSize.isValid()
        || decodedSize.width() > maximumDimension
        || decodedSize.height() > maximumDimension) {
        reply->deleteLater();
        reject(remoteUrl);
        return;
    }

    QImage image = reader.read();
    if (image.isNull()) {
        reply->deleteLater();
        reject(remoteUrl);
        return;
    }
    if (image.width() > cachedDimension || image.height() > cachedDimension) {
        image = image.scaled(cachedDimension, cachedDimension, Qt::KeepAspectRatio,
                             Qt::SmoothTransformation);
    }

    const QString cachePath = cachePathForUrl(remoteUrl);
    QSaveFile output(cachePath);
    if (!output.open(QIODevice::WriteOnly)
        || !image.save(&output, "PNG")
        || !output.commit()) {
        reply->deleteLater();
        reject(remoteUrl);
        return;
    }

    pruneCacheDirectory(m_cacheDirectory);
    if (!QFile::exists(cachePath)) {
        reply->deleteLater();
        reject(remoteUrl);
        return;
    }
    const QString localSource = localSourceForPath(cachePath);
    m_sources.insert(remoteUrl, localSource);
    reply->deleteLater();
    advanceRevision();
    emit imageReady(remoteUrl, localSource);
}

void StationImageCache::reject(const QString &remoteUrl)
{
    m_blocked.insert(remoteUrl);
    advanceRevision();
    emit imageRejected(remoteUrl);
}

void StationImageCache::advanceRevision()
{
    ++m_revision;
    emit revisionChanged();
}

void StationImageCache::touchCacheFile(const QString &path) const
{
    QFile file(path);
    if (file.open(QIODevice::ReadOnly)) {
        file.setFileTime(QDateTime::currentDateTimeUtc(),
                         QFileDevice::FileModificationTime);
    }
}
