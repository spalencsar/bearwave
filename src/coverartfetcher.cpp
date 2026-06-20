// Copyright (c) 2026 Sebastian Palencsar
// SPDX-License-Identifier: GPL-3.0-or-later

#include "coverartfetcher.h"
#include <QUrlQuery>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QDebug>

CoverArtFetcher::CoverArtFetcher(QObject *parent)
    : QObject(parent)
{
    m_networkManager = new QNetworkAccessManager(this);
}

void CoverArtFetcher::fetch(const QString &artist, const QString &title)
{
    if (m_currentReply) {
        QNetworkReply *reply = m_currentReply;
        m_currentReply = nullptr;
        reply->abort();
        reply->deleteLater();
    }

    if (artist.isEmpty() && title.isEmpty()) {
        emit coverUrlReady(QString());
        return;
    }

    QString term = artist;
    if (!title.isEmpty()) {
        if (!term.isEmpty()) term += " ";
        term += title;
    }

    QUrl url("https://itunes.apple.com/search");
    QUrlQuery query;
    query.addQueryItem("term", term);
    query.addQueryItem("entity", "song");
    query.addQueryItem("limit", "1");
    url.setQuery(query);

    QNetworkRequest request(url);
    request.setTransferTimeout(10000);
    m_currentReply = m_networkManager->get(request);
    connect(m_currentReply, &QNetworkReply::finished, this, &CoverArtFetcher::onReplyFinished);
}

void CoverArtFetcher::onReplyFinished()
{
    QNetworkReply *reply = qobject_cast<QNetworkReply*>(sender());
    if (!reply) return;

    if (reply == m_currentReply) {
        m_currentReply = nullptr;
    }

    QString coverUrl;
    if (reply->error() == QNetworkReply::NoError) {
        QByteArray data = reply->readAll();
        QJsonDocument doc = QJsonDocument::fromJson(data);
        if (doc.isObject()) {
            QJsonObject obj = doc.object();
            if (obj.value("resultCount").toInt() > 0) {
                QJsonArray results = obj.value("results").toArray();
                if (!results.isEmpty()) {
                    QJsonObject firstResult = results.first().toObject();
                    QString url = firstResult.value("artworkUrl100").toString();
                    if (!url.isEmpty()) {
                        // Replace 100x100bb with a higher resolution
                        url.replace("100x100bb", "600x600bb");
                        coverUrl = url;
                    }
                }
            }
        }
    } else {
        if (reply->error() != QNetworkReply::OperationCanceledError) {
            qDebug() << "CoverArtFetcher error:" << reply->errorString();
        }
    }

    reply->deleteLater();
    emit coverUrlReady(coverUrl);
}
