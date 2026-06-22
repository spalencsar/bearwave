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
    QVERIFY(QFile::exists(QStringLiteral(":/assets/app/bearwave.png")));
}

QTEST_APPLESS_MAIN(QrcResourcesTest)
#include "qrc_resources_test.moc"