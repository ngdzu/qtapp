#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include "core/MockDeviceDataService.h"
#include "ui/DashboardController.h"

int main(int argc, char *argv[])
{
    QGuiApplication app(argc, argv);

    // Create the backend services
    MockDeviceDataService *dataService = new MockDeviceDataService(&app);
    DashboardController *dashboardController = new DashboardController(&app);

    // Wire them up
    dashboardController->setService(dataService);

    QQmlApplicationEngine engine;

    // Expose the controller to QML
    engine.rootContext()->setContextProperty("dashboard", dashboardController);

    const QUrl url(QStringLiteral("qrc:/resources/qml/Main.qml"));
    QObject::connect(&engine, &QQmlApplicationEngine::objectCreated, &app, [url](QObject *obj, const QUrl &objUrl)
                     {
        if (!obj && url == objUrl)
            QCoreApplication::exit(-1); }, Qt::QueuedConnection);

    engine.load(url);

    return app.exec();
}
