// Copyright (c) 2026 Sebastian Palencsar
// SPDX-License-Identifier: GPL-3.0-or-later

#include "stationimagecache.h"

#include <algorithm>
#include <QBuffer>
#include <QCryptographicHash>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QImage>
#include <QImageReader>
#include <QHostAddress>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QSaveFile>
#include <QDateTime>
#include <QUrl>

namespace {
const QString fallbackSource = QStringLiteral("qrc:/assets/app/bearwave.svg");

QString defaultCacheDirectory()
{
    return QDir::homePath() + QStringLiteral("/.cache/bearwave/covers");
}

bool isPrivateLiteralHost(const QString &host)
{
    QHostAddress address;
    if (!address.setAddress(host)) {
        return false;
    }
    if (address.protocol() == QAbstractSocket::IPv4Protocol) {
        const quint32 ip = address.toIPv4Address();
        return (ip & 0xff000000u) == 0x00000000u
               || (ip & 0xff000000u) == 0x0a000000u
               || (ip & 0xff000000u) == 0x7f000000u
               || (ip & 0xffff0000u) == 0xa9fe0000u
               || (ip & 0xfff00000u) == 0xac100000u
               || (ip & 0xffff0000u) == 0xc0a80000u
               || (ip & 0xf0000000u) == 0xe0000000u;
    }
    return address.isNull() || address == QHostAddress::LocalHostIPv6
           || address.isLinkLocal() || address.isMulticast()
           || (address.toIPv6Address().c[0] & 0xfe) == 0xfc;
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
    QDir().mkpath(m_cacheDirectory);
    pruneCacheDirectory(m_cacheDirectory);
}

QString StationImageCache::sourceForUrl(const QString &remoteUrl)
{
    const QString normalizedUrl = remoteUrl.trimmed();
    if (!isAllowedRemoteUrl(normalizedUrl) || m_blocked.contains(normalizedUrl)) {
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

bool StationImageCache::isAllowedRemoteUrl(const QString &remoteUrl) const
{
    const QUrl url(remoteUrl);
    if (!url.isValid() || url.host().isEmpty()) {
        return false;
    }
    const QString scheme = url.scheme().toLower();
    if (scheme != QStringLiteral("https") && scheme != QStringLiteral("http")) {
        return false;
    }
    return !isPrivateLiteralHost(url.host())
           || (m_allowHttpForTests && scheme == QStringLiteral("http"));
}

void StationImageCache::startDownload(const QString &remoteUrl)
{
    m_pending.insert(remoteUrl);

    QNetworkRequest request{QUrl(remoteUrl)};
    request.setTransferTimeout(10000);
    request.setAttribute(QNetworkRequest::RedirectPolicyAttribute,
                         QNetworkRequest::UserVerifiedRedirectPolicy);
    request.setHeader(QNetworkRequest::UserAgentHeader,
                      QStringLiteral("BearWave/1.0 station image cache"));

    QNetworkReply *reply = m_networkManager->get(request);
    reply->setProperty("stationImageUrl", remoteUrl);
    connect(reply, &QNetworkReply::redirected, this,
            [this, reply](const QUrl &target) {
        if (isAllowedRemoteUrl(target.toString())) {
            reply->redirectAllowed();
        } else {
            reply->setProperty("stationImageRejected", true);
            reply->abort();
        }
    });
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
    m_pending.remove(remoteUrl);

    if (reply->error() != QNetworkReply::NoError
        || reply->property("stationImageRejected").toBool()) {
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
