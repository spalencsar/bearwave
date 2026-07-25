// Copyright (c) 2026 Sebastian Palencsar
// SPDX-License-Identifier: GPL-3.0-or-later

#include <QtTest>
#include <QCoreApplication>
#include <QSignalSpy>
#include <QTemporaryDir>
#include <QDir>
#include <QFile>
#include <QCryptographicHash>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QEventLoop>
#include <QHostAddress>
#include <QTcpServer>
#include <QTcpSocket>
#include <QTimer>

#include "radiobrowser.h"

class RadioBrowserRaceTest : public QObject
{
    Q_OBJECT

private slots:
    void initTestCase();
    void latest_request_wins_after_rapid_category_switch();
    void transient_http_error_fails_over_to_next_server();
    void non_retryable_http_error_stops_immediately();
    void transient_error_reports_after_three_single_server_attempts();
    void cached_response_suppresses_error_after_retries();
    void api_host_validation_rejects_untrusted_names();
    void retry_policy_recognizes_temporary_http_statuses();
};

namespace {
QTemporaryDir *s_tempHome = nullptr;

class HttpTestServer : public QTcpServer
{
public:
    HttpTestServer(const QList<int> &statuses, const QByteArray &successBody, QObject *parent = nullptr)
        : QTcpServer(parent)
        , m_statuses(statuses)
        , m_successBody(successBody)
    {
        connect(this, &QTcpServer::newConnection, this, [this]() {
            while (hasPendingConnections()) {
                QTcpSocket *socket = nextPendingConnection();
                connect(socket, &QTcpSocket::readyRead, socket, [this, socket]() {
                    if (socket->property("responded").toBool()) {
                        return;
                    }

                    bool requestComplete = false;
                    while (socket->canReadLine()) {
                        if (socket->readLine() == QByteArrayLiteral("\r\n")) {
                            requestComplete = true;
                            break;
                        }
                    }
                    if (!requestComplete) {
                        return;
                    }

                    socket->setProperty("responded", true);
                    ++m_requestCount;

                    const int responseIndex = qMin(m_requestCount - 1, m_statuses.size() - 1);
                    const int status = m_statuses.value(responseIndex, 503);
                    const QByteArray body = status == 200 ? m_successBody : QByteArrayLiteral("temporary failure");
                    const QByteArray reason = status == 200 ? QByteArrayLiteral("OK")
                                              : status == 404 ? QByteArrayLiteral("Not Found")
                                                              : QByteArrayLiteral("Service Unavailable");
                    QByteArray response = QByteArrayLiteral("HTTP/1.1 ") + QByteArray::number(status)
                                          + QByteArrayLiteral(" ") + reason + QByteArrayLiteral("\r\n")
                                          + QByteArrayLiteral("Content-Type: application/json\r\n")
                                          + QByteArrayLiteral("Content-Length: ") + QByteArray::number(body.size())
                                          + QByteArrayLiteral("\r\nConnection: close\r\n\r\n") + body;
                    socket->write(response);
                    socket->disconnectFromHost();
                });
            }
        });
        QVERIFY(listen(QHostAddress::LocalHost));
    }

    QString apiBaseUrl() const
    {
        return QStringLiteral("http://127.0.0.1:%1/json").arg(serverPort());
    }

