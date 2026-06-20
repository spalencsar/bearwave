// Copyright (c) 2026 Sebastian Palencsar
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef RADIOBROWSER_H
#define RADIOBROWSER_H

#include <QObject>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QList>
#include <QVariantList>

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
    void getCountries();

signals:
    void stationsLoaded(const QList<RadioStation*> &stations);
    void countriesLoaded(const QVariantList &countries);
    void error(const QString &message);

private slots:
    void onReplyFinished();

private:
    QNetworkAccessManager *m_networkManager = nullptr;
    QNetworkReply *m_activeReply = nullptr;
    QString m_baseUrl = "https://all.api.radio-browser.info/json";
    int m_requestGeneration = 0;

    void makeRequest(const QString &endpoint);
    void emitCachedResponse(const QString &endpoint, const QString &cachePath, int requestGeneration);
    bool isCountriesEndpoint(const QString &endpoint) const;
    QList<RadioStation*> parseJsonResponse(const QByteArray &jsonData);
    QVariantList parseCountriesJson(const QByteArray &jsonData);
};

#endif
