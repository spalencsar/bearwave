// Copyright (c) 2026 Sebastian Palencsar
// SPDX-License-Identifier: GPL-3.0-or-later

#include <QtTest>

#include <QBuffer>
#include <QColor>
#include <QDateTime>
#include <QFile>
#include <QHostAddress>
#include <QImage>
#include <QSignalSpy>
#include <QTcpServer>
#include <QTcpSocket>
#include <QTemporaryDir>
#include <QUrl>

#include "stationimagecache.h"

namespace {
const QString fallbackSource = QStringLiteral("qrc:/assets/app/bearwave.svg");

class ImageTestServer : public QTcpServer
{
public:
    ImageTestServer(const QByteArray &contentType, const QByteArray &body,
                    QObject *parent = nullptr)
        : QTcpServer(parent)
        , m_contentType(contentType)
        , m_body(body)
    {
        connect(this, &QTcpServer::newConnection, this, [this]() {
            while (hasPendingConnections()) {
                QTcpSocket *socket = nextPendingConnection();
                connect(socket, &QTcpSocket::readyRead, socket, [this, socket]() {
                    QByteArray request = socket->property("request").toByteArray();
                    request += socket->readAll();
                    socket->setProperty("request", request);
                    if (socket->property("responded").toBool()
                        || !request.contains(QByteArrayLiteral("\r\n\r\n"))) {
                        return;
                    }
                    socket->setProperty("responded", true);
                    ++m_requestCount;
                    const QByteArray response =
                        QByteArrayLiteral("HTTP/1.1 200 OK\r\nContent-Type: ")
                        + m_contentType
                        + QByteArrayLiteral("\r\nContent-Length: ")
                        + QByteArray::number(m_body.size())
                        + QByteArrayLiteral("\r\nConnection: close\r\n\r\n")
                        + m_body;
                    socket->write(response);
                    socket->disconnectFromHost();
                });
            }
        });
        QVERIFY(listen(QHostAddress::LocalHost));
    }

    QString url(const QString &path = QStringLiteral("/image")) const
    {
        return QStringLiteral("http://127.0.0.1:%1%2").arg(serverPort()).arg(path);
    }

    int requestCount() const { return m_requestCount; }

private:
    QByteArray m_contentType;
    QByteArray m_body;
    int m_requestCount = 0;
};

QByteArray pngImageData()
{
    QImage image(24, 18, QImage::Format_ARGB32_Premultiplied);
    image.fill(QColor(QStringLiteral("#ff4f86")));
    QByteArray data;
    QBuffer buffer(&data);
    buffer.open(QIODevice::WriteOnly);
    const bool saved = image.save(&buffer, "PNG");
    Q_ASSERT(saved);
    return data;
}

QByteArray oversizedDimensionPngData()
{
    QImage image(2049, 1, QImage::Format_ARGB32_Premultiplied);
    image.fill(Qt::black);
    QByteArray data;
    QBuffer buffer(&data);
    buffer.open(QIODevice::WriteOnly);
    const bool saved = image.save(&buffer, "PNG");
    Q_ASSERT(saved);
    return data;
}
}

class StationImageCacheTest : public QObject
{
    Q_OBJECT

private slots:
    void downloadsValidImageAndReusesPngCache();
    void blocksUndecodableImageForSession();
    void rejectsNonImageMimeTypeAndOversizedDownload();
    void rejectsPrivateHttpOutsideTestMode();
    void prunesExpiredAndLeastRecentlyUsedFiles();
};

void StationImageCacheTest::downloadsValidImageAndReusesPngCache()
{
    QTemporaryDir directory;
    QVERIFY(directory.isValid());
    ImageTestServer server(QByteArrayLiteral("image/png"), pngImageData());
    StationImageCache cache(directory.path(), true);
    QSignalSpy readySpy(&cache, &StationImageCache::imageReady);

    QCOMPARE(cache.sourceForUrl(server.url()), fallbackSource);
    QTRY_COMPARE_WITH_TIMEOUT(readySpy.count(), 1, 2000);

    const QString localSource = cache.sourceForUrl(server.url());
    QVERIFY(localSource.startsWith(QStringLiteral("file:")));
    const QString localPath = QUrl(localSource).toLocalFile();
    QVERIFY(QFile::exists(localPath));
    QImage normalized(localPath);
    QVERIFY(!normalized.isNull());
    QCOMPARE(normalized.size(), QSize(24, 18));
    QCOMPARE(server.requestCount(), 1);

    StationImageCache secondInstance(directory.path(), true);
    QCOMPARE(secondInstance.sourceForUrl(server.url()), localSource);
    QCOMPARE(server.requestCount(), 1);
}