    int requestCount() const { return m_requestCount; }

private:
    QList<int> m_statuses;
    QByteArray m_successBody;
    int m_requestCount = 0;
};

QString cachePathForEndpoint(const QString &endpoint)
{
    const QString cacheDir = QDir::homePath() + QStringLiteral("/.cache/bearwave/api_cache");
    const QString hash = QString(QCryptographicHash::hash(endpoint.toUtf8(), QCryptographicHash::Md5).toHex());
    return cacheDir + QStringLiteral("/") + hash + QStringLiteral(".json");
}

void writeStationCache(const QString &endpoint, const QString &name, const QString &country)
{
    QDir().mkpath(QDir::homePath() + QStringLiteral("/.cache/bearwave/api_cache"));

    QJsonObject station;
    station.insert(QStringLiteral("name"), name);
    station.insert(QStringLiteral("url_resolved"), QStringLiteral("http://example.com/") + name);
    station.insert(QStringLiteral("country"), country);
    station.insert(QStringLiteral("stationuuid"), name);

    QJsonArray array;
    array.append(station);

    QFile file(cachePathForEndpoint(endpoint));
    QVERIFY(file.open(QIODevice::WriteOnly | QIODevice::Truncate));
    file.write(QJsonDocument(array).toJson());
    file.close();
}

void writeCountryCache()
{
    QDir().mkpath(QDir::homePath() + QStringLiteral("/.cache/bearwave/api_cache"));

    QJsonObject country;
    country.insert(QStringLiteral("name"), QStringLiteral("Germany"));
    country.insert(QStringLiteral("iso_3166_1"), QStringLiteral("DE"));
    country.insert(QStringLiteral("stationcount"), 100);

    QFile file(cachePathForEndpoint(QStringLiteral("/countries")));
    QVERIFY(file.open(QIODevice::WriteOnly | QIODevice::Truncate));
    file.write(QJsonDocument(QJsonArray{country}).toJson());
    file.close();
}

QByteArray stationResponse(const QString &name)
{
    QJsonObject station;
    station.insert(QStringLiteral("name"), name);
    station.insert(QStringLiteral("url_resolved"), QStringLiteral("http://example.com/") + name);
    station.insert(QStringLiteral("country"), QStringLiteral("Testland"));
    station.insert(QStringLiteral("stationuuid"), name);
    return QJsonDocument(QJsonArray{station}).toJson(QJsonDocument::Compact);
}
}

void RadioBrowserRaceTest::initTestCase()
{
    s_tempHome = new QTemporaryDir();
    QVERIFY(s_tempHome->isValid());
    qputenv("HOME", s_tempHome->path().toUtf8());

    writeStationCache(QStringLiteral("/stations/topvote/5"), QStringLiteral("Top Station"), QStringLiteral("Global"));
    writeStationCache(QStringLiteral("/stations/bycountrycodeexact/DE?limit=50&order=votes&reverse=true"),
                      QStringLiteral("DE Station"), QStringLiteral("Germany"));
    writeStationCache(QStringLiteral("/stations/bycountrycodeexact/NL?limit=50&order=votes&reverse=true"),
                      QStringLiteral("NL Station"), QStringLiteral("The Netherlands"));
}

void RadioBrowserRaceTest::latest_request_wins_after_rapid_category_switch()
{
    RadioBrowser browser;
    QSignalSpy stationsSpy(&browser, &RadioBrowser::stationsLoaded);

    browser.getTopStations(5);
    browser.getGermanStations();
    browser.getDutchStations();

    QCOMPARE(stationsSpy.count(), 3);

    const auto lastVariant = stationsSpy.at(2).at(0);
    const QList<RadioStation*> lastStations = lastVariant.value<QList<RadioStation*>>();
    QVERIFY(!lastStations.isEmpty());
    QCOMPARE(lastStations.first()->name(), QStringLiteral("NL Station"));

    QEventLoop loop;
    QTimer::singleShot(2000, &loop, &QEventLoop::quit);
    loop.exec();

    const auto finalVariant = stationsSpy.takeLast().at(0);
    const QList<RadioStation*> finalStations = finalVariant.value<QList<RadioStation*>>();
    QVERIFY(!finalStations.isEmpty());

    const QString finalName = finalStations.first()->name();
    const QString finalCountry = finalStations.first()->country();
    QVERIFY(finalName != QStringLiteral("Top Station"));
    QVERIFY(finalName != QStringLiteral("DE Station"));
    QVERIFY(finalCountry.contains(QStringLiteral("Netherlands"), Qt::CaseInsensitive)
            || finalName == QStringLiteral("NL Station"));
}

void RadioBrowserRaceTest::transient_http_error_fails_over_to_next_server()
{
    HttpTestServer failingServer({503}, stationResponse(QStringLiteral("Unused")));
    HttpTestServer healthyServer({200}, stationResponse(QStringLiteral("Failover Station")));
    RadioBrowser browser({failingServer.apiBaseUrl(), healthyServer.apiBaseUrl()});
    QSignalSpy stationsSpy(&browser, &RadioBrowser::stationsLoaded);
    QSignalSpy errorSpy(&browser, &RadioBrowser::error);

    browser.getTopStations(7);

    QTRY_COMPARE_WITH_TIMEOUT(stationsSpy.count(), 1, 2000);
    QCOMPARE(errorSpy.count(), 0);
    QCOMPARE(failingServer.requestCount(), 1);
    QCOMPARE(healthyServer.requestCount(), 1);

    const QList<RadioStation*> stations = stationsSpy.first().first().value<QList<RadioStation*>>();
    QVERIFY(!stations.isEmpty());
    QCOMPARE(stations.first()->name(), QStringLiteral("Failover Station"));
}

