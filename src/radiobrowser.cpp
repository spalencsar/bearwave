// Copyright (c) 2026 Sebastian Palencsar
// SPDX-License-Identifier: GPL-3.0-or-later

#include "radiobrowser.h"

#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>
#include <QUrl>
#include <QUrlQuery>
#include "bearwavepaths.h"
#include <QDir>
#include <QFile>
#include <QCryptographicHash>
#include <QDateTime>
#include <QDebug>
#include <QHash>
#include <QHostInfo>
#include <QTimer>

namespace {
const QString kDefaultApiBaseUrl = QStringLiteral("https://all.api.radio-browser.info/json");
constexpr int kMaxAttempts = 3;
constexpr int kRetryDelayMs = 150;
constexpr qint64 kFailedNodeCooldownMs = 60 * 1000;

QHash<QString, qint64> s_failedNodeUntil;

QString apiCacheDir()
{
    return BearwavePaths::apiCacheDir();
}

QString httpErrorMessage(QNetworkReply *reply)
{
    const int status = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
    if (status > 0) {
        return RadioBrowser::tr("HTTP response %1").arg(status);
    }
    return reply->errorString();
}
}

RadioBrowser::RadioBrowser(QObject *parent)
    : RadioBrowser(QStringList{kDefaultApiBaseUrl}, parent)
{
    resolveApiServers();
}

RadioBrowser::RadioBrowser(const QStringList &apiBaseUrls, QObject *parent)
    : QObject(parent)
{
    m_networkManager = new QNetworkAccessManager(this);
    m_apiBaseUrls = apiBaseUrls;
    if (m_apiBaseUrls.isEmpty()) {
        m_apiBaseUrls.append(kDefaultApiBaseUrl);
    }
}

void RadioBrowser::search(const QString &query)
{
    QUrlQuery queryParams;
    queryParams.addQueryItem("name", query);
    queryParams.addQueryItem("hidebroken", "true");
    queryParams.addQueryItem("limit", "50");
    queryParams.addQueryItem("order", "votes");
    queryParams.addQueryItem("reverse", "true");

    QString endpoint = "/stations/search?" + queryParams.toString(QUrl::FullyEncoded);
    qDebug() << "RadioBrowser: Searching with endpoint:" << endpoint;
    makeRequest(endpoint);
}

void RadioBrowser::getTopStations(int count)
{
    QString endpoint = "/stations/topvote/" + QString::number(count);
    makeRequest(endpoint);
}

void RadioBrowser::getByCountry(const QString &countryCode)
{
    QString endpoint = "/stations/bycountrycodeexact/" + countryCode;
    makeRequest(endpoint);
}

void RadioBrowser::getByTag(const QString &tag)
{
    QString endpoint = "/stations/bytag/" + QUrl::toPercentEncoding(tag);
    makeRequest(endpoint);
}

void RadioBrowser::getWorldStations(int count)
{
    QString endpoint = "/stations?hidebroken=true&limit=" + QString::number(count) + "&order=votes&reverse=true";
    makeRequest(endpoint);
}

void RadioBrowser::getGermanStations()
{
    QString endpoint = "/stations/bycountrycodeexact/DE?limit=50&order=votes&reverse=true";
    makeRequest(endpoint);
}

void RadioBrowser::getDutchStations()
{
    QString endpoint = "/stations/bycountrycodeexact/NL?limit=50&order=votes&reverse=true";
    makeRequest(endpoint);
}

void RadioBrowser::getCountries()
{
    makeRequest("/countries");
}

bool RadioBrowser::isCountriesEndpoint(const QString &endpoint) const
{
    return endpoint == QStringLiteral("/countries");
}

bool RadioBrowser::emitCachedResponse(const QString &endpoint, const QString &cachePath, int requestGeneration)
{
    if (requestGeneration != m_requestGeneration) {
        return false;
    }

    QFile cacheFile(cachePath);
    if (!cacheFile.exists() || !cacheFile.open(QIODevice::ReadOnly)) {
        return false;
    }

    const QByteArray cachedData = cacheFile.readAll();
    cacheFile.close();

    if (isCountriesEndpoint(endpoint)) {
        const QVariantList list = parseCountriesJson(cachedData);
        if (!list.isEmpty()) {
            emit countriesLoaded(list);
            return true;
        }
        return false;
    }

    const QList<RadioStation*> stations = parseJsonResponse(cachedData);
    if (!stations.isEmpty()) {
        emit stationsLoaded(stations);
        return true;
    }
    return false;
}

