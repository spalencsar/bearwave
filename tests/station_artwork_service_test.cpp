// Copyright (c) 2026 Sebastian Palencsar
// SPDX-License-Identifier: GPL-3.0-or-later

#include <QtTest>

#include <QHostAddress>
#include <QUrl>

#include "stationartworkservice.h"
#include "stationlogostyle.h"

class StationArtworkServiceTest : public QObject
{
    Q_OBJECT

private slots:
    void createsDeterministicInitialsAndPalette();
    void parsesHomepageCandidatesInQualityOrder();
    void parsesManifestAndResolvesRelativeUrls();
    void rejectsLiteralPrivateNetworkTargets();
    void toleratesMalformedDocuments();
};

void StationArtworkServiceTest::createsDeterministicInitialsAndPalette()
{
    QCOMPARE(StationLogoStyle::initials(QStringLiteral("Radio Paradise")),
             QStringLiteral("RP"));
    QCOMPARE(StationLogoStyle::initials(QStringLiteral("  NPO  ")),
             QStringLiteral("NP"));
    QCOMPARE(StationLogoStyle::initials(QString()), QStringLiteral("♫"));
    QCOMPARE(StationLogoStyle::paletteIndex(QStringLiteral("station-1"), 6),
             StationLogoStyle::paletteIndex(QStringLiteral("station-1"), 6));
    QVERIFY(StationLogoStyle::paletteIndex(QStringLiteral("station-1"), 6) >= 0);
    QVERIFY(StationLogoStyle::paletteIndex(QStringLiteral("station-1"), 6) < 6);
}

void StationArtworkServiceTest::parsesHomepageCandidatesInQualityOrder()
{
    const QByteArray html = R"(
        <html><head>
        <link rel="icon" sizes="32x32" href="/small.png">
        <link href="touch-180.png" rel="apple-touch-icon" sizes="180x180">
        <link rel="icon" sizes="256x256" href="/large.png">
        <link rel="manifest" href="/app/site.webmanifest">
        <meta content="images/social.jpg" property="og:image">
        </head></html>)";
    QUrl manifest;
    QStringList openGraph;
    const QStringList candidates = StationArtworkService::htmlCandidates(
        html, QUrl(QStringLiteral("https://radio.example/path/")), &manifest,
        &openGraph);

    QCOMPARE(candidates,
             QStringList({QStringLiteral("https://radio.example/path/touch-180.png"),
                          QStringLiteral("https://radio.example/large.png"),
                          QStringLiteral("https://radio.example/small.png")}));
    QCOMPARE(manifest,
             QUrl(QStringLiteral("https://radio.example/app/site.webmanifest")));
    QCOMPARE(openGraph,
             QStringList({QStringLiteral(
                 "https://radio.example/path/images/social.jpg")}));
}

void StationArtworkServiceTest::parsesManifestAndResolvesRelativeUrls()
{
    const QByteArray json = R"({
        "icons": [
            {"src": "small.png", "sizes": "64x64"},
            {"src": "../large.png", "sizes": "512x512"},
            {"src": "", "sizes": "1024x1024"}
        ]
    })";
    QCOMPARE(StationArtworkService::manifestCandidates(
                 json,
                 QUrl(QStringLiteral("https://radio.example/app/manifest.json"))),
             QStringList({QStringLiteral("https://radio.example/large.png"),
                          QStringLiteral("https://radio.example/app/small.png")}));
}

void StationArtworkServiceTest::rejectsLiteralPrivateNetworkTargets()
{
    QVERIFY(StationArtworkService::isSafeNetworkUrl(
        QUrl(QStringLiteral("https://radio.example/logo")), false));
    QVERIFY(!StationArtworkService::isSafeNetworkUrl(
        QUrl(QStringLiteral("http://radio.example/logo")), false));
    QVERIFY(StationArtworkService::isSafeNetworkUrl(
        QUrl(QStringLiteral("http://radio.example/logo")), true));
    QVERIFY(!StationArtworkService::isSafeNetworkUrl(
        QUrl(QStringLiteral("https://127.0.0.1/logo")), false));
    QVERIFY(!StationArtworkService::isSafeNetworkUrl(
        QUrl(QStringLiteral("https://192.168.1.20/logo")), false));
    QVERIFY(!StationArtworkService::isSafeNetworkUrl(
        QUrl(QStringLiteral("https://[::1]/logo")), false));
    QVERIFY(!StationArtworkService::isSafeNetworkUrl(
        QUrl(QStringLiteral("http://127.0.0.1/logo")), true));
}

void StationArtworkServiceTest::toleratesMalformedDocuments()
{
    QUrl manifest;
    QStringList openGraph;
    QVERIFY(StationArtworkService::htmlCandidates(
                QByteArrayLiteral("<link rel='icon'><meta property='og:image'>"),
                QUrl(QStringLiteral("https://radio.example/")), &manifest,
                &openGraph)
                .isEmpty());
    QVERIFY(StationArtworkService::manifestCandidates(
                QByteArrayLiteral("{not json"),
                QUrl(QStringLiteral("https://radio.example/manifest.json")))
                .isEmpty());
}

QTEST_GUILESS_MAIN(StationArtworkServiceTest)
#include "station_artwork_service_test.moc"
