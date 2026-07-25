// Copyright (c) 2026 Sebastian Palencsar
// SPDX-License-Identifier: GPL-3.0-or-later

#include <QtTest>

#include <QSignalSpy>
#include <QTemporaryDir>

#include "appsettings.h"

class AppSettingsTest : public QObject
{
    Q_OBJECT

private slots:
    void persistsSupportedLanguageAndTracksRestart();
    void rejectsUnsupportedLanguage();
};

void AppSettingsTest::persistsSupportedLanguageAndTracksRestart()
{
    QTemporaryDir directory;
    QVERIFY(directory.isValid());
    const QString settingsFile = directory.filePath(QStringLiteral("bearwave.ini"));

    {
        AppSettings settings(settingsFile);
        QCOMPARE(settings.language(), QStringLiteral("system"));
        QVERIFY(!settings.restartRequired());

        QSignalSpy languageSpy(&settings, &AppSettings::languageChanged);
        QSignalSpy restartSpy(&settings, &AppSettings::restartRequiredChanged);
        settings.setLanguage(QStringLiteral("NL"));

        QCOMPARE(settings.language(), QStringLiteral("nl"));
        QVERIFY(settings.restartRequired());
        QCOMPARE(languageSpy.count(), 1);
        QCOMPARE(restartSpy.count(), 1);

        settings.setLanguage(QStringLiteral("system"));
        QVERIFY(!settings.restartRequired());
        QCOMPARE(restartSpy.count(), 2);
    }

    {
        AppSettings settings(settingsFile);
        QCOMPARE(settings.language(), QStringLiteral("system"));
        QVERIFY(!settings.restartRequired());
        settings.setLanguage(QStringLiteral("ru"));
    }

    AppSettings reloaded(settingsFile);
    QCOMPARE(reloaded.language(), QStringLiteral("ru"));
    QVERIFY(!reloaded.restartRequired());
}

void AppSettingsTest::rejectsUnsupportedLanguage()
{
    QTemporaryDir directory;
    QVERIFY(directory.isValid());

    AppSettings settings(directory.filePath(QStringLiteral("bearwave.ini")));
    QSignalSpy languageSpy(&settings, &AppSettings::languageChanged);
    settings.setLanguage(QStringLiteral("fr"));

    QCOMPARE(settings.language(), QStringLiteral("system"));
    QVERIFY(!settings.restartRequired());
    QCOMPARE(languageSpy.count(), 0);
}

QTEST_APPLESS_MAIN(AppSettingsTest)
#include "appsettings_test.moc"