void RadioBrowser::makeRequest(const QString &endpoint)
{
    ++m_requestGeneration;
    const int requestGeneration = m_requestGeneration;

    if (m_activeReply) {
        QNetworkReply *reply = m_activeReply;
        m_activeReply = nullptr;
        reply->abort();
        reply->deleteLater();
    }

    const QString cacheDir = apiCacheDir();
    QDir().mkpath(cacheDir);
    const QString hash = QString(QCryptographicHash::hash(endpoint.toUtf8(), QCryptographicHash::Md5).toHex());
    const QString cachePath = cacheDir + "/" + hash + ".json";

    m_currentEndpoint = endpoint;
    m_currentCachePath = cachePath;
    m_lastRequestError.clear();
    m_attemptedBaseUrls.clear();
    m_attemptCount = 0;
    m_cacheDelivered = emitCachedResponse(endpoint, cachePath, requestGeneration);

    startAttempt(requestGeneration);
}

void RadioBrowser::startAttempt(int requestGeneration)
{
    if (requestGeneration != m_requestGeneration) {
        return;
    }
    if (m_attemptCount >= kMaxAttempts) {
        finishWithError(m_lastRequestError, requestGeneration);
        return;
    }

    const QString baseUrl = nextBaseUrl();
    if (baseUrl.isEmpty()) {
        finishWithError(m_lastRequestError.isEmpty() ? tr("No Radio Browser server available")
                                                     : m_lastRequestError,
                        requestGeneration);
        return;
    }

    ++m_attemptCount;
    m_attemptedBaseUrls.insert(baseUrl);

    QUrl url(baseUrl + m_currentEndpoint);
    QNetworkRequest request;
    request.setUrl(url);
    // Radio Browser asks for a meaningful User-Agent (app + version).
    request.setHeader(QNetworkRequest::UserAgentHeader,
                      QStringLiteral("BearWave/%1").arg(QStringLiteral(BEARWAVE_VERSION)));
    request.setTransferTimeout(10000);

    QNetworkReply *reply = m_networkManager->get(request);
    reply->setProperty("requestGeneration", requestGeneration);
    reply->setProperty("baseUrl", baseUrl);

    m_activeReply = reply;
    connect(reply, &QNetworkReply::finished, this, &RadioBrowser::onReplyFinished);
}

void RadioBrowser::scheduleRetry(int requestGeneration)
{
    QTimer::singleShot(kRetryDelayMs, this, [this, requestGeneration]() {
        startAttempt(requestGeneration);
    });
}

void RadioBrowser::finishWithError(const QString &message, int requestGeneration)
{
    if (requestGeneration != m_requestGeneration || m_cacheDelivered) {
        return;
    }
    emit error(message.isEmpty() ? tr("Network error") : message);
}

QString RadioBrowser::nextBaseUrl() const
{
    const qint64 now = QDateTime::currentMSecsSinceEpoch();

    for (const QString &baseUrl : m_apiBaseUrls) {
        if (!m_attemptedBaseUrls.contains(baseUrl)
            && s_failedNodeUntil.value(baseUrl, 0) <= now) {
            return baseUrl;
        }
    }

    for (const QString &baseUrl : m_apiBaseUrls) {
        if (s_failedNodeUntil.value(baseUrl, 0) <= now) {
            return baseUrl;
        }
    }

    if (m_apiBaseUrls.size() == 1) {
        return m_apiBaseUrls.first();
    }
    return QString();
}

void RadioBrowser::resolveApiServers()
{
    QHostInfo::lookupHost(QStringLiteral("all.api.radio-browser.info"), this,
                          [this](const QHostInfo &forwardInfo) {
        if (forwardInfo.error() != QHostInfo::NoError) {
            return;
        }

        for (const QHostAddress &address : forwardInfo.addresses()) {
            QHostInfo::lookupHost(address.toString(), this, [this](const QHostInfo &reverseInfo) {
                if (reverseInfo.error() != QHostInfo::NoError) {
                    return;
                }

                QString host = reverseInfo.hostName().trimmed().toLower();
                if (host.endsWith(QLatin1Char('.'))) {
                    host.chop(1);
                }
                if (!isAllowedApiHost(host)) {
                    return;
                }

                const QString baseUrl = QStringLiteral("https://") + host + QStringLiteral("/json");
                if (!m_apiBaseUrls.contains(baseUrl)) {
                    m_apiBaseUrls.prepend(baseUrl);
                }
            });
        }
    });
}

bool RadioBrowser::isAllowedApiHost(QString host)
{
    host = host.trimmed().toLower();
    if (host.endsWith(QLatin1Char('.'))) {
        host.chop(1);
    }
    return host == QStringLiteral("all.api.radio-browser.info")
           || (host.endsWith(QStringLiteral(".api.radio-browser.info"))
               && host.size() > QStringLiteral(".api.radio-browser.info").size());
}

