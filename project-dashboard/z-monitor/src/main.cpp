/**
 * @file main.cpp
 * @brief Entry point for the Z Monitor Qt application.
 *
 * This file bootstraps the Qt application and loads the QML-based
 * user interface. Controllers are instantiated and registered with QML
 * for live sensor data display.
 */

#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QUrl>
#include <memory>

// Infrastructure
#include "infrastructure/sensors/SharedMemorySensorDataSource.h"
#include "infrastructure/caching/VitalsCache.h"
#include "infrastructure/caching/WaveformCache.h"
#include "infrastructure/persistence/DatabaseManager.h"
#include "infrastructure/persistence/SQLiteVitalsRepository.h"

// Application
#include "application/services/MonitoringService.h"

// Interface
#include "interface/controllers/DashboardController.h"
#include "interface/controllers/WaveformController.h"

/**
 * @brief Application entry point.
 *
 * Creates the Qt application object, instantiates service and controller layers,
 * registers controllers with QML, and loads the root QML file for the Z Monitor interface.
 *
 * @param argc Argument count from the C runtime.
 * @param argv Argument vector from the C runtime.
 * @return int Qt event loop exit code.
 */
int main(int argc, char *argv[])
{
    QGuiApplication app(argc, argv);

    // Create infrastructure layer
    // Note: Ownership transferred to QObject parent hierarchy or managed by shared_ptr
    auto sensorDataSource = std::make_shared<zmon::SharedMemorySensorDataSource>(
        "/tmp/z-monitor-sensor.sock",
        &app);

    auto vitalsCache = std::make_shared<zmon::VitalsCache>(259200);    // 3 days @ 60 Hz
    auto waveformCache = std::make_shared<zmon::WaveformCache>(22500); // 30 seconds @ 250 Hz × 3 channels

    // Create infrastructure persistence (DatabaseManager + Vitals repository)
    auto dbManager = std::make_shared<zmon::DatabaseManager>(&app);
    auto vitalsRepoConcrete = std::make_shared<zmon::SQLiteVitalsRepository>(dbManager);
    std::shared_ptr<zmon::IVitalsRepository> vitalsRepo = std::static_pointer_cast<zmon::IVitalsRepository>(vitalsRepoConcrete);

    // Create application service layer
    auto monitoringService = new zmon::MonitoringService(
        std::shared_ptr<zmon::IPatientRepository>{},   // not yet implemented
        std::shared_ptr<zmon::ITelemetryRepository>{}, // not yet implemented
        std::shared_ptr<zmon::IAlarmRepository>{},     // not yet implemented
        vitalsRepo,                                    // vitals repository
        sensorDataSource,
        vitalsCache,
        waveformCache,
        &app); // Parent to app for lifecycle management

    // Create interface controllers
    auto dashboardController = new zmon::DashboardController(
        monitoringService,
        vitalsCache.get(),
        &app);

    auto waveformController = new zmon::WaveformController(
        waveformCache.get(),
        &app);

    // Start monitoring service
    bool started = monitoringService->start();
    if (!started)
    {
        qWarning() << "Failed to start monitoring service";
        // Continue anyway - UI will show disconnected state
    } // Create QML engine
    QQmlApplicationEngine engine;

    // Register controllers as QML singletons (accessible globally in QML)
    engine.rootContext()->setContextProperty("dashboardController", dashboardController);
    engine.rootContext()->setContextProperty("waveformController", waveformController);

    const QUrl mainQmlUrl(QStringLiteral("qrc:/qml/Main.qml"));

    QObject::connect(
        &engine,
        &QQmlApplicationEngine::objectCreated,
        &app,
        [mainQmlUrl](QObject *obj, const QUrl &objUrl)
        {
            if (!obj && mainQmlUrl == objUrl)
            {
                QCoreApplication::exit(-1);
            }
        },
        Qt::QueuedConnection);

    engine.load(mainQmlUrl);

    return app.exec();
}
