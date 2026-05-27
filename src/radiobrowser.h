// Copyright (c) 2026 Sebastian Palencsar
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef RADIOBROWSER_H
#define RADIOBROWSER_H

#include <QObject>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QList>

#include "radiostation.h"

class RadioBrowser : public QObject
{
    Q_OBJECT

public:
    explicit RadioBrowser(QObject *parent = nullptr);

    void search(const QString &query);
    void getTopStations(int count = 50);
    void getByCountry(const QString &countryCode);
    void getByTag(const QString &tag);
    void getWorldStations(int count = 200);
    void getGermanStations();
    void getDutchStations();

signals:
    void stationsLoaded(const QList<RadioStation*> &stations);
    void error(const QString &message);

private slots:
    void onReplyFinished();

private:
    QNetworkAccessManager *m_networkManager = nullptr;
    QString m_baseUrl = "https://all.api.radio-browser.info/json";

    void makeRequest(const QString &endpoint);
    QList<RadioStation*> parseJsonResponse(const QByteArray &jsonData);
};

#endif
