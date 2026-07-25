// Copyright (c) 2026 Sebastian Palencsar
// SPDX-License-Identifier: GPL-3.0-or-later

#include "stationartworkservice.h"

#include "stationimagecache.h"

#include <algorithm>
#include <QHostAddress>
#include <QImageReader>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QRegularExpression>
#include <QTimer>

namespace {
constexpr qint64 maximumHomepageBytes = 512 * 1024;
constexpr qint64 maximumManifestBytes = 256 * 1024;
constexpr int maximumRedirects = 5;
constexpr int maximumConcurrentDiscoveries = 4;

QString attribute(const QString &tag, const QString &name)
{
    const QRegularExpression expression(
        QStringLiteral(R"re(\b%1\s*=\s*(?:"([^"]*)"|'([^']*)'|([^\s>]+)))re")
            .arg(QRegularExpression::escape(name)),
        QRegularExpression::CaseInsensitiveOption);
    const QRegularExpressionMatch match = expression.match(tag);
    for (int index = 1; index <= 3; ++index) {
        if (match.hasMatch() && !match.captured(index).isEmpty()) {
            return match.captured(index).trimmed();
        }
    }
    return {};
}

int declaredSize(const QString &tag)
{
    const QString sizes = attribute(tag, QStringLiteral("sizes"));
    const QRegularExpressionMatch match =
        QRegularExpression(QStringLiteral(R"((\d+)\s*x\s*(\d+))"),
                           QRegularExpression::CaseInsensitiveOption)
            .match(sizes);
    return match.hasMatch()
               ? qMin(match.captured(1).toInt(), match.captured(2).toInt())
               : 0;
}

QStringList uniqueUrls(const QStringList &urls)
{
    QStringList result;
    QSet<QString> seen;
    for (const QString &url : urls) {
        if (!url.isEmpty() && !seen.contains(url)) {
            seen.insert(url);
            result.append(url);
        }
    }
    return result;
}
}

StationArtworkService::StationArtworkService(StationImageCache *imageCache,
                                             QObject *parent)
    : StationArtworkService(imageCache, true, parent)
{
}

StationArtworkService::StationArtworkService(StationImageCache *imageCache,
                                             bool allowHttpForTests,
                                             QObject *parent)
    : QObject(parent)
    , m_imageCache(imageCache)
    , m_networkManager(new QNetworkAccessManager(this))
    , m_allowHttpForTests(allowHttpForTests)
{
    Q_ASSERT(m_imageCache);
    connect(m_imageCache, &StationImageCache::imageReady,
            this, &StationArtworkService::handleImageReady);
    connect(m_imageCache, &StationImageCache::imageRejected,
            this, &StationArtworkService::handleImageRejected);
}

QString StationArtworkService::sourceForStation(const QString &stationKey,
                                                const QString &faviconUrl,
                                                const QString &homepageUrl)
{
    const QString key = stationKey.trimmed();
    if (key.isEmpty()) {
        return {};
    }
    StationState &state = m_stations[key];
    if (!state.started) {
        state.started = true;
        state.favicon = faviconUrl.trimmed();
        state.homepage = homepageUrl.trimmed();
        if (!state.favicon.isEmpty()) {
            state.candidates.append({state.favicon, false});
        }
        tryNextCandidate(key);
    }
    return state.source;
}

void StationArtworkService::tryNextCandidate(const QString &stationKey)
{
    auto iterator = m_stations.find(stationKey);
    if (iterator == m_stations.end() || iterator->complete) {
        return;
    }
    if (iterator->nextCandidate < iterator->candidates.size()) {
        requestCandidate(stationKey,
                         iterator->candidates.at(iterator->nextCandidate++));
        return;
    }
    startDiscovery(stationKey);
}

void StationArtworkService::requestCandidate(const QString &stationKey,
                                             const Candidate &candidate)
{
    if (candidate.url.isEmpty()) {
        tryNextCandidate(stationKey);
        return;
    }
    m_imageWaiters[candidate.url].insert(stationKey, candidate.requireSquare);
    const QString source = m_imageCache->sourceForUrl(candidate.url);
    if (source.startsWith(QStringLiteral("file:"))) {
        handleImageReady(candidate.url, source);
    } else if (m_imageCache->isBlocked(candidate.url)) {
        handleImageRejected(candidate.url);
    }
}

void StationArtworkService::handleImageReady(const QString &remoteUrl,
                                             const QString &localUrl)
{
    const auto waiters = m_imageWaiters.take(remoteUrl);
    for (auto iterator = waiters.constBegin(); iterator != waiters.constEnd();
         ++iterator) {
        StationState &state = m_stations[iterator.key()];
        if (state.complete) {
            continue;
        }
        if (logoIsUsable(localUrl, iterator.value())) {
            state.source = localUrl;
            state.complete = true;
            state.discovering = false;
            advanceRevision();
        } else {
            tryNextCandidate(iterator.key());
        }
    }
}

void StationArtworkService::handleImageRejected(const QString &remoteUrl)
{
    const auto waiters = m_imageWaiters.take(remoteUrl);
    for (auto iterator = waiters.constBegin(); iterator != waiters.constEnd();
         ++iterator) {
        tryNextCandidate(iterator.key());
    }
}

void StationArtworkService::startDiscovery(const QString &stationKey)
{
    StationState &state = m_stations[stationKey];
    if (state.discovering || state.complete) {
        return;
    }
    if (state.discoveryAttempted) {
        completeWithoutLogo(stationKey);
        return;
    }
    state.discoveryAttempted = true;
    const QUrl homepage(state.homepage);
    if (!isSafeNetworkUrl(homepage, m_allowHttpForTests)) {
        completeWithoutLogo(stationKey);
        return;
    }
    const QString normalized = homepage.adjusted(QUrl::RemoveFragment).toString();
    if (m_discoveryCache.contains(normalized)) {
        state.candidates = m_discoveryCache.value(normalized);
        state.nextCandidate = 0;
        if (state.candidates.isEmpty()) {
            completeWithoutLogo(stationKey);
        } else {
            tryNextCandidate(stationKey);
        }
        return;
    }
    state.discovering = true;
    m_discoveryWaiters[normalized].insert(stationKey);
    if (m_discoveryWaiters.value(normalized).size() == 1) {
        m_discoveryQueue.enqueue(normalized);
        pumpDiscoveryQueue();
    }
}

void StationArtworkService::pumpDiscoveryQueue()
{
    while (m_activeDiscoveries < maximumConcurrentDiscoveries
           && !m_discoveryQueue.isEmpty()) {
        const QString homepage = m_discoveryQueue.dequeue();
        ++m_activeDiscoveries;
        DiscoveryJob job;
        job.homepage = homepage;
        job.url = QUrl(homepage);
        fetchDiscovery(job);
    }
}

void StationArtworkService::fetchDiscovery(const DiscoveryJob &job)
{
    QNetworkRequest request(job.url);
    request.setTransferTimeout(5000);
    request.setAttribute(QNetworkRequest::RedirectPolicyAttribute,
                         QNetworkRequest::ManualRedirectPolicy);
    request.setHeader(QNetworkRequest::UserAgentHeader,
                      QStringLiteral("BearWave/1.0 station logo discovery"));
    QNetworkReply *reply = m_networkManager->get(request);
    reply->setProperty("discoveryHomepage", job.homepage);
    reply->setProperty("discoveryUrl", job.url);
    reply->setProperty("discoveryPageUrl", job.pageUrl);
    reply->setProperty("discoveryManifestUrl", job.manifestUrl);
    reply->setProperty("discoveryPrimary", job.primaryCandidates);
    reply->setProperty("discoveryOpenGraph", job.openGraphCandidates);
    reply->setProperty("discoveryRedirectCount", job.redirectCount);
    reply->setProperty("discoveryManifestPhase", job.manifestPhase);
    connect(reply, &QNetworkReply::readyRead, this, [reply, job]() {
        const qint64 limit =
            job.manifestPhase ? maximumManifestBytes : maximumHomepageBytes;
        if (reply->bytesAvailable() > limit) {
            reply->setProperty("discoveryTooLarge", true);
            reply->abort();
        }
    });
    connect(reply, &QNetworkReply::finished, this,
            [this, reply]() { finishDiscoveryReply(reply); });
}

void StationArtworkService::finishDiscoveryReply(QNetworkReply *reply)
{
    DiscoveryJob job;
    job.homepage = reply->property("discoveryHomepage").toString();
    job.url = reply->property("discoveryUrl").toUrl();
    job.pageUrl = reply->property("discoveryPageUrl").toUrl();
    job.manifestUrl = reply->property("discoveryManifestUrl").toUrl();
    job.primaryCandidates = reply->property("discoveryPrimary").toStringList();
    job.openGraphCandidates = reply->property("discoveryOpenGraph").toStringList();
    job.redirectCount = reply->property("discoveryRedirectCount").toInt();
    job.manifestPhase = reply->property("discoveryManifestPhase").toBool();

    const QVariant redirect = reply->attribute(QNetworkRequest::RedirectionTargetAttribute);
    if (redirect.isValid() && job.redirectCount < maximumRedirects) {
        const QUrl target = job.url.resolved(redirect.toUrl());
        if (isSafeNetworkUrl(target, m_allowHttpForTests)) {
            job.url = target;
            ++job.redirectCount;
            reply->deleteLater();
            fetchDiscovery(job);
            return;
        }
    }

    const qint64 limit =
        job.manifestPhase ? maximumManifestBytes : maximumHomepageBytes;
    const bool replyFailed =
        reply->error() != QNetworkReply::NoError
        || reply->property("discoveryTooLarge").toBool()
        || !reply->isReadable();
    const QByteArray data = replyFailed ? QByteArray() : reply->readAll();
    const bool failed = replyFailed || data.size() > limit;
    reply->deleteLater();
    if (failed || redirect.isValid()) {
        completeDiscovery(job);
        return;
    }

    if (job.manifestPhase) {
        completeDiscovery(job, manifestCandidates(data, job.url));
        return;
    }

    job.pageUrl = job.url;
    job.primaryCandidates =
        htmlCandidates(data, job.pageUrl, &job.manifestUrl,
                       &job.openGraphCandidates);
    if (isSafeNetworkUrl(job.manifestUrl, m_allowHttpForTests)) {
        job.url = job.manifestUrl;
        job.manifestPhase = true;
        job.redirectCount = 0;
        fetchDiscovery(job);
        return;
    }
    completeDiscovery(job);
}

void StationArtworkService::completeDiscovery(
    const DiscoveryJob &job, const QStringList &manifestIcons)
{
    QList<Candidate> candidates;
    QSet<QString> seen;
    auto append = [&candidates, &seen](const QStringList &urls, bool square) {
        for (const QString &url : urls) {
            if (!url.isEmpty() && !seen.contains(url)) {
                seen.insert(url);
                candidates.append({url, square});
            }
        }
    };

    // htmlCandidates returns Apple icons and declared icons first.
    append(job.primaryCandidates, false);
    append(manifestIcons, false);
    if (job.pageUrl.isValid()) {
        append({job.pageUrl.resolved(QUrl(QStringLiteral("/favicon.ico"))).toString()},
               false);
    }
    append(job.openGraphCandidates, true);
    assignDiscoveryCandidates(job.homepage, candidates);
    --m_activeDiscoveries;
    pumpDiscoveryQueue();
}

void StationArtworkService::assignDiscoveryCandidates(
    const QString &homepage, const QList<Candidate> &candidates)
{
    m_discoveryCache.insert(homepage, candidates);
    const QSet<QString> waiters = m_discoveryWaiters.take(homepage);
    for (const QString &stationKey : waiters) {
        StationState &state = m_stations[stationKey];
        state.discovering = false;
        state.candidates = candidates;
        state.nextCandidate = 0;
        if (candidates.isEmpty()) {
            completeWithoutLogo(stationKey);
        } else {
            tryNextCandidate(stationKey);
        }
    }
}

void StationArtworkService::completeWithoutLogo(const QString &stationKey)
{
    StationState &state = m_stations[stationKey];
    state.complete = true;
    state.discovering = false;
    advanceRevision();
}

void StationArtworkService::advanceRevision()
{
    if (m_revisionUpdatePending) {
        return;
    }
    m_revisionUpdatePending = true;
    QTimer::singleShot(0, this, [this]() {
        m_revisionUpdatePending = false;
        ++m_revision;
        emit revisionChanged();
    });
}

bool StationArtworkService::logoIsUsable(const QString &localUrl,
                                         bool requireSquare)
{
    QImageReader reader(QUrl(localUrl).toLocalFile());
    const QSize size = reader.size();
    if (!size.isValid() || qMin(size.width(), size.height()) < 64) {
        return false;
    }
    if (requireSquare) {
        const qreal ratio = qreal(qMin(size.width(), size.height()))
                            / qreal(qMax(size.width(), size.height()));
        if (ratio < 0.75) {
            return false;
        }
    }
    return reader.canRead();
}

QStringList StationArtworkService::htmlCandidates(
    const QByteArray &html, const QUrl &pageUrl, QUrl *manifestUrl,
    QStringList *openGraphImages)
{
    struct SizedUrl {
        int size;
        QString url;
    };
    QList<SizedUrl> apple;
    QList<SizedUrl> icons;
    const QString document = QString::fromUtf8(html);
    QRegularExpression linkExpression(QStringLiteral(R"(<link\b[^>]*>)"),
                                      QRegularExpression::CaseInsensitiveOption);
    auto links = linkExpression.globalMatch(document);
    while (links.hasNext()) {
        const QString tag = links.next().captured();
        const QString rel = attribute(tag, QStringLiteral("rel")).toLower();
        const QString href = attribute(tag, QStringLiteral("href"));
        if (href.isEmpty()) {
            continue;
        }
        const QString resolved = pageUrl.resolved(QUrl(href)).toString();
        if (rel.contains(QStringLiteral("apple-touch-icon"))) {
            apple.append({declaredSize(tag), resolved});
        } else if (rel.split(QRegularExpression(QStringLiteral("\\s+")))
                       .contains(QStringLiteral("icon"))) {
            icons.append({declaredSize(tag), resolved});
        } else if (manifestUrl && rel.contains(QStringLiteral("manifest"))) {
            *manifestUrl = pageUrl.resolved(QUrl(href));
        }
    }
    auto largerFirst = [](const SizedUrl &left, const SizedUrl &right) {
        return left.size > right.size;
    };
    std::stable_sort(apple.begin(), apple.end(), largerFirst);
    std::stable_sort(icons.begin(), icons.end(), largerFirst);
    QStringList result;
    for (const SizedUrl &candidate : apple) {
        result.append(candidate.url);
    }
    for (const SizedUrl &candidate : icons) {
        result.append(candidate.url);
    }

    if (openGraphImages) {
        QRegularExpression metaExpression(QStringLiteral(R"(<meta\b[^>]*>)"),
                                          QRegularExpression::CaseInsensitiveOption);
        auto metas = metaExpression.globalMatch(document);
        while (metas.hasNext()) {
            const QString tag = metas.next().captured();
            const QString property = attribute(tag, QStringLiteral("property")).toLower();
            if (property == QStringLiteral("og:image")) {
                openGraphImages->append(
                    pageUrl.resolved(QUrl(attribute(tag, QStringLiteral("content"))))
                        .toString());
            }
        }
        *openGraphImages = uniqueUrls(*openGraphImages);
    }
    return uniqueUrls(result);
}

QStringList StationArtworkService::manifestCandidates(
    const QByteArray &json, const QUrl &manifestUrl)
{
    struct SizedUrl {
        int size;
        QString url;
    };
    QList<SizedUrl> icons;
    const QJsonDocument document = QJsonDocument::fromJson(json);
    for (const QJsonValue &value :
         document.object().value(QStringLiteral("icons")).toArray()) {
        const QJsonObject object = value.toObject();
        const QString source = object.value(QStringLiteral("src")).toString();
        if (source.isEmpty()) {
            continue;
        }
        int size = 0;
        const QRegularExpressionMatch match =
            QRegularExpression(QStringLiteral(R"((\d+)\s*x\s*(\d+))"))
                .match(object.value(QStringLiteral("sizes")).toString());
        if (match.hasMatch()) {
            size = qMin(match.captured(1).toInt(), match.captured(2).toInt());
        }
        icons.append({size, manifestUrl.resolved(QUrl(source)).toString()});
    }
    std::stable_sort(icons.begin(), icons.end(),
                     [](const SizedUrl &left, const SizedUrl &right) {
        return left.size > right.size;
    });
    QStringList result;
    for (const SizedUrl &icon : icons) {
        result.append(icon.url);
    }
    return uniqueUrls(result);
}

bool StationArtworkService::isSafeNetworkUrl(const QUrl &url, bool allowHttp)
{
    const QString scheme = url.scheme().toLower();
    if (!url.isValid() || url.host().isEmpty()
        || (scheme != QStringLiteral("https")
            && !(allowHttp && scheme == QStringLiteral("http")))) {
        return false;
    }
    QHostAddress address;
    if (!address.setAddress(url.host())) {
        return true;
    }
    if (address.protocol() == QAbstractSocket::IPv4Protocol) {
        const quint32 ip = address.toIPv4Address();
        return (ip & 0xff000000u) != 0x00000000u
               && (ip & 0xff000000u) != 0x0a000000u
               && (ip & 0xff000000u) != 0x7f000000u
               && (ip & 0xffff0000u) != 0xa9fe0000u
               && (ip & 0xfff00000u) != 0xac100000u
               && (ip & 0xffff0000u) != 0xc0a80000u
               && (ip & 0xf0000000u) != 0xe0000000u;
    }
    return !address.isNull() && address != QHostAddress::LocalHostIPv6
           && !address.isLinkLocal() && !address.isMulticast()
           && (address.toIPv6Address().c[0] & 0xfe) != 0xfc;
}
