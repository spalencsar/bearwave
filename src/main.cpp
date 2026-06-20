// Copyright (c) 2026 Sebastian Palencsar
// SPDX-License-Identifier: GPL-3.0-or-later

#include <QApplication>
#include <QDBusConnection>
#include <QDBusConnectionInterface>
#include <QDBusMessage>
#include <QDebug>
#include <QIcon>
#include <QLocale>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QQuickWindow>
#include <QTranslator>

#include "radiobackend.h"
#include "mprisadaptor.h"
#include "bearwavecontroladaptor.h"
#include "notificationmanager.h"
#include "systemtraymanager.h"


int main(int argc, char *argv[])
{
    QApplication::setApplicationName(QStringLiteral("BearWave"));
    QApplication::setDesktopFileName(QStringLiteral("de.nerdbear.bearwave"));
    QApplication::setOrganizationName(QStringLiteral("BearWave"));
    QApplication::setApplicationVersion(QStringLiteral("1.0.4"));

    QApplication app(argc, argv);

    QDBusConnection sessionBus = QDBusConnection::sessionBus();
    if (sessionBus.isConnected() && sessionBus.interface() && sessionBus.interface()->isServiceRegistered(QStringLiteral("org.mpris.MediaPlayer2.bearwave"))) {
        QDBusMessage call = QDBusMessage::createMethodCall(
            QStringLiteral("org.mpris.MediaPlayer2.bearwave"),
            QStringLiteral("/org/mpris/MediaPlayer2"),
            QStringLiteral("org.mpris.MediaPlayer2"),
            QStringLiteral("Raise")
        );
        sessionBus.call(call);
        return 0;
    }
    app.setWindowIcon(QIcon::fromTheme(QStringLiteral("de.nerdbear.bearwave")));
    app.setQuitOnLastWindowClosed(false);

    QTranslator appTranslator;
    if (appTranslator.load(QLocale::system(), QStringLiteral("bearwave"), QStringLiteral("_"), QStringLiteral(":/i18n"))) {
        app.installTranslator(&appTranslator);
    }

    QQmlApplicationEngine engine;
    engine.addImportPath(QStringLiteral("qrc:/qml"));

    RadioBackend backend;
    engine.rootContext()->setContextProperty("radioBackend", &backend);

    auto *mprisRoot = new MprisRootAdaptor(&backend, &app);
    auto *mprisPlayer = new MprisPlayerAdaptor(&backend);
    auto *controlAdaptor = new BearWaveControlAdaptor(&backend);
    auto *notificationManager = new NotificationManager(&backend, &backend);
    Q_UNUSED(mprisRoot)
    Q_UNUSED(controlAdaptor)
    Q_UNUSED(notificationManager)

    if (!sessionBus.registerObject(QStringLiteral("/org/mpris/MediaPlayer2"), &backend, QDBusConnection::ExportAdaptors)) {
        qWarning() << "Failed to register MPRIS D-Bus object";
    }
    if (!sessionBus.registerService(QStringLiteral("org.mpris.MediaPlayer2.bearwave"))) {
        qWarning() << "Failed to register MPRIS D-Bus service org.mpris.MediaPlayer2.bearwave";
    } else {
        mprisPlayer->publishState();
    }

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

    SystemTrayManager trayManager(&backend, mainWindow, &app);
    Q_UNUSED(trayManager)

    return app.exec();
}
