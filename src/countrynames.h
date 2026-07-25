// Copyright (c) 2026 Sebastian Palencsar
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef COUNTRYNAMES_H
#define COUNTRYNAMES_H

#include <QLocale>
#include <QString>

class CountryNames
{
public:
    static QString displayName(const QString &code, const QString &englishName,
                               const QLocale &locale = QLocale());
    static bool matches(const QString &code, const QString &englishName,
                        const QString &localizedName, const QString &query);
};

#endif // COUNTRYNAMES_H
