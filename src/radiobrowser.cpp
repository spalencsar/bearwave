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

void RadioBrowser::makeRequest(const QString &endpoint)
{
    const QString cacheDir = apiCacheDir();
    QDir().mkpath(cacheDir);
    const QString hash = QString(QCryptographicHash::hash(endpoint.toUtf8(), QCryptographicHash::Md5).toHex());
    const QString cachePath = cacheDir + "/" + hash + ".json";

    QFile cacheFile(cachePath);
    if (cacheFile.exists() && cacheFile.open(QIODevice::ReadOnly)) {
        QByteArray cachedData = cacheFile.readAll();
        QList<RadioStation*> stations = parseJsonResponse(cachedData);
        if (!stations.isEmpty()) {
            emit stationsLoaded(stations);
        }
        cacheFile.close();
    }

    QUrl url(m_baseUrl + endpoint);
    QNetworkRequest request;
    request.setUrl(url);
    request.setHeader(QNetworkRequest::UserAgentHeader, "BearWave/1.0");
    request.setTransferTimeout(10000);

    QNetworkReply *reply = m_networkManager->get(request);
    reply->setProperty("cachePath", cachePath);

    connect(reply, &QNetworkReply::finished, this, &RadioBrowser::onReplyFinished);
}

void RadioBrowser::onReplyFinished()
{
    QNetworkReply *reply = qobject_cast<QNetworkReply*>(sender());

    if (!reply) {
        emit error(tr("Network error"));
        return;
    }

    if (reply->error() != QNetworkReply::NoError) {
        emit error(reply->errorString());
        reply->deleteLater();
        return;
    }

    QByteArray data = reply->readAll();
    QList<RadioStation*> stations = parseJsonResponse(data);

    QString cachePath = reply->property("cachePath").toString();
    if (!cachePath.isEmpty() && !stations.isEmpty()) {
        QFile file(cachePath);
        if (file.open(QIODevice::WriteOnly)) {
            file.write(data);
            file.close();
        }
    }

    emit stationsLoaded(stations);
    reply->deleteLater();
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
