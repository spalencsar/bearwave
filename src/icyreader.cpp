// Copyright (c) 2026 Sebastian Palencsar
// SPDX-License-Identifier: GPL-3.0-or-later

#include "icyreader.h"
#include "streamurl.h"
#include <QNetworkRequest>
#include <QDebug>
#include <QRegularExpression>
#include <QSslError>

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
    // Many Shoutcast/Icecast relays still expect a classic player UA.
    request.setRawHeader("User-Agent", "VLC/3.0.16 LibVLC/3.0.16");
    request.setRawHeader("Accept", "*/*");
    // Streams often use 302 redirects to load balancers/relays
    request.setAttribute(QNetworkRequest::RedirectPolicyAttribute, QNetworkRequest::NoLessSafeRedirectPolicy);
    // HTTP/2 frequently breaks ICY metaint framing; force HTTP/1.1 for metadata.
    request.setAttribute(QNetworkRequest::Http2AllowedAttribute, false);
    request.setTransferTimeout(20000);

    m_reply = m_nam->get(request);
    m_reply->setProperty("sourceGeneration", QVariant::fromValue(sourceGeneration));
    m_headersChecked = false;
    m_metaInt = 0;

    connect(m_reply, &QNetworkReply::readyRead, this, &IcyReader::onReadyRead);
    connect(m_reply, &QNetworkReply::finished, this, &IcyReader::onFinished);
    connect(m_reply, &QNetworkReply::sslErrors, this, [this](const QList<QSslError> &errors) {
        // Log once; do not call ignoreSslErrors() — keep security defaults.
        if (!m_reply) {
            return;
        }
        qWarning() << "ICYREADER: TLS/SSL errors on metadata connection:"
                   << errors;
    });
    connect(m_reply, &QNetworkReply::errorOccurred, this, [this](QNetworkReply::NetworkError code) {
        if (!m_reply || code == QNetworkReply::NoError || code == QNetworkReply::OperationCanceledError) {
            return;
        }
        qWarning() << "ICYREADER: network error" << code << m_reply->errorString();
    });
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
    m_headersChecked = false;
}

bool IcyReader::ensureMetaInt(QNetworkReply *reply)
{
    if (m_headersChecked) {
        return m_metaInt > 0;
    }

    // Wait until HTTP headers are available.
    const QVariant status = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute);
    if (!status.isValid() && !reply->hasRawHeader("icy-metaint")
        && !reply->hasRawHeader("content-type")) {
        return false;
    }

    m_headersChecked = true;

    if (reply->hasRawHeader("icy-metaint")) {
        m_metaInt = reply->rawHeader("icy-metaint").toInt();
        if (m_metaInt > 0) {
            qDebug() << "ICYREADER: Found icy-metaint =" << m_metaInt;
            return true;
        }
    }

    qDebug() << "ICYREADER: No usable icy-metaint (stream sends no track titles)";
    m_metaInt = -1;
    // Stop the secondary download; keep main player audio undisturbed.
    reply->abort();
    return false;
}

void IcyReader::onReadyRead()
{
    QNetworkReply *reply = qobject_cast<QNetworkReply*>(sender());
    if (!reply || reply != m_reply)
        return;

    const quint64 sourceGeneration = reply->property("sourceGeneration").toULongLong();

    if (!ensureMetaInt(reply)) {
        // Headers not ready yet, or stream has no ICY metadata.
        if (m_metaInt < 0) {
            // Drain/abort path — nothing more to parse.
            reply->readAll();
        }
        return;
    }

    while (reply == m_reply && reply->bytesAvailable() > 0) {
        if (m_state == ReadingAudio) {
            qint64 bytesToRead = m_metaInt - m_audioBytesRead;
            if (bytesToRead <= 0) {
                m_metaInt = -1;
                break;
            }
            if (reply->bytesAvailable() >= bytesToRead) {
                reply->read(bytesToRead); // Discard audio data
                m_audioBytesRead = 0;
                m_state = ReadingMetaLength;
            } else {
                m_audioBytesRead += static_cast<int>(reply->readAll().size());
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
            } else {
                break;
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
    // Also accept double quotes used by some relays.
    QRegularExpression re(QStringLiteral("StreamTitle=['\"](.*?)['\"]"),
                          QRegularExpression::DotMatchesEverythingOption);
    QRegularExpressionMatch match = re.match(metaString);

    if (match.hasMatch()) {
        QString fullTitle = match.captured(1).trimmed();
        if (fullTitle.isEmpty())
            return;

        QString artist;
        QString title;

        int dashIndex = fullTitle.indexOf(QStringLiteral(" - "));
        if (dashIndex != -1) {
            artist = fullTitle.left(dashIndex).trimmed();
            title = fullTitle.mid(dashIndex + 3).trimmed();
        } else {
            dashIndex = fullTitle.indexOf(QStringLiteral(" – ")); // en-dash
            if (dashIndex != -1) {
                artist = fullTitle.left(dashIndex).trimmed();
                title = fullTitle.mid(dashIndex + 3).trimmed();
            } else {
                title = fullTitle;
            }
        }

        qDebug() << "ICYREADER: PARSED METADATA -> Artist:" << artist << "Title:" << title;
        emit metaDataReceived(artist, title, sourceGeneration);
    } else {
        qDebug() << "ICYREADER: Regex failed on string:" << metaString;
    }
}
