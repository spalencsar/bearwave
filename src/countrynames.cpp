// Copyright (c) 2026 Sebastian Palencsar
// SPDX-License-Identifier: GPL-3.0-or-later

#include "countrynames.h"

#include <QFile>
#include <QHash>
#include <QJsonDocument>
#include <QJsonObject>

namespace {
QHash<QString, QString> loadTerritories(const QString &localeName)
{
    QFile file(QStringLiteral(":/l10n/territories_%1.json").arg(localeName));
    if (!file.open(QIODevice::ReadOnly)) {
        return {};
    }

    const QJsonObject territories = QJsonDocument::fromJson(file.readAll())
                                        .object()
                                        .value(QStringLiteral("main"))
                                        .toObject()
                                        .value(localeName)
                                        .toObject()
                                        .value(QStringLiteral("localeDisplayNames"))
                                        .toObject()
                                        .value(QStringLiteral("territories"))
                                        .toObject();

    QHash<QString, QString> result;
    for (auto it = territories.constBegin(); it != territories.constEnd(); ++it) {
        if (it.key().size() == 2 && it.value().isString()) {
            result.insert(it.key(), it.value().toString());
        }
    }
    return result;
}

const QHash<QString, QString> &territoriesForLanguage(QLocale::Language language)
{
    static const QHash<QString, QString> german = loadTerritories(QStringLiteral("de"));
    static const QHash<QString, QString> dutch = loadTerritories(QStringLiteral("nl"));
    static const QHash<QString, QString> russian = loadTerritories(QStringLiteral("ru"));
    static const QHash<QString, QString> empty;

    switch (language) {
    case QLocale::German:
        return german;
    case QLocale::Dutch:
        return dutch;
    case QLocale::Russian:
        return russian;
    default:
        return empty;
    }
}
}

QString CountryNames::displayName(const QString &code, const QString &englishName,
                                  const QLocale &locale)
{
    const QString normalizedCode = code.trimmed().toUpper();
    const QString localizedName = territoriesForLanguage(locale.language()).value(normalizedCode);
    return localizedName.isEmpty() ? englishName : localizedName;
}

bool CountryNames::matches(const QString &code, const QString &englishName,
                           const QString &localizedName, const QString &query)
{
    const QString normalizedQuery = query.trimmed();
    if (normalizedQuery.isEmpty()) {
        return true;
    }

    return code.contains(normalizedQuery, Qt::CaseInsensitive)
           || englishName.contains(normalizedQuery, Qt::CaseInsensitive)
           || localizedName.contains(normalizedQuery, Qt::CaseInsensitive);
}
