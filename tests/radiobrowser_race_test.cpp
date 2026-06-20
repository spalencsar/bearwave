// Copyright (c) 2026 Sebastian Palencsar
// SPDX-License-Identifier: GPL-3.0-or-later

#include <QtTest>
#include <QApplication>
#include <QSignalSpy>
#include <QTemporaryDir>
#include <QDir>
#include <QFile>
#include <QCryptographicHash>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QEventLoop>
#include <QTimer>

#include "radiobrowser.h"

class RadioBrowserRaceTest : public QObject
{
    Q_OBJECT

private slots:
    void initTestCase();
    void latest_request_wins_after_rapid_category_switch();
};

namespace {
QTemporaryDir *s_tempHome = nullptr;

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

QTEST_MAIN(RadioBrowserRaceTest)
#include "radiobrowser_race_test.moc"