// Copyright (c) 2026 Sebastian Palencsar
// SPDX-License-Identifier: GPL-3.0-or-later

#include "appsettings.h"

#include <QSettings>
#include <QStringList>

namespace {
const QString languageKey = QStringLiteral("ui/language");
}

AppSettings::AppSettings(QObject *parent)
    : QObject(parent)
    , m_settings(new QSettings(this))
{
    load();
}

AppSettings::AppSettings(const QString &settingsFile, QObject *parent)
    : QObject(parent)
    , m_settings(new QSettings(settingsFile, QSettings::IniFormat, this))
{
    load();
}

QString AppSettings::language() const
{
    return m_language;
}

void AppSettings::setLanguage(const QString &language)
{
    const QString normalizedLanguage = language.trimmed().toLower();
    if (!isSupportedLanguage(normalizedLanguage) || normalizedLanguage == m_language) {
        return;
    }

    const bool previouslyRequiredRestart = restartRequired();
    m_language = normalizedLanguage;
    m_settings->setValue(languageKey, m_language);
    m_settings->sync();
    emit languageChanged();

    if (previouslyRequiredRestart != restartRequired()) {
        emit restartRequiredChanged();
    }
}

bool AppSettings::restartRequired() const
{
    return m_language != m_startupLanguage;
}

bool AppSettings::isSupportedLanguage(const QString &language)
{
    static const QStringList supportedLanguages = {
        QStringLiteral("system"),
        QStringLiteral("de"),
        QStringLiteral("en"),
        QStringLiteral("nl"),
        QStringLiteral("ru"),
    };
    return supportedLanguages.contains(language.trimmed().toLower());
}

void AppSettings::load()
{
    const QString storedLanguage =
        m_settings->value(languageKey, QStringLiteral("system")).toString().trimmed().toLower();
    m_language = isSupportedLanguage(storedLanguage) ? storedLanguage : QStringLiteral("system");
    m_startupLanguage = m_language;
}
