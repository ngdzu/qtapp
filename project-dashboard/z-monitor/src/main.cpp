/**
 * @file main.cpp
 * @brief Entry point for the Z Monitor Qt application.
 *
 * This file bootstraps the Qt application and loads the QML-based
 * user interface. At this bootstrap stage the UI is a placeholder
 * dashboard view; future iterations will wire QObject controllers
 * from the interface layer as described in the architecture docs.
 */

#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QUrl>

/**
 * @brief Application entry point.
 *
 * Creates the Qt application object and loads the root QML file for
 * the Z Monitor interface. The function returns the Qt event loop
 * exit code.
 *
 * @param argc Argument count from the C runtime.
 * @param argv Argument vector from the C runtime.
 * @return int Qt event loop exit code.
 */
int main(int argc, char *argv[])
{
    QGuiApplication app(argc, argv);

    QQmlApplicationEngine engine;
    const QUrl mainQmlUrl(QStringLiteral("qrc:/qml/Main.qml"));

    QObject::connect(
        &engine,
        &QQmlApplicationEngine::objectCreated,
        &app,
        [mainQmlUrl](QObject *obj, const QUrl &objUrl) {
            if (!obj && mainQmlUrl == objUrl) {
                QCoreApplication::exit(-1);
            }
        },
        Qt::QueuedConnection);

    engine.load(mainQmlUrl);

    return app.exec();
}


