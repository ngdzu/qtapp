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
#include <QStandardPaths>
#include <QUrl>
#include <memory>

// Application configuration & DI
#include "application/config/ConfigLoader.h"
#include "interface/bootstrap/DIContainer.h"

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

// Interface
#include "interface/controllers/DashboardController.h"
#include "interface/controllers/WaveformController.h"
#include "interface/controllers/PatientController.h"
#include "interface/controllers/AlarmController.h"
#include "interface/controllers/SettingsController.h"
#include "interface/controllers/TrendsController.h"

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

    // Add executable directory to plugin search path
    // Qt will automatically append "/sqldrivers" when searching for SQL plugins
    // MUST be done before any QSqlDatabase operations
    QCoreApplication::addLibraryPath(QCoreApplication::applicationDirPath());
    // Also add explicit sqldrivers subdir to be safe
    QCoreApplication::addLibraryPath(QCoreApplication::applicationDirPath() + "/sqldrivers");
    qInfo() << "Qt library paths:" << QCoreApplication::libraryPaths();
    qInfo() << "Available SQL drivers:" << QSqlDatabase::drivers();

    // Verify SQLite driver is available
    if (!QSqlDatabase::isDriverAvailable("QSQLITE"))
    {
        qCritical() << "QSQLITE driver not available!";
        qCritical() << "Available drivers:" << QSqlDatabase::drivers();
        qCritical() << "Plugin paths:" << QCoreApplication::libraryPaths();
    }
    else
    {
        qInfo() << "QSQLITE driver is available";
    }

    // Load configuration and build DI container
    const zmon::AppConfig cfg = zmon::ConfigLoader::load();
    zmon::DIContainer container(cfg, &app);

    // Initialize container (DB + migrations + queries + repositories)
    auto openResult = container.databaseManager()->open(cfg.databasePath);
    if (openResult.isError())
    {
        qCritical() << "Failed to open database:" << QString::fromStdString(openResult.error().message);
        // Continue anyway - repositories will handle closed database gracefully
    }
    else
    {
        qInfo() << "Database opened successfully at:" << cfg.databasePath;
    }

    // Run migrations to create schema (if not already created)
    auto migrateResult = container.databaseManager()->executeMigrations();
    if (migrateResult.isError())
    {
        qCritical() << "Failed to run migrations:" << QString::fromStdString(migrateResult.error().message);
    }
    else
    {
        qInfo() << "Database migrations executed successfully";
    }

    // Initialize all prepared queries (REQUIRED before using any repository)
    zmon::persistence::QueryCatalog::initializeQueries(container.databaseManager().get());
    qInfo() << "Database queries initialized";

    // Create repositories
    std::shared_ptr<zmon::IPatientRepository> patientRepo = container.patientRepository();
    std::shared_ptr<zmon::IVitalsRepository> vitalsRepo = container.vitalsRepository();
    std::shared_ptr<zmon::ITelemetryRepository> telemetryRepo = container.telemetryRepository();
    std::shared_ptr<zmon::IAlarmRepository> alarmRepo = container.alarmRepository();

    // Create application service layer
    auto monitoringService = container.monitoringService();

    // Create interface controllers
    auto dashboardController = new zmon::DashboardController(
        monitoringService,
        container.vitalsCache().get(),
        &app);

    auto waveformController = new zmon::WaveformController(
        container.waveformCache().get(),
        &app);

    // Instantiate AdmissionService (placeholder: retrieve from DI if available)
    // For now, construct with nullptr for action log repo
    auto admissionService = new zmon::AdmissionService(nullptr, &app);
    auto patientController = new zmon::PatientController(
        admissionService,
        &app);

    auto alarmController = new zmon::AlarmController(
        monitoringService,
        &app);

    // TrendsController: provides historical trend data via IVitalsRepository
    auto trendsController = new zmon::TrendsController(
        vitalsRepo.get(),
        &app);

    // Action log repository for audit trail
    auto actionLogRepoConcrete = std::make_shared<zmon::SQLiteActionLogRepository>(cfg.databasePath);
    std::shared_ptr<zmon::IActionLogRepository> actionLogRepo = std::static_pointer_cast<zmon::IActionLogRepository>(actionLogRepoConcrete);

    // SettingsController with logging and permissions
    auto settingsController = new zmon::SettingsController(
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

    // Start waveform rendering at 60 FPS
    waveformController->startWaveforms();

    // Create QML engine
    QQmlApplicationEngine engine;

    // Ensure QML engine can import resources under qrc:/qml
    engine.addImportPath("qrc:/qml");

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
