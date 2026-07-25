// Copyright (c) 2026 Sebastian Palencsar
// SPDX-License-Identifier: GPL-3.0-or-later

#include "icyreader.h"
#include "streamurl.h"
#include <QNetworkRequest>
#include <QDebug>
#include <QRegularExpression>

IcyReader::IcyReader(QObject *parent)
    : QObject(parent)
{
    m_nam = new QNetworkAccessManager(this);
}

IcyReader::~IcyReader()
{
    stop();
}

void IcyReader::start(const QString &url, quint64 sourceGeneration)
{
    stop();

    if (url.isEmpty() || !isAllowedStreamUrl(url))
        return;

    QNetworkRequest request((QUrl(url)));
    request.setRawHeader("Icy-MetaData", "1");
    // Some streams require a User-Agent to send ICY metadata
    request.setRawHeader("User-Agent", "VLC/3.0.16 LibVLC/3.0.16");
    // Streams often use 302 redirects to load balancers/relays
    request.setAttribute(QNetworkRequest::RedirectPolicyAttribute, QNetworkRequest::NoLessSafeRedirectPolicy);
    request.setTransferTimeout(15000);

    m_reply = m_nam->get(request);
    m_reply->setProperty("sourceGeneration", QVariant::fromValue(sourceGeneration));

    connect(m_reply, &QNetworkReply::readyRead, this, &IcyReader::onReadyRead);
    connect(m_reply, &QNetworkReply::finished, this, &IcyReader::onFinished);
}

void IcyReader::stop()
{
    if (m_reply) {
        QNetworkReply *reply = m_reply;
        m_reply = nullptr;
        reply->abort();
        reply->deleteLater();
    }
    m_state = ReadingAudio;
    m_metaInt = 0;
    m_audioBytesRead = 0;
    m_metaLength = 0;
}

void IcyReader::onReadyRead()
{
    QNetworkReply *reply = qobject_cast<QNetworkReply*>(sender());
    if (!reply || reply != m_reply)
        return;

    const quint64 sourceGeneration = reply->property("sourceGeneration").toULongLong();

    // Check if we just received headers and parse icy-metaint
    if (m_metaInt == 0) {
        if (reply->hasRawHeader("icy-metaint")) {
            m_metaInt = reply->rawHeader("icy-metaint").toInt();
            qDebug() << "ICYREADER: Found icy-metaint =" << m_metaInt;
        } else {
            // No ICY metadata supported by stream
            qDebug() << "ICYREADER: No icy-metaint header found!";
            return;
        }
    }

    if (m_metaInt <= 0)
        return;

    while (reply == m_reply && reply->bytesAvailable() > 0) {
        if (m_state == ReadingAudio) {
            qint64 bytesToRead = m_metaInt - m_audioBytesRead;
            if (reply->bytesAvailable() >= bytesToRead) {
                reply->read(bytesToRead); // Discard audio data
                m_audioBytesRead = 0;
                m_state = ReadingMetaLength;
            } else {
                m_audioBytesRead += reply->readAll().size();
                break; // Wait for more data
            }
        } else if (m_state == ReadingMetaLength) {
            char lengthByte;
            if (reply->read(&lengthByte, 1) == 1) {
                m_metaLength = static_cast<unsigned char>(lengthByte) * 16;
                if (m_metaLength > 0) {
                    m_state = ReadingMetaData;
                } else {
                    m_state = ReadingAudio;
                }
            }
        } else if (m_state == ReadingMetaData) {
            if (reply->bytesAvailable() >= m_metaLength) {
                QByteArray metaData = reply->read(m_metaLength);
                parseMetaData(metaData, sourceGeneration);
                m_state = ReadingAudio;
            } else {
                break; // Wait for more metadata bytes
            }
        }
    }
}

void IcyReader::onFinished()
{
    QNetworkReply *reply = qobject_cast<QNetworkReply*>(sender());
    if (!reply) {
        return;
    }

    if (reply == m_reply) {
        m_reply = nullptr;
    }
    reply->deleteLater();
}

void IcyReader::parseMetaData(const QByteArray &metaData, quint64 sourceGeneration)
{
    QString metaString = QString::fromUtf8(metaData).trimmed();
    if (metaString.isEmpty())
        return;

    // Format usually is: StreamTitle='Artist - Song';StreamUrl='';
    // Use non-greedy match (.*?) to allow apostrophes in the title
    QRegularExpression re("StreamTitle='(.*?)';");
    QRegularExpressionMatch match = re.match(metaString);
    
    if (match.hasMatch()) {
        QString fullTitle = match.captured(1).trimmed();
        if (fullTitle.isEmpty())
            return;

        QString artist;
        QString title;

        int dashIndex = fullTitle.indexOf(" - ");
        if (dashIndex != -1) {
            artist = fullTitle.left(dashIndex).trimmed();
            title = fullTitle.mid(dashIndex + 3).trimmed();
        } else {
            title = fullTitle;
        }

        qDebug() << "ICYREADER: PARSED METADATA -> Artist:" << artist << "Title:" << title;
        emit metaDataReceived(artist, title, sourceGeneration);
    } else {
        qDebug() << "ICYREADER: Regex failed on string:" << metaString;
    }
}