bool RadioBrowser::isTransientFailure(QNetworkReply *reply)
{
    const int status = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
    if (isTransientHttpStatus(status)) {
        return true;
    }

    switch (reply->error()) {
    case QNetworkReply::ConnectionRefusedError:
    case QNetworkReply::RemoteHostClosedError:
    case QNetworkReply::HostNotFoundError:
    case QNetworkReply::TimeoutError:
    case QNetworkReply::TemporaryNetworkFailureError:
    case QNetworkReply::NetworkSessionFailedError:
    case QNetworkReply::ProxyConnectionRefusedError:
    case QNetworkReply::ProxyConnectionClosedError:
    case QNetworkReply::ProxyNotFoundError:
    case QNetworkReply::ProxyTimeoutError:
    case QNetworkReply::ServiceUnavailableError:
        return true;
    default:
        return false;
    }
}

bool RadioBrowser::isTransientHttpStatus(int status)
{
    return status == 429 || status == 502 || status == 503 || status == 504;
}

void RadioBrowser::onReplyFinished()
{
    QNetworkReply *reply = qobject_cast<QNetworkReply*>(sender());

    if (!reply) {
        emit error(tr("Network error"));
        return;
    }

    if (reply == m_activeReply) {
        m_activeReply = nullptr;
    }

    const int requestGeneration = reply->property("requestGeneration").toInt();
    if (requestGeneration != m_requestGeneration) {
        reply->deleteLater();
        return;
    }

    if (reply->error() == QNetworkReply::OperationCanceledError) {
        reply->deleteLater();
        return;
    }

    const QString baseUrl = reply->property("baseUrl").toString();
    if (isTransientFailure(reply)) {
        m_lastRequestError = httpErrorMessage(reply);
        s_failedNodeUntil.insert(baseUrl, QDateTime::currentMSecsSinceEpoch() + kFailedNodeCooldownMs);
        reply->deleteLater();
        scheduleRetry(requestGeneration);
        return;
    }

    if (reply->error() != QNetworkReply::NoError) {
        finishWithError(httpErrorMessage(reply), requestGeneration);
        reply->deleteLater();
        return;
    }

    s_failedNodeUntil.remove(baseUrl);
    const QByteArray data = reply->readAll();

    if (isCountriesEndpoint(m_currentEndpoint)) {
        const QVariantList list = parseCountriesJson(data);
        if (!m_currentCachePath.isEmpty() && !list.isEmpty()) {
            QFile file(m_currentCachePath);
            if (file.open(QIODevice::WriteOnly)) {
                file.write(data);
                file.close();
            }
        }
        emit countriesLoaded(list);
    } else {
        const QList<RadioStation*> stations = parseJsonResponse(data);
        if (!m_currentCachePath.isEmpty() && !stations.isEmpty()) {
            QFile file(m_currentCachePath);
            if (file.open(QIODevice::WriteOnly)) {
                file.write(data);
                file.close();
            }
        }
        emit stationsLoaded(stations);
    }
    reply->deleteLater();
}

QVariantList RadioBrowser::parseCountriesJson(const QByteArray &jsonData)
{
    QVariantList countries;
    QJsonDocument doc = QJsonDocument::fromJson(jsonData);
    if (!doc.isArray()) {
        return countries;
    }

    QJsonArray array = doc.array();
    for (const QJsonValue &value : array) {
        if (value.isObject()) {
            QJsonObject obj = value.toObject();
            QString name = obj.value("name").toString();
            QString code = obj.value("iso_3166_1").toString();
            int count = obj.value("stationcount").toInt();

            if (name.isEmpty() || code.isEmpty() || count <= 0) {
                continue;
            }

            QVariantMap map;
            map["name"] = name;
            map["code"] = code;
            map["count"] = count;
            countries.append(map);
        }
    }

    return countries;
}

QList<RadioStation*> RadioBrowser::parseJsonResponse(const QByteArray &jsonData)
{
    QList<RadioStation*> stations;

    QJsonDocument doc = QJsonDocument::fromJson(jsonData);
    if (!doc.isArray()) {
        return stations;
    }

    QJsonArray array = doc.array();
    for (const QJsonValue &value : array) {
        if (value.isObject()) {
            QJsonObject obj = value.toObject();
            QString name = obj.value("name").toString();
            QString url = obj.value("url_resolved").toString();

            if (name.isEmpty() || url.isEmpty()) {
                continue;
            }

            RadioStation *station = RadioStation::fromJson(obj);
            stations.append(station);
        }
    }

    return stations;
}
