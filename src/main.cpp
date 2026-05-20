#include <QApplication>
#include <QDBusConnection>
#include <QIcon>
#include <QQmlApplicationEngine>
#include <QQmlContext>

#include "radiobackend.h"
#include "mprisadaptor.h"
#include "bearwavecontroladaptor.h"


int main(int argc, char *argv[])
{
    QApplication::setApplicationName(QStringLiteral("BearWave"));
    QApplication::setDesktopFileName(QStringLiteral("org.kde.bearwave.desktop"));
    QApplication::setOrganizationName(QStringLiteral("BearWave"));
    QApplication::setApplicationVersion(QStringLiteral("1.0.0"));

    QApplication app(argc, argv);
    app.setWindowIcon(QIcon::fromTheme(QStringLiteral("org.kde.bearwave")));
    app.setQuitOnLastWindowClosed(false);

    QQmlApplicationEngine engine;

    RadioBackend backend;
    engine.rootContext()->setContextProperty("radioBackend", &backend);

    MprisRootAdaptor mprisRoot(&backend, &app);
    MprisPlayerAdaptor mprisPlayer(&backend);
    BearWaveControlAdaptor controlAdaptor(&backend);
    Q_UNUSED(mprisRoot)
    Q_UNUSED(mprisPlayer)
    Q_UNUSED(controlAdaptor)

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
