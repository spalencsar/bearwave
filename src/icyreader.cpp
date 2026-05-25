#include "icyreader.h"
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

void IcyReader::start(const QString &url)
{
    stop();

    if (url.isEmpty())
        return;

    QNetworkRequest request((QUrl(url)));
    request.setRawHeader("Icy-MetaData", "1");
    // Some streams require a User-Agent to send ICY metadata
    request.setRawHeader("User-Agent", "VLC/3.0.16 LibVLC/3.0.16");

    m_reply = m_nam->get(request);

    connect(m_reply, &QNetworkReply::readyRead, this, &IcyReader::onReadyRead);
    connect(m_reply, &QNetworkReply::finished, this, &IcyReader::onFinished);
}

void IcyReader::stop()
{
    if (m_reply) {
        m_reply->abort();
        m_reply->deleteLater();
        m_reply = nullptr;
    }
    m_state = ReadingAudio;
    m_metaInt = 0;
    m_audioBytesRead = 0;
    m_metaLength = 0;
}

void IcyReader::onReadyRead()
{
    if (!m_reply)
        return;

    // Check if we just received headers and parse icy-metaint
    if (m_metaInt == 0) {
        if (m_reply->hasRawHeader("icy-metaint")) {
            m_metaInt = m_reply->rawHeader("icy-metaint").toInt();
        } else {
            // No ICY metadata supported by stream
            return;
        }
    }

    if (m_metaInt <= 0)
        return;

    while (m_reply && m_reply->bytesAvailable() > 0) {
        if (m_state == ReadingAudio) {
            qint64 bytesToRead = m_metaInt - m_audioBytesRead;
            if (m_reply->bytesAvailable() >= bytesToRead) {
                m_reply->read(bytesToRead); // Discard audio data
                m_audioBytesRead = 0;
                m_state = ReadingMetaLength;
            } else {
                m_audioBytesRead += m_reply->readAll().size();
                break; // Wait for more data
            }
        } else if (m_state == ReadingMetaLength) {
            char lengthByte;
            if (m_reply->read(&lengthByte, 1) == 1) {
                m_metaLength = static_cast<unsigned char>(lengthByte) * 16;
                if (m_metaLength > 0) {
                    m_state = ReadingMetaData;
                } else {
                    m_state = ReadingAudio;
                }
            }
        } else if (m_state == ReadingMetaData) {
            if (m_reply->bytesAvailable() >= m_metaLength) {
                QByteArray metaData = m_reply->read(m_metaLength);
                parseMetaData(metaData);
                m_state = ReadingAudio;
            } else {
                break; // Wait for more metadata bytes
            }
        }
    }
}

void IcyReader::onFinished()
{
    if (m_reply) {
        m_reply->deleteLater();
        m_reply = nullptr;
    }
}

void IcyReader::parseMetaData(const QByteArray &metaData)
{
    QString metaString = QString::fromUtf8(metaData).trimmed();
    if (metaString.isEmpty())
        return;

    // Format usually is: StreamTitle='Artist - Song';StreamUrl='';
    QRegularExpression re("StreamTitle='([^']*)';");
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

        emit metaDataReceived(artist, title);
    }
}
