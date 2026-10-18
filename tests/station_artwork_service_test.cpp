// Copyright (c) 2026 Sebastian Palencsar
// SPDX-License-Identifier: GPL-3.0-or-later

#include <QtTest>

#include <QHostAddress>
#include <QNetworkRequest>
#include <QUrl>

#include "stationartworkservice.h"
#include "stationlogostyle.h"
#include "networkurlguard.h"

class StationArtworkServiceTest : public QObject
{
    Q_OBJECT

private slots:
    void createsDeterministicInitialsAndPalette();
    void parsesHomepageCandidatesInQualityOrder();
    void parsesManifestAndResolvesRelativeUrls();
    void rejectsPrivateAndSpecialNetworkTargets();
    void rejectsHostnameResolvingToLoopback();
    void pinsRequestsToResolvedAddress();
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

void StationArtworkServiceTest::rejectsPrivateAndSpecialNetworkTargets()
{
    const QUrl httpsUrl(QStringLiteral("https://radio.example/logo"));
    const QUrl httpUrl(QStringLiteral("http://radio.example/logo"));
    QVERIFY(NetworkUrlGuard::isSafeUrl(
        httpsUrl, {QHostAddress(QStringLiteral("93.184.216.34"))}, false));
    QVERIFY(!NetworkUrlGuard::isSafeUrl(
        httpUrl, {QHostAddress(QStringLiteral("93.184.216.34"))}, false));
    QVERIFY(NetworkUrlGuard::isSafeUrl(
        httpUrl, {QHostAddress(QStringLiteral("93.184.216.34"))}, true));

    const QStringList unsafeAddresses{
        QStringLiteral("0.0.0.0"),       QStringLiteral("10.0.0.1"),
        QStringLiteral("100.64.0.1"),    QStringLiteral("127.0.0.1"),
        QStringLiteral("169.254.1.1"),   QStringLiteral("172.16.0.1"),
        QStringLiteral("192.168.1.20"),  QStringLiteral("198.18.0.1"),
        QStringLiteral("224.0.0.1"),     QStringLiteral("::"),
        QStringLiteral("::1"),           QStringLiteral("fe80::1"),
        QStringLiteral("fc00::1"),       QStringLiteral("ff02::1"),
        QStringLiteral("::ffff:127.0.0.1")};
    for (const QString &address : unsafeAddresses) {
        const QHostAddress parsed(address);
        QVERIFY2(!NetworkUrlGuard::isPublicAddress(parsed),
                 qPrintable(address));
        QVERIFY(!NetworkUrlGuard::isSafeUrl(httpsUrl, {parsed}, false));
    }

    QVERIFY(!NetworkUrlGuard::isSafeUrl(
        httpsUrl,
        {QHostAddress(QStringLiteral("93.184.216.34")),
         QHostAddress(QStringLiteral("127.0.0.1"))},
        false));
    QVERIFY(!NetworkUrlGuard::isSafeUrl(httpsUrl, {}, false));
}

void StationArtworkServiceTest::rejectsHostnameResolvingToLoopback()
{
    bool completed = false;
    bool allowed = true;
    NetworkUrlGuard::resolve(
        QUrl(QStringLiteral("http://localhost:8123/logo")), true, false, this,
        [&completed, &allowed](bool result, const QList<QHostAddress> &) {
            allowed = result;
            completed = true;
        });
    QTRY_VERIFY_WITH_TIMEOUT(completed, 2000);
    QVERIFY(!allowed);
}

void StationArtworkServiceTest::pinsRequestsToResolvedAddress()
{
    const QNetworkRequest request = NetworkUrlGuard::requestForResolvedUrl(
        QUrl(QStringLiteral("http://radio.example:8080/logo")),
        QHostAddress(QStringLiteral("93.184.216.34")));
    QCOMPARE(request.url(),
             QUrl(QStringLiteral("http://93.184.216.34:8080/logo")));
    QCOMPARE(request.rawHeader(QByteArrayLiteral("Host")),
             QByteArrayLiteral("radio.example:8080"));
    QCOMPARE(request.attribute(QNetworkRequest::Http2AllowedAttribute).toBool(),
             false);

    const QNetworkRequest secureRequest =
        NetworkUrlGuard::requestForResolvedUrl(
            QUrl(QStringLiteral("https://radio.example/logo")),
            QHostAddress(QStringLiteral("2001:4860:4860::8888")));
    QCOMPARE(secureRequest.url(),
             QUrl(QStringLiteral("https://[2001:4860:4860::8888]/logo")));
    QCOMPARE(secureRequest.rawHeader(QByteArrayLiteral("Host")),
             QByteArrayLiteral("radio.example"));
    QCOMPARE(secureRequest.peerVerifyName(), QStringLiteral("radio.example"));
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