void StationImageCacheTest::blocksUndecodableImageForSession()
{
    QTemporaryDir directory;
    QVERIFY(directory.isValid());
    ImageTestServer server(QByteArrayLiteral("image/png"),
                           QByteArrayLiteral("this is not an image"));
    StationImageCache cache(directory.path(), true);
    QSignalSpy rejectedSpy(&cache, &StationImageCache::imageRejected);

    QCOMPARE(cache.sourceForUrl(server.url()), fallbackSource);
    QTRY_COMPARE_WITH_TIMEOUT(rejectedSpy.count(), 1, 2000);
    QVERIFY(cache.isBlocked(server.url()));
    QCOMPARE(cache.sourceForUrl(server.url()), fallbackSource);
    QCOMPARE(server.requestCount(), 1);
}

void StationImageCacheTest::rejectsNonImageMimeTypeAndOversizedDownload()
{
    QTemporaryDir directory;
    QVERIFY(directory.isValid());
    ImageTestServer htmlServer(QByteArrayLiteral("text/html"), pngImageData());
    ImageTestServer largeServer(QByteArrayLiteral("image/png"),
                                QByteArray(2 * 1024 * 1024 + 1, 'x'));
    ImageTestServer dimensionServer(QByteArrayLiteral("image/png"),
                                    oversizedDimensionPngData());
    StationImageCache cache(directory.path(), true);
    QSignalSpy rejectedSpy(&cache, &StationImageCache::imageRejected);

    QCOMPARE(cache.sourceForUrl(htmlServer.url()), fallbackSource);
    QTRY_COMPARE_WITH_TIMEOUT(rejectedSpy.count(), 1, 2000);
    QVERIFY(cache.isBlocked(htmlServer.url()));

    QCOMPARE(cache.sourceForUrl(largeServer.url()), fallbackSource);
    QTRY_COMPARE_WITH_TIMEOUT(rejectedSpy.count(), 2, 2000);
    QVERIFY(cache.isBlocked(largeServer.url()));

    QCOMPARE(cache.sourceForUrl(dimensionServer.url()), fallbackSource);
    QTRY_COMPARE_WITH_TIMEOUT(rejectedSpy.count(), 3, 2000);
    QVERIFY(cache.isBlocked(dimensionServer.url()));
}

void StationImageCacheTest::rejectsPrivateHttpOutsideTestMode()
{
    QTemporaryDir directory;
    QVERIFY(directory.isValid());
    ImageTestServer server(QByteArrayLiteral("image/png"), pngImageData());
    StationImageCache cache(directory.path(), false);

    QCOMPARE(cache.sourceForUrl(server.url()), fallbackSource);
    QTest::qWait(50);
    QCOMPARE(server.requestCount(), 0);
}

void StationImageCacheTest::prunesExpiredAndLeastRecentlyUsedFiles()
{
    QTemporaryDir directory;
    QVERIFY(directory.isValid());
    const QDateTime now = QDateTime::currentDateTimeUtc();

    auto createFile = [&directory](const QString &name, int bytes,
                                   const QDateTime &modified) {
        QFile file(directory.filePath(name));
        if (!file.open(QIODevice::WriteOnly)) {
            return false;
        }
        file.write(QByteArray(bytes, 'x'));
        file.close();
        if (!file.open(QIODevice::ReadOnly)) {
            return false;
        }
        return file.setFileTime(modified, QFileDevice::FileModificationTime);
    };

    QVERIFY(createFile(QStringLiteral("expired.png"), 4, now.addDays(-31)));
    QVERIFY(createFile(QStringLiteral("oldest.png"), 4, now.addSecs(-20)));
    QVERIFY(createFile(QStringLiteral("newest.png"), 4, now.addSecs(-10)));
    StationImageCache::pruneCacheDirectory(directory.path(), 4, 30);

    QVERIFY(!QFile::exists(directory.filePath(QStringLiteral("expired.png"))));
    QVERIFY(!QFile::exists(directory.filePath(QStringLiteral("oldest.png"))));
    QVERIFY(QFile::exists(directory.filePath(QStringLiteral("newest.png"))));
}

QTEST_GUILESS_MAIN(StationImageCacheTest)
#include "station_image_cache_test.moc"
