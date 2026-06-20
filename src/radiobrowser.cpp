// Copyright (c) 2026 Sebastian Palencsar
// SPDX-License-Identifier: GPL-3.0-or-later

#include "radiobrowser.h"

#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>
#include <QUrl>
#include <QUrlQuery>
#include <QStandardPaths>
#include <QDir>
#include <QFile>
#include <QCryptographicHash>
#include <QDebug>

namespace {
QString apiCacheDir()
{
    return QDir::homePath() + QStringLiteral("/.cache/bearwave/api_cache");
}
}

RadioBrowser::RadioBrowser(QObject *parent)
    : QObject(parent)
{
    m_networkManager = new QNetworkAccessManager(this);
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

void RadioBrowser::emitCachedResponse(const QString &endpoint, const QString &cachePath, int requestGeneration)
{
    if (requestGeneration != m_requestGeneration) {
        return;
    }

    QFile cacheFile(cachePath);
    if (!cacheFile.exists() || !cacheFile.open(QIODevice::ReadOnly)) {
        return;
    }

    const QByteArray cachedData = cacheFile.readAll();
    cacheFile.close();

    if (isCountriesEndpoint(endpoint)) {
        const QVariantList list = parseCountriesJson(cachedData);
        if (!list.isEmpty()) {
            emit countriesLoaded(list);
        }
        return;
    }

    const QList<RadioStation*> stations = parseJsonResponse(cachedData);
    if (!stations.isEmpty()) {
        emit stationsLoaded(stations);
    }
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

    emitCachedResponse(endpoint, cachePath, requestGeneration);

    QUrl url(m_baseUrl + endpoint);
    QNetworkRequest request;
    request.setUrl(url);
    request.setHeader(QNetworkRequest::UserAgentHeader, "BearWave/1.0");
    request.setTransferTimeout(10000);

    QNetworkReply *reply = m_networkManager->get(request);
    reply->setProperty("cachePath", cachePath);
    reply->setProperty("requestGeneration", requestGeneration);
    reply->setProperty("endpoint", endpoint);

    m_activeReply = reply;
    connect(reply, &QNetworkReply::finished, this, &RadioBrowser::onReplyFinished);
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

    if (reply->error() != QNetworkReply::NoError) {
        emit error(reply->errorString());
        reply->deleteLater();
        return;
    }

    const QByteArray data = reply->readAll();
    const QString cachePath = reply->property("cachePath").toString();
    const QString endpoint = reply->property("endpoint").toString();

    if (isCountriesEndpoint(endpoint)) {
        const QVariantList list = parseCountriesJson(data);
        if (!cachePath.isEmpty() && !list.isEmpty()) {
            QFile file(cachePath);
            if (file.open(QIODevice::WriteOnly)) {
                file.write(data);
                file.close();
            }
        }
        emit countriesLoaded(list);
    } else {
        const QList<RadioStation*> stations = parseJsonResponse(data);
        if (!cachePath.isEmpty() && !stations.isEmpty()) {
            QFile file(cachePath);
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
