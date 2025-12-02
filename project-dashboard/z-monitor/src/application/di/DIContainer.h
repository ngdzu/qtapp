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

    /**
     * @class DIContainer
     * @brief Constructs and holds core application dependencies.
     */
    class DIContainer
    {
    public:
        DIContainer(const AppConfig &cfg, QObject *app);

        /** Initialize DB and repositories. */
        bool initialize();

        // Accessors
        std::shared_ptr<ISensorDataSource> sensorDataSource() const;
        std::shared_ptr<VitalsCache> vitalsCache() const;
        std::shared_ptr<WaveformCache> waveformCache() const;
        std::shared_ptr<DatabaseManager> databaseManager() const;
        std::shared_ptr<IPatientRepository> patientRepository() const;
        std::shared_ptr<IVitalsRepository> vitalsRepository() const;
        std::shared_ptr<ITelemetryRepository> telemetryRepository() const;
        std::shared_ptr<IAlarmRepository> alarmRepository() const;
        MonitoringService *monitoringService() const;

    private:
        AppConfig m_cfg;
        QGuiApplication *m_app;

        std::shared_ptr<ISensorDataSource> m_sensor;
        std::shared_ptr<VitalsCache> m_vitalsCache;
        std::shared_ptr<WaveformCache> m_waveformCache;
        std::shared_ptr<DatabaseManager> m_db;
        std::shared_ptr<IPatientRepository> m_patientRepo;
        std::shared_ptr<IVitalsRepository> m_vitalsRepo;
        std::shared_ptr<ITelemetryRepository> m_telemetryRepo;
        std::shared_ptr<IAlarmRepository> m_alarmRepo;
        MonitoringService *m_monitoringService{nullptr};
    };

} // namespace zmon
