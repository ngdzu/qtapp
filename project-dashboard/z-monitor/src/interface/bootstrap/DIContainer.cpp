/**
 * @file DIContainer.cpp
 */

#include "interface/bootstrap/DIContainer.h"

#include <QObject>

// Infrastructure includes
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

// Domain
#include "domain/events/DomainEventDispatcher.h"

// Application service
#include "application/services/MonitoringService.h"
#include "application/services/AdmissionService.h"
#include "application/managers/PatientManager.h"

namespace zmon
{

    DIContainer::DIContainer(const AppConfig &cfg, QObject *app)
        : m_cfg(cfg), m_app(app)
    {
        // Domain Event Dispatcher
        m_eventDispatcher = std::make_shared<DomainEventDispatcher>();

        // Sensor source
        if (m_cfg.sensorSource == SensorSourceMode::SharedMemory)
        {
            m_sensor = std::make_shared<SharedMemorySensorDataSource>(m_cfg.sharedMemorySocket, nullptr);
        }
        else
        {
            m_sensor = std::make_shared<InMemorySensorDataSource>(0, nullptr);
        }

        // Caches
        m_vitalsCache = std::make_shared<VitalsCache>(m_cfg.vitalsCacheSeconds);
        m_waveformCache = std::make_shared<WaveformCache>(m_cfg.waveformCacheSamples);

        // Database manager
        m_db = std::make_shared<DatabaseManager>(nullptr);
    }

    bool DIContainer::initialize()
    {
        auto openResult = m_db->open(m_cfg.databasePath);
        if (openResult.isError())
        {
            return false;
        }

        auto migrateResult = m_db->executeMigrations();
        if (migrateResult.isError())
        {
            return false;
        }

        zmon::persistence::QueryCatalog::initializeQueries(m_db.get());

        auto patientConcrete = std::make_shared<SQLitePatientRepository>(m_db.get());
        m_patientRepo = std::static_pointer_cast<IPatientRepository>(patientConcrete);

        auto vitalsConcrete = std::make_shared<SQLiteVitalsRepository>(m_db);
        m_vitalsRepo = std::static_pointer_cast<IVitalsRepository>(vitalsConcrete);

        auto telemetryConcrete = std::make_shared<SQLiteTelemetryRepository>(m_db);
        m_telemetryRepo = std::static_pointer_cast<ITelemetryRepository>(telemetryConcrete);

        auto alarmConcrete = std::make_shared<SQLiteAlarmRepository>(m_db);
        m_alarmRepo = std::static_pointer_cast<IAlarmRepository>(alarmConcrete);

        // Services
        m_admissionService = std::make_shared<AdmissionService>(
            nullptr, // ActionLogRepo (TODO)
            m_eventDispatcher,
            m_app);

        // Managers
        m_patientManager = std::make_shared<PatientManager>(m_admissionService, m_app);

        m_monitoringService = new MonitoringService(
            m_patientRepo,
            m_telemetryRepo,
            m_alarmRepo,
            m_vitalsRepo,
            m_sensor,
            m_vitalsCache,
            m_waveformCache,
            m_eventDispatcher,
            m_app);

        return true;
    }

    std::shared_ptr<ISensorDataSource> DIContainer::sensorDataSource() const { return m_sensor; }
    std::shared_ptr<VitalsCache> DIContainer::vitalsCache() const { return m_vitalsCache; }
    std::shared_ptr<WaveformCache> DIContainer::waveformCache() const { return m_waveformCache; }
    std::shared_ptr<DatabaseManager> DIContainer::databaseManager() const { return m_db; }
    std::shared_ptr<IPatientRepository> DIContainer::patientRepository() const { return m_patientRepo; }
    std::shared_ptr<IVitalsRepository> DIContainer::vitalsRepository() const { return m_vitalsRepo; }
    std::shared_ptr<ITelemetryRepository> DIContainer::telemetryRepository() const { return m_telemetryRepo; }
    std::shared_ptr<IAlarmRepository> DIContainer::alarmRepository() const { return m_alarmRepo; }
    std::shared_ptr<DomainEventDispatcher> DIContainer::domainEventDispatcher() const { return m_eventDispatcher; }
    std::shared_ptr<AdmissionService> DIContainer::admissionService() const { return m_admissionService; }
    std::shared_ptr<IPatientManager> DIContainer::patientManager() const { return m_patientManager; }
    MonitoringService *DIContainer::monitoringService() const { return m_monitoringService; }

} // namespace zmon
