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

QTEST_APPLESS_MAIN(QrcResourcesTest)
#include "qrc_resources_test.moc"
