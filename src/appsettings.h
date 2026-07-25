// Copyright (c) 2026 Sebastian Palencsar
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QObject>
#include <QString>

class QSettings;

class AppSettings : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString language READ language WRITE setLanguage NOTIFY languageChanged)
    Q_PROPERTY(bool restartRequired READ restartRequired NOTIFY restartRequiredChanged)

public:
    explicit AppSettings(QObject *parent = nullptr);
    explicit AppSettings(const QString &settingsFile, QObject *parent = nullptr);

    QString language() const;
    void setLanguage(const QString &language);

    bool restartRequired() const;

    static bool isSupportedLanguage(const QString &language);

signals:
    void languageChanged();
    void restartRequiredChanged();

private:
    void load();

    QSettings *m_settings = nullptr;
    QString m_language;
    QString m_startupLanguage;
};
