// Copyright (c) 2026 Sebastian Palencsar
// SPDX-License-Identifier: GPL-3.0-or-later

#include <QApplication>
#include <QCommandLineOption>
#include <QCommandLineParser>
#include <QDBusConnection>
#include <QDBusConnectionInterface>
#include <QDBusMessage>
#include <QDebug>
#include <QFile>
#include <QHash>
#include <QIcon>
#include <QLocale>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QQuickStyle>
#include <QQuickWindow>
#include <QStyleHints>
#include <QTranslator>

#include "appsettings.h"
#include "radiobackend.h"
#include "mprisadaptor.h"
#include "bearwavecontroladaptor.h"
#include "notificationmanager.h"
#include "systemtraymanager.h"
#include "colorschemecontroller.h"

namespace {
bool isPlasmaDesktopSession()
{
    const QString desktop = QString::fromLocal8Bit(qgetenv("XDG_CURRENT_DESKTOP")).toLower();
    return desktop.contains(QLatin1String("kde")) || desktop.contains(QLatin1String("plasma"));
}

// On non-KDE desktops avoid inheriting a KDE/Breeze platform theme so the app
// does not look like a "KDE guest".
void configureNonPlasmaChrome()
{
    if (isPlasmaDesktopSession()) {
        return;
    }
    if (qEnvironmentVariableIsEmpty("QT_QUICK_CONTROLS_STYLE")) {
        qputenv("QT_QUICK_CONTROLS_STYLE", "Fusion");
    }
    const QByteArray platformTheme = qgetenv("QT_QPA_PLATFORMTHEME").toLower();
    if (platformTheme.contains("kde") || platformTheme.contains("plasma")) {
        qunsetenv("QT_QPA_PLATFORMTHEME");
    }
}
} // namespace

