// Copyright (c) 2026 Sebastian Palencsar
// SPDX-License-Identifier: GPL-3.0-or-later

#include <QFile>
#include <QtTest>

class QrcResourcesTest : public QObject
{
    Q_OBJECT

private slots:
    void mainQmlIsEmbedded();
    void qmlModulesAreEmbedded();
    void remoteTextIsRenderedAsPlainText();
};

void QrcResourcesTest::mainQmlIsEmbedded()
{
    QVERIFY2(QFile::exists(QStringLiteral(":/qml/Main.qml")),
             "Main UI shell must be available at qrc:/qml/Main.qml");
}

void QrcResourcesTest::qmlModulesAreEmbedded()
{
    QVERIFY(QFile::exists(QStringLiteral(":/qml/theme/qmldir")));
    QVERIFY(QFile::exists(QStringLiteral(":/qml/components/qmldir")));
    QVERIFY(QFile::exists(QStringLiteral(":/qml/components/AppButton.qml")));
    QVERIFY(QFile::exists(QStringLiteral(":/qml/components/SidebarNavigation.qml")));
    QVERIFY(QFile::exists(QStringLiteral(":/qml/components/StationDetailPanel.qml")));
    QVERIFY(QFile::exists(QStringLiteral(":/assets/app/bearwave.svg")));
    QVERIFY(QFile::exists(QStringLiteral(":/assets/app/bearwave.png")));
    QFile changelog(QStringLiteral(":/CHANGELOG.md"));
    QVERIFY(changelog.open(QIODevice::ReadOnly));
    QVERIFY(changelog.readAll().startsWith(QByteArrayLiteral("# Changelog")));
    QVERIFY(QFile::exists(QStringLiteral(":/l10n/territories_de.json")));
    QVERIFY(QFile::exists(QStringLiteral(":/l10n/territories_nl.json")));
    QVERIFY(QFile::exists(QStringLiteral(":/l10n/territories_ru.json")));
}

void QrcResourcesTest::remoteTextIsRenderedAsPlainText()
{
    const auto qmlContents = [](const QString &path) {
        QFile file(path);
        if (!file.open(QIODevice::ReadOnly)) {
            return QByteArray();
        }
        return file.readAll();
    };

    const QByteArray stationCard =
        qmlContents(QStringLiteral(":/qml/components/StationCard.qml"));
    QVERIFY(stationCard.contains(
        QByteArrayLiteral("text: root.modelData.name || \"\"\n"
                          "                    textFormat: Text.PlainText")));
    const QByteArray stationDetail =
        qmlContents(QStringLiteral(":/qml/components/StationDetailPanel.qml"));
    QVERIFY(stationDetail.contains(
        QByteArrayLiteral("text: root.stationName.length > 0 ? root.stationName"
                          " : qsTr(\"Unknown station\")\n"
                          "                    textFormat: Text.PlainText")));
    QVERIFY(stationDetail.contains(
        QByteArrayLiteral("text: root.previousTrack ? (root.previousTrack.artist"
                          " || \"\") : \"\"\n"
                          "                        textFormat: Text.PlainText")));
    QVERIFY(stationDetail.contains(
        QByteArrayLiteral("text: parent.modelData\n"
                          "                            textFormat: Text.PlainText")));
    QVERIFY(stationDetail.contains(
        QByteArrayLiteral("text: root.codec.toUpperCase()\n"
                          "                        textFormat: Text.PlainText")));
    const QByteArray playerBar =
        qmlContents(QStringLiteral(":/qml/components/PlayerBar.qml"));
    QVERIFY(playerBar.contains(
        QByteArrayLiteral("text: root.connectionStatusText()\n"
                          "                textFormat: Text.PlainText")));
    QCOMPARE(playerBar.count(
                 QByteArrayLiteral("text: root.connectionStatusText()\n"
                                   "                textFormat: Text.PlainText")),
             1);
}

QTEST_APPLESS_MAIN(QrcResourcesTest)
#include "qrc_resources_test.moc"
