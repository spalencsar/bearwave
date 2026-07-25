// Copyright (c) 2026 Sebastian Palencsar
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef RADIOBROWSER_H
#define RADIOBROWSER_H

#include <QObject>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QList>
#include <QSet>
#include <QStringList>
#include <QVariantList>

#include "radiostation.h"

class RadioBrowser : public QObject
{
    Q_OBJECT

public:
    explicit RadioBrowser(QObject *parent = nullptr);
    explicit RadioBrowser(const QStringList &apiBaseUrls, QObject *parent = nullptr);

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
    friend class RadioBrowserRaceTest;

    QNetworkAccessManager *m_networkManager = nullptr;
    QNetworkReply *m_activeReply = nullptr;
    QStringList m_apiBaseUrls;
    QSet<QString> m_attemptedBaseUrls;
    QString m_currentEndpoint;
    QString m_currentCachePath;
    QString m_lastRequestError;
    int m_requestGeneration = 0;
    int m_attemptCount = 0;
    bool m_cacheDelivered = false;

    void makeRequest(const QString &endpoint);
    void startAttempt(int requestGeneration);
    void scheduleRetry(int requestGeneration);
    void finishWithError(const QString &message, int requestGeneration);
    QString nextBaseUrl() const;
    void resolveApiServers();
    bool emitCachedResponse(const QString &endpoint, const QString &cachePath, int requestGeneration);
    bool isCountriesEndpoint(const QString &endpoint) const;
    static bool isAllowedApiHost(QString host);
    static bool isTransientHttpStatus(int status);
    static bool isTransientFailure(QNetworkReply *reply);
    QList<RadioStation*> parseJsonResponse(const QByteArray &jsonData);
    QVariantList parseCountriesJson(const QByteArray &jsonData);
};

#endif
