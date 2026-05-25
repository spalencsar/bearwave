#include <QApplication>
#include <QDBusConnection>
#include <QIcon>
#include <QLocale>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QTranslator>

#include "radiobackend.h"
#include "mprisadaptor.h"
#include "bearwavecontroladaptor.h"
#include "notificationmanager.h"


int main(int argc, char *argv[])
{
    QApplication::setApplicationName(QStringLiteral("BearWave"));
    QApplication::setDesktopFileName(QStringLiteral("de.nerdbear.bearwave"));
    QApplication::setOrganizationName(QStringLiteral("BearWave"));
    QApplication::setApplicationVersion(QStringLiteral("1.0.1"));

    QApplication app(argc, argv);
    app.setWindowIcon(QIcon::fromTheme(QStringLiteral("de.nerdbear.bearwave")));
    app.setQuitOnLastWindowClosed(false);

    QTranslator appTranslator;
    if (appTranslator.load(QLocale::system(), QStringLiteral("bearwave"), QStringLiteral("_"), QStringLiteral(":/i18n"))) {
        app.installTranslator(&appTranslator);
    }

    QQmlApplicationEngine engine;

    RadioBackend backend;
    engine.rootContext()->setContextProperty("radioBackend", &backend);

    MprisRootAdaptor mprisRoot(&backend, &app);
    MprisPlayerAdaptor mprisPlayer(&backend);
    BearWaveControlAdaptor controlAdaptor(&backend);
    NotificationManager notificationManager(&backend);
    Q_UNUSED(mprisRoot)
    Q_UNUSED(mprisPlayer)
    Q_UNUSED(controlAdaptor)
    Q_UNUSED(notificationManager)

    QDBusConnection sessionBus = QDBusConnection::sessionBus();
    sessionBus.registerObject(QStringLiteral("/org/mpris/MediaPlayer2"), &backend, QDBusConnection::ExportAdaptors);
    sessionBus.registerService(QStringLiteral("org.mpris.MediaPlayer2.bearwave"));

    const QUrl url(QStringLiteral("qrc:/Main.qml"));
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

    return app.exec();
}