int main(int argc, char *argv[])
{
    configureNonPlasmaChrome();

    QApplication app(argc, argv);

    // Must run before any QML is loaded (non-Plasma only).
    if (!isPlasmaDesktopSession()
        && (qEnvironmentVariableIsEmpty("QT_QUICK_CONTROLS_STYLE")
            || qEnvironmentVariable("QT_QUICK_CONTROLS_STYLE") == QLatin1String("Fusion"))) {
        QQuickStyle::setStyle(QStringLiteral("Fusion"));
    }

    QApplication::setApplicationName(QStringLiteral("BearWave"));
    QApplication::setDesktopFileName(QStringLiteral("de.nerdbear.bearwave"));
    QApplication::setOrganizationName(QStringLiteral("BearWave"));
    QApplication::setApplicationVersion(QStringLiteral(BEARWAVE_VERSION));

    QCommandLineParser commandLine;
    commandLine.setApplicationDescription(QStringLiteral("Desktop internet radio player"));
    commandLine.addHelpOption();
    commandLine.addVersionOption();
    const QCommandLineOption languageOption(
        {QStringLiteral("l"), QStringLiteral("language")},
        QStringLiteral("Temporarily override UI and country-name language: system, de, en, nl, or ru"),
        QStringLiteral("language"));
    commandLine.addOption(languageOption);
    commandLine.process(app);

    AppSettings appSettings;
    QLocale appLocale = QLocale::system();
    const QString requestedLanguage =
        commandLine.isSet(languageOption)
        ? commandLine.value(languageOption).trimmed().toLower()
        : appSettings.language();
    if (requestedLanguage != QStringLiteral("system")) {
        static const QHash<QString, QString> localeNames = {
            {QStringLiteral("de"), QStringLiteral("de_DE")},
            {QStringLiteral("en"), QStringLiteral("en_US")},
            {QStringLiteral("nl"), QStringLiteral("nl_NL")},
            {QStringLiteral("ru"), QStringLiteral("ru_RU")},
        };
        const auto localeName = localeNames.constFind(requestedLanguage);
        if (localeName != localeNames.constEnd()) {
            appLocale = QLocale(*localeName);
        } else {
            qWarning().noquote()
                << QStringLiteral("Unsupported language '%1'; using system locale %2.")
                       .arg(requestedLanguage, appLocale.name());
        }
    }
    QLocale::setDefault(appLocale);

    QDBusConnection sessionBus = QDBusConnection::sessionBus();
    if (sessionBus.isConnected() && sessionBus.interface() && sessionBus.interface()->isServiceRegistered(QStringLiteral("org.mpris.MediaPlayer2.bearwave"))) {
        QDBusMessage call = QDBusMessage::createMethodCall(
            QStringLiteral("org.mpris.MediaPlayer2.bearwave"),
            QStringLiteral("/org/mpris/MediaPlayer2"),
            QStringLiteral("org.mpris.MediaPlayer2"),
            QStringLiteral("Raise")
        );
        sessionBus.call(call);
        qInfo() << "BearWave is already running; raised the existing window.";
        return 0;
    }
    app.setWindowIcon(QIcon(QStringLiteral(":/assets/app/bearwave.svg")));
    app.setQuitOnLastWindowClosed(false);

    QTranslator appTranslator;
    const QString uiLanguage = appLocale.name().section(QLatin1Char('_'), 0, 0);
    const QString translationPath =
        QStringLiteral(":/i18n/bearwave_%1.qm").arg(uiLanguage);
    if (appTranslator.load(translationPath)) {
        app.installTranslator(&appTranslator);
        qInfo().noquote() << QStringLiteral("Application locale: %1 (translation loaded)")
                                .arg(appLocale.name());
    } else {
        qInfo().noquote() << QStringLiteral("Application locale: %1 (English UI fallback)")
                                .arg(appLocale.name());
    }

    // QML disk cache can retain stale bytecode across app updates (same qrc paths).
    qputenv("QML_DISABLE_DISK_CACHE", "1");

    QQmlApplicationEngine engine;
    engine.addImportPath(QStringLiteral("qrc:/qml"));

    RadioBackend backend;
    ColorSchemeController colorScheme;
    QFile changelogFile(QStringLiteral(":/CHANGELOG.md"));
    QString changelogText;
    if (changelogFile.open(QIODevice::ReadOnly)) {
        changelogText = QString::fromUtf8(changelogFile.readAll());
    }
    engine.rootContext()->setContextProperty("radioBackend", &backend);
    engine.rootContext()->setContextProperty("appLanguageSettings", &appSettings);
    engine.rootContext()->setContextProperty("bearwaveVersion", QStringLiteral(BEARWAVE_VERSION));
    engine.rootContext()->setContextProperty("bearwaveBuildId", QStringLiteral(BEARWAVE_GIT_HASH));
    engine.rootContext()->setContextProperty("bearwaveChangelog", changelogText);
    // Light/dark: portal / gsettings / optional shell session (ColorSchemeController).
    engine.rootContext()->setContextProperty(QStringLiteral("bearwaveColorScheme"), &colorScheme);
    // Frameless client chrome outside KDE sessions (own BearTheme bar).
    engine.rootContext()->setContextProperty(
        QStringLiteral("bearwaveClientChrome"),
        QVariant(!isPlasmaDesktopSession()));

    auto *mprisRoot = new MprisRootAdaptor(&backend, &app);
    auto *mprisPlayer = new MprisPlayerAdaptor(&backend);
    auto *controlAdaptor = new BearWaveControlAdaptor(&backend);
    auto *notificationManager = new NotificationManager(&backend, &backend);
    Q_UNUSED(mprisRoot)
    Q_UNUSED(controlAdaptor)
    Q_UNUSED(notificationManager)

    const QUrl url(QStringLiteral("qrc:/qml/Main.qml"));
    QObject::connect(&engine, &QQmlApplicationEngine::objectCreated,
                     &app, [&app, url](QObject *obj, const QUrl &objUrl) {
        if (!obj && url == objUrl) {
            qCritical("Failed to load QML file");
            app.exit(EXIT_FAILURE);
        }
    }, Qt::QueuedConnection);

    engine.load(url);

    if (engine.rootObjects().isEmpty()) {
        return EXIT_FAILURE;
    }

    QQuickWindow *mainWindow = qobject_cast<QQuickWindow *>(engine.rootObjects().constFirst());
    if (!mainWindow) {
        qCritical("Failed to obtain main window");
        return EXIT_FAILURE;
    }

    QObject::connect(&backend, &RadioBackend::raiseRequested, mainWindow, [mainWindow]() {
        mainWindow->show();
        mainWindow->raise();
        mainWindow->requestActivate();
    });

    if (!sessionBus.registerObject(QStringLiteral("/org/mpris/MediaPlayer2"), &backend, QDBusConnection::ExportAdaptors)) {
        qWarning() << "Failed to register MPRIS D-Bus object";
    }
    if (!sessionBus.registerService(QStringLiteral("org.mpris.MediaPlayer2.bearwave"))) {
        qWarning() << "Failed to register MPRIS D-Bus service org.mpris.MediaPlayer2.bearwave";
    } else {
        mprisPlayer->publishState();
    }

    mainWindow->show();
    mainWindow->raise();
    mainWindow->requestActivate();

    SystemTrayManager trayManager(&backend, mainWindow, &app);
    Q_UNUSED(trayManager)

    return app.exec();
}
