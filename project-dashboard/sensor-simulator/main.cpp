#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include "Simulator.h"
#include <QQmlError>
#include <QDebug>
#include <QCoreApplication>
#include <QUrl>
#include <QQmlComponent>
#include <QQuickStyle>

int main(int argc, char *argv[])
{
    QGuiApplication app(argc, argv);

    // Use Qt Quick Controls Material style for a modern look
    QQuickStyle::setStyle("Material");

    Simulator simulator;
    simulator.startServer(9002); // start WebSocket server for device connections

    QQmlApplicationEngine engine;
    engine.rootContext()->setContextProperty("simulator", &simulator);
    // Ensure quit requests from QML are handled on the application (main) thread
    QObject::connect(&simulator, &Simulator::quitRequested, &app, &QCoreApplication::quit);

    const QUrl url(QStringLiteral("qrc:/qml/Main.qml"));
    // Ensure the compiled resource is initialized (manual Q_INIT_RESOURCE can help
    // in some build/link scenarios where the generated initializer isn't run)
    Q_INIT_RESOURCE(qml);

    // Try to instantiate the main QML component first so we can surface any errors
    {
        QQmlComponent component(&engine, url);
        if (component.isError())
        {
            const auto errors = component.errors();
            for (const QQmlError &e : errors)
            {
                qWarning() << "QQmlError:" << e.toString();
            }
        }
        else
        {
            engine.load(url);
        }
    }

    // If QRC-based QML wasn't embedded or failed to load, fall back to local qml files.
    if (engine.rootObjects().isEmpty())
    {
        QString localPath = QCoreApplication::applicationDirPath() + "/qml/Main.qml";
        QUrl localUrl = QUrl::fromLocalFile(localPath);
        engine.load(localUrl);
    }

    // If still empty, fail gracefully.
    if (engine.rootObjects().isEmpty())
    {
        QCoreApplication::exit(-1);
    }

    int rc = app.exec();
    qDebug() << "Main: app.exec() returned" << rc;
    return rc;
}
