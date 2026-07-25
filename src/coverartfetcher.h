// Copyright (c) 2026 Sebastian Palencsar
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef COVERARTFETCHER_H
#define COVERARTFETCHER_H

#include <QObject>
#include <QNetworkAccessManager>
#include <QNetworkReply>

class CoverArtFetcher : public QObject
{
    Q_OBJECT

public:
    explicit CoverArtFetcher(QObject *parent = nullptr);

    void fetch(const QString &artist, const QString &title, quint64 sourceGeneration);
    void cancel();

signals:
    void coverUrlReady(const QString &url, quint64 sourceGeneration);

private slots:
    void onReplyFinished();

private:
    QNetworkAccessManager *m_networkManager;
    QNetworkReply *m_currentReply = nullptr;
};

#endif // COVERARTFETCHER_H
