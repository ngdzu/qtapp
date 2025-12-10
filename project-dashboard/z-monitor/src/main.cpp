/**
 * @file main.cpp
 * @brief Entry point for the Z Monitor Qt application.
 *
 * This file bootstraps the Qt application and loads the QML-based
 * user interface. Controllers are instantiated and registered with QML
 * for live sensor data display.
 */

#include <QApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QStandardPaths>
#include <QUrl>
#include <memory>

// Application configuration & DI
#include "application/config/ConfigLoader.h"
#include "ui/bootstrap/DIContainer.h"

// Infrastructure
#include "infrastructure/sensors/SharedMemorySensorDataSource.h"
#include "infrastructure/sensors/InMemorySensorDataSource.h"
#include "infrastructure/caching/VitalsCache.h"
#include "infrastructure/caching/WaveformCache.h"
#include "infrastructure/persistence/DatabaseManager.h"
#include "infrastructure/persistence/QueryRegistry.h"
#include "infrastructure/persistence/SQLitePatientRepository.h"
#include "infrastructure/persistence/SQLiteVitalsRepository.h"
#include "infrastructure/persistence/SQLiteTelemetryRepository.h"
#include "infrastructure/persistence/SQLiteAlarmRepository.h"
#include "infrastructure/persistence/SQLiteActionLogRepository.h"

// Application
#include "application/services/MonitoringService.h"
#include "application/services/AdmissionService.h"
#include "application/services/SecurityService.h"

// UI Layer
#include "ui/controllers/DashboardController.h"
#include "ui/controllers/WaveformController.h"
#include "ui/controllers/PatientController.h"
#include "ui/controllers/AlarmController.h"
#include "ui/controllers/SettingsController.h"
#include "ui/controllers/TrendsController.h"

using namespace zmon;

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
    QApplication app(argc, argv);

    // Add executable directory to plugin search path
    // Qt will automatically append "/sqldrivers" when searching for SQL plugins
    // MUST be done before any QSqlDatabase operations
    QCoreApplication::addLibraryPath(QCoreApplication::applicationDirPath());
    // Also add explicit sqldrivers subdir to be safe
    QCoreApplication::addLibraryPath(QCoreApplication::applicationDirPath() + "/sqldrivers");

    // Verify SQLite driver is available
    if (!QSqlDatabase::isDriverAvailable("QSQLITE"))
    {
        qCritical() << "QSQLITE driver not available!";
        qCritical() << "Available drivers:" << QSqlDatabase::drivers();
        qCritical() << "Plugin paths:" << QCoreApplication::libraryPaths();
    }
    else
    {
    }

    // Load configuration and build DI container
    const AppConfig cfg = ConfigLoader::load();
    DIContainer container(cfg, &app);

    // Initialize container (DB + migrations + queries + repositories + services)
    if (!container.initialize())
    {
        qCritical() << "Failed to initialize DI container";
        return 1;
    }

    // Create repositories (now they're already created by container.initialize())
    std::shared_ptr<IPatientRepository> patientRepo = container.patientRepository();

    std::shared_ptr<IVitalsRepository> vitalsRepo = container.vitalsRepository();

    std::shared_ptr<ITelemetryRepository> telemetryRepo = container.telemetryRepository();

    std::shared_ptr<IAlarmRepository> alarmRepo = container.alarmRepository();

    // Create application service layer (already created by container.initialize())
    auto monitoringService = container.monitoringService();

    // Create interface controllers
    auto dashboardController = new DashboardController(
        monitoringService,
        container.vitalsCache().get(),
        &app);

    auto waveformController = new WaveformController(
        container.waveformCache().get(),
        &app);

    // Instantiate AdmissionService (retrieve from DI)
    auto admissionService = container.admissionService();

    auto patientController = new PatientController(
        admissionService.get(),
        &app);

    auto alarmController = new AlarmController(
        monitoringService,
        &app);

    // TrendsController: provides historical trend data via IVitalsRepository
    auto trendsController = new TrendsController(
        vitalsRepo.get(),
        &app);

    // Action log repository for audit trail
    auto actionLogRepoConcrete = std::make_shared<SQLiteActionLogRepository>(cfg.databasePath);
    std::shared_ptr<IActionLogRepository> actionLogRepo = std::static_pointer_cast<IActionLogRepository>(actionLogRepoConcrete);

    // SettingsController with logging and permissions
    auto settingsController = new SettingsController(
        actionLogRepo.get(),
        nullptr,
        &app);

    // Start monitoring service

    bool started = monitoringService->start();
    if (!started)
    {
        qWarning() << "Failed to start monitoring service";
        // Continue anyway - UI will show disconnected state
    }
    else
    {
    }

    // Start waveform rendering at 60 FPS
    waveformController->startWaveforms();

    // Create QML engine
    QQmlApplicationEngine engine;

    // Ensure QML engine can import Colors singleton module from QRC
    engine.addImportPath(":/qml");
    // Add qt_add_qml_module import path (default resource prefix)
    engine.addImportPath(":/qt/qml");

    // Register controllers as QML singletons (accessible globally in QML)
    engine.rootContext()->setContextProperty("dashboardController", dashboardController);
    engine.rootContext()->setContextProperty("waveformController", waveformController);
    engine.rootContext()->setContextProperty("patientController", patientController);
    engine.rootContext()->setContextProperty("alarmController", alarmController);
    engine.rootContext()->setContextProperty("settingsController", settingsController);
    engine.rootContext()->setContextProperty("trendsController", trendsController);

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
