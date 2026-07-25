// Copyright (c) 2026 Sebastian Palencsar
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef ICYREADER_H
#define ICYREADER_H

#include <QObject>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QUrl>

class IcyReader : public QObject
{
    Q_OBJECT
public:
    explicit IcyReader(QObject *parent = nullptr);
    ~IcyReader();

    void start(const QString &url, quint64 sourceGeneration);
    void stop();

signals:
    void metaDataReceived(const QString &artist, const QString &title, quint64 sourceGeneration);

private slots:
    void onReadyRead();
    void onFinished();

private:
    void parseMetaData(const QByteArray &metaData, quint64 sourceGeneration);

    QNetworkAccessManager *m_nam = nullptr;
    QNetworkReply *m_reply = nullptr;

    enum State {
        ReadingAudio,
        ReadingMetaLength,
        ReadingMetaData
    };

    State m_state = ReadingAudio;
    int m_metaInt = 0;
    int m_audioBytesRead = 0;
    int m_metaLength = 0;
};

#endif // ICYREADER_H