void RadioBrowserRaceTest::non_retryable_http_error_stops_immediately()
{
    HttpTestServer missingServer({404}, stationResponse(QStringLiteral("Unused")));
    HttpTestServer unusedServer({200}, stationResponse(QStringLiteral("Should Not Load")));
    RadioBrowser browser({missingServer.apiBaseUrl(), unusedServer.apiBaseUrl()});
    QSignalSpy stationsSpy(&browser, &RadioBrowser::stationsLoaded);
    QSignalSpy errorSpy(&browser, &RadioBrowser::error);

    browser.getTopStations(8);

    QTRY_COMPARE_WITH_TIMEOUT(errorSpy.count(), 1, 1000);
    QCOMPARE(stationsSpy.count(), 0);
    QCOMPARE(missingServer.requestCount(), 1);
    QCOMPARE(unusedServer.requestCount(), 0);
    QVERIFY(errorSpy.first().first().toString().contains(QStringLiteral("404")));
}

void RadioBrowserRaceTest::transient_error_reports_after_three_single_server_attempts()
{
    HttpTestServer failingServer({503, 503, 503}, stationResponse(QStringLiteral("Unused")));
    RadioBrowser browser({failingServer.apiBaseUrl()});
    QSignalSpy errorSpy(&browser, &RadioBrowser::error);

    browser.getTopStations(10);

    QTRY_COMPARE_WITH_TIMEOUT(errorSpy.count(), 1, 2000);
    QCOMPARE(failingServer.requestCount(), 3);
    QVERIFY(errorSpy.first().first().toString().contains(QStringLiteral("503")));
}

void RadioBrowserRaceTest::cached_response_suppresses_error_after_retries()
{
    writeCountryCache();

    HttpTestServer failingServer({503, 503, 503}, stationResponse(QStringLiteral("Unused")));
    RadioBrowser browser({failingServer.apiBaseUrl()});
    QSignalSpy countriesSpy(&browser, &RadioBrowser::countriesLoaded);
    QSignalSpy errorSpy(&browser, &RadioBrowser::error);

    browser.getCountries();

    QCOMPARE(countriesSpy.count(), 1);
    QTRY_COMPARE_WITH_TIMEOUT(failingServer.requestCount(), 3, 2000);
    QCOMPARE(errorSpy.count(), 0);

    const QVariantList countries = countriesSpy.first().first().toList();
    QVERIFY(!countries.isEmpty());
    QCOMPARE(countries.first().toMap().value(QStringLiteral("code")).toString(), QStringLiteral("DE"));
}

void RadioBrowserRaceTest::api_host_validation_rejects_untrusted_names()
{
    QVERIFY(RadioBrowser::isAllowedApiHost(QStringLiteral("de1.api.radio-browser.info")));
    QVERIFY(RadioBrowser::isAllowedApiHost(QStringLiteral("ALL.API.RADIO-BROWSER.INFO.")));
    QVERIFY(!RadioBrowser::isAllowedApiHost(QStringLiteral("api.radio-browser.info.evil.example")));
    QVERIFY(!RadioBrowser::isAllowedApiHost(QStringLiteral("radio-browser.info")));
    QVERIFY(!RadioBrowser::isAllowedApiHost(QStringLiteral("127.0.0.1")));
}

void RadioBrowserRaceTest::retry_policy_recognizes_temporary_http_statuses()
{
    QVERIFY(RadioBrowser::isTransientHttpStatus(429));
    QVERIFY(RadioBrowser::isTransientHttpStatus(502));
    QVERIFY(RadioBrowser::isTransientHttpStatus(503));
    QVERIFY(RadioBrowser::isTransientHttpStatus(504));
    QVERIFY(!RadioBrowser::isTransientHttpStatus(400));
    QVERIFY(!RadioBrowser::isTransientHttpStatus(401));
    QVERIFY(!RadioBrowser::isTransientHttpStatus(404));
    QVERIFY(!RadioBrowser::isTransientHttpStatus(500));
}

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);
    RadioBrowserRaceTest test;
    return QTest::qExec(&test, argc, argv);
}

#include "radiobrowser_race_test.moc"
