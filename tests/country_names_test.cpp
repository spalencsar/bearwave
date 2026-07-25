// Copyright (c) 2026 Sebastian Palencsar
// SPDX-License-Identifier: GPL-3.0-or-later

#include <QtTest>

#include "countrynames.h"

class CountryNamesTest : public QObject
{
    Q_OBJECT

private slots:
    void localizes_supported_languages_and_variants();
    void falls_back_to_english_for_unsupported_language_and_unknown_code();
    void matches_code_english_and_localized_names();
};

void CountryNamesTest::localizes_supported_languages_and_variants()
{
    QCOMPARE(CountryNames::displayName(QStringLiteral("DE"), QStringLiteral("Germany"),
                                       QLocale(QStringLiteral("de_AT"))),
             QStringLiteral("Deutschland"));
    QCOMPARE(CountryNames::displayName(QStringLiteral("HU"), QStringLiteral("Hungary"),
                                       QLocale(QStringLiteral("nl_BE"))),
             QStringLiteral("Hongarije"));
    QCOMPARE(CountryNames::displayName(QStringLiteral("VI"), QStringLiteral("U.S. Virgin Islands"),
                                       QLocale(QStringLiteral("ru_RU"))),
             QStringLiteral("Виргинские о-ва (США)"));
}

void CountryNamesTest::falls_back_to_english_for_unsupported_language_and_unknown_code()
{
    QCOMPARE(CountryNames::displayName(QStringLiteral("DE"), QStringLiteral("Germany"),
                                       QLocale(QStringLiteral("en_GB"))),
             QStringLiteral("Germany"));
    QCOMPARE(CountryNames::displayName(QStringLiteral("XX"), QStringLiteral("Unknown Place"),
                                       QLocale(QStringLiteral("ru_RU"))),
             QStringLiteral("Unknown Place"));
}

void CountryNamesTest::matches_code_english_and_localized_names()
{
    QVERIFY(CountryNames::matches(QStringLiteral("HU"), QStringLiteral("Hungary"),
                                  QStringLiteral("Ungarn"), QStringLiteral("hu")));
    QVERIFY(CountryNames::matches(QStringLiteral("HU"), QStringLiteral("Hungary"),
                                  QStringLiteral("Ungarn"), QStringLiteral("hung")));
    QVERIFY(CountryNames::matches(QStringLiteral("HU"), QStringLiteral("Hungary"),
                                  QStringLiteral("Ungarn"), QStringLiteral("ung")));
    QVERIFY(!CountryNames::matches(QStringLiteral("HU"), QStringLiteral("Hungary"),
                                   QStringLiteral("Ungarn"), QStringLiteral("france")));
}

QTEST_APPLESS_MAIN(CountryNamesTest)
#include "country_names_test.moc"
