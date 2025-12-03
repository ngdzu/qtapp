/**
 * @file DIContainer.h
 * @brief Simple dependency injection container for application wiring.
 */
#pragma once

#include "application/config/AppConfig.h"

#include <memory>

class QObject;

namespace zmon
{

    class ISensorDataSource;
    class VitalsCache;
    class WaveformCache;
    class DatabaseManager;
    class IPatientRepository;
    class IVitalsRepository;
    class ITelemetryRepository;
    class IAlarmRepository;
    class MonitoringService;
    class TelemetryService;

    /**
     * @class DIContainer
     * @brief Central wiring point for application dependencies.
     *
     * DIContainer follows DDD boundaries to construct and expose
     * infrastructure adapters (database manager, repositories, caches,
     * sensor sources) and application services (MonitoringService,
     * TelemetryService). It is intentionally thin and contains no
     * business logic — only lifecycle and composition.
     *
     * Usage:
     * - Construct with `AppConfig` and application QObject
     * - Call `initialize()` to open DB, run migrations, register queries,
     *   and build repositories/services
     * - Retrieve components via accessors and start services as needed
     */
    class DIContainer
    {
    public:
        /**
         * @brief Construct container with configuration and app object.
         * @param cfg Application configuration (database path, sensor mode,
         *            cache sizes, shared memory socket path).
         * @param app Owning application object for QObject parenting.
         */
        DIContainer(const AppConfig &cfg, QObject *app);

        /**
         * @brief Initialize database, migrations, queries, repositories, services.
         * @return true on success, false if any step fails (open/migrate/register).
         */
        bool initialize();

        // Accessors
        /** @brief Active sensor data source (shared memory or in-memory). */
        std::shared_ptr<ISensorDataSource> sensorDataSource() const;
        /** @brief In-memory vitals cache. */
        std::shared_ptr<VitalsCache> vitalsCache() const;
        /** @brief In-memory waveform cache. */
        std::shared_ptr<WaveformCache> waveformCache() const;
        /** @brief Database manager instance (opened). */
        std::shared_ptr<DatabaseManager> databaseManager() const;
        /** @brief Patient repository (SQLite). */
        std::shared_ptr<IPatientRepository> patientRepository() const;
        /** @brief Vitals repository (SQLite, time-series optimized). */
        std::shared_ptr<IVitalsRepository> vitalsRepository() const;
        /** @brief Telemetry repository (SQLite). */
        std::shared_ptr<ITelemetryRepository> telemetryRepository() const;
        /** @brief Alarm repository (SQLite with snapshots). */
        std::shared_ptr<IAlarmRepository> alarmRepository() const;
        /** @brief Real-time monitoring service. */
        MonitoringService *monitoringService() const;
        /** @brief Telemetry batching/upload service. */
        TelemetryService *telemetryService() const;
        TelemetryService *telemetryService() const;

    private:
        AppConfig m_cfg;        ///< Configuration source for construction
        QGuiApplication *m_app; ///< Application object for QObject parenting

        std::shared_ptr<ISensorDataSource> m_sensor;           ///< Sensor source
        std::shared_ptr<VitalsCache> m_vitalsCache;            ///< Vitals cache
        std::shared_ptr<WaveformCache> m_waveformCache;        ///< Waveform cache
        std::shared_ptr<DatabaseManager> m_db;                 ///< DB manager
        std::shared_ptr<IPatientRepository> m_patientRepo;     ///< Patient repo
        std::shared_ptr<IVitalsRepository> m_vitalsRepo;       ///< Vitals repo
        std::shared_ptr<ITelemetryRepository> m_telemetryRepo; ///< Telemetry repo
        std::shared_ptr<IAlarmRepository> m_alarmRepo;         ///< Alarm repo
        MonitoringService *m_monitoringService{nullptr};       ///< Monitoring service
        TelemetryService *m_telemetryService{nullptr};         ///< Telemetry service
    };

} // namespace zmon
