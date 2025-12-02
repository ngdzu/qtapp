---
doc_id: DOC-COMP-010
title: MonitoringService
version: v1.0
category: Component
subcategory: Application Layer / Use Case Orchestration
status: Draft
owner: Application Layer Team
reviewers: 
  - Architecture Team
  - Domain Layer Team
last_reviewed: 2025-01-26
next_review: 2026-01-26
related_docs:
  - DOC-ARCH-002  # System architecture
  - DOC-COMP-002  # PatientAggregate
  - DOC-COMP-004  # TelemetryBatch
  - DOC-COMP-014  # IPatientRepository
  - DOC-COMP-015  # ITelemetryRepository
  - DOC-COMP-016  # IAlarmRepository
related_tasks:
  - TASK-3B-001  # Phase 3B Migration
related_requirements:
  - REQ-MON-001  # Monitoring requirements
  - REQ-VIT-001  # Vitals processing
tags:
  - application-service
  - use-case-orchestration
  - monitoring
  - vitals
  - telemetry
  - alarms
diagram_files:
  - DOC-COMP-010_monitoringservice_dependencies.mmd
  - DOC-COMP-010_monitoringservice_dependencies.svg
---

# DOC-COMP-010: MonitoringService

## 1. Overview

**Purpose:** Application service orchestrating the monitoring use case, coordinating vitals ingestion from sensor data sources, telemetry batching for transmission, alarm evaluation, and persistence via repository interfaces.

**Responsibilities:**
- Receive vitals from ISensorDataSource (e.g., SharedMemorySensorDataSource, DeviceSimulator)
- Update PatientAggregate with incoming vitals
- Cache vitals in VitalsCache (3-day capacity, ~39 MB) and WaveformCache (30-sec capacity, ~0.1 MB)
- Evaluate alarm conditions via AlarmAggregate
- Batch telemetry data for transmission via TelemetryBatch
- Persist data via repository interfaces (IPatientRepository, ITelemetryRepository, IAlarmRepository, IVitalsRepository)
- Emit Qt signals for UI updates (vitalsUpdated, alarmRaised, telemetryBatchReady)
- Coordinate real-time processing thread operations (<5ms latency requirement)

**Layer:** Application Layer

**Module:** `z-monitor/src/application/services/MonitoringService.h`

**Thread Affinity:** Real-Time Processing Thread (high priority, ~100Hz vitals processing)

**Dependencies:**
- **Domain Aggregates:** PatientAggregate (DOC-COMP-002), TelemetryBatch (DOC-COMP-004), AlarmAggregate
- **Repository Interfaces:** IPatientRepository (DOC-COMP-014), ITelemetryRepository (DOC-COMP-015), IAlarmRepository (DOC-COMP-016), IVitalsRepository
- **Infrastructure Services:** ISensorDataSource (sensor data ingestion), VitalsCache (in-memory 3-day cache), WaveformCache (in-memory 30-sec circular buffer)
- **Value Objects:** VitalRecord, WaveformSample, AlarmSnapshot, SensorError

## 2. Architecture

<!-- TODO: Add dependencies diagram -->

**Key Design Decisions:**
- **Decision 1: Real-Time Thread Affinity** - MonitoringService runs on real-time processing thread with high priority to meet <5ms latency requirement for vital processing. Uses Qt::QueuedConnection for cross-thread communication to UI.
- **Decision 2: Repository Dependency Injection** - Depends on repository interfaces (IPatientRepository, ITelemetryRepository, etc.) rather than concrete implementations, enabling testability and infrastructure independence.
- **Decision 3: Cache-First Strategy** - Writes vitals to in-memory cache (VitalsCache) first for <5ms response time, then persists to repository via background scheduler (PersistenceScheduler) every 10 minutes.
- **Decision 4: Event-Driven Alarm Evaluation** - Evaluates alarm conditions immediately upon vital reception, raising alarmRaised signal if thresholds exceeded. AlarmAggregate maintains alarm state.

**Design Patterns Used:**
- **Application Service Pattern:** Orchestrates use case flow across domain aggregates and repositories without containing business logic
- **Dependency Injection:** Constructor injection of repository interfaces and infrastructure services for testability
- **Observer Pattern (Qt Signals/Slots):** Emits signals for UI updates (vitalsUpdated, alarmRaised) using Qt::QueuedConnection for thread-safe cross-thread communication
- **Cache-Aside Pattern:** Checks VitalsCache first, loads from repository on miss, keeps cache synchronized with persistence layer

## 3. Public API

**Interface Specification:** QObject-based Qt service with constructor injection and signal/slot communication.

### 3.1 Key Classes

```cpp
class MonitoringService : public QObject
{
    Q_OBJECT

public:
    /**
     * @brief Constructor with dependency injection.
     *
     * @param patientRepo Patient repository for persistence
     * @param telemetryRepo Telemetry repository for batch persistence
     * @param alarmRepo Alarm repository for alarm history
     * @param vitalsRepo Vitals repository for vitals history
     * @param sensorDataSource Sensor data source (SharedMemory or DeviceSimulator)
     * @param vitalsCache In-memory cache for vitals (3-day capacity)
     * @param waveformCache In-memory cache for waveforms (30-second capacity)
     * @param parent Parent QObject for Qt ownership
     */
    explicit MonitoringService(
        std::shared_ptr<IPatientRepository> patientRepo,
        std::shared_ptr<ITelemetryRepository> telemetryRepo,
        std::shared_ptr<IAlarmRepository> alarmRepo,
        std::shared_ptr<IVitalsRepository> vitalsRepo,
        std::shared_ptr<ISensorDataSource> sensorDataSource,
        std::shared_ptr<VitalsCache> vitalsCache,
        std::shared_ptr<WaveformCache> waveformCache,
        QObject *parent = nullptr);

    /**
     * @brief Destructor.
     */
    ~MonitoringService();

    // === Lifecycle Management ===

    /**
     * @brief Start monitoring.
     *
     * Starts the sensor data source and begins processing vitals.
     *
     * @return true if start succeeded, false otherwise
     */
    bool start();

    /**
     * @brief Stop monitoring.
     *
     * Stops the sensor data source and flushes pending telemetry batches.
     */
    void stop();

    // === Vital Processing ===

    /**
     * @brief Process a vital record.
     *
     * Processes a vital record: updates patient aggregate, evaluates alarms,
     * and adds to telemetry batch.
     *
     * @param vital Vital record to process
     */
    void processVital(const VitalRecord &vital);

    // === Patient Management ===

    /**
     * @brief Get current patient aggregate.
     *
     * @return Shared pointer to patient aggregate, or nullptr if no patient admitted
     */
    std::shared_ptr<PatientAggregate> getCurrentPatient() const;

    // === Alarm Management ===

    /**
     * @brief Acknowledge an alarm.
     *
     * Marks an alarm as acknowledged in the AlarmAggregate and persists to repository.
     *
     * @param alarmId Alarm identifier (UUID)
     * @param userId User ID who acknowledged the alarm
     * @return true if acknowledgment succeeded, false otherwise
     */
    bool acknowledgeAlarm(const QString &alarmId, const QString &userId);

    /**
     * @brief Silence an alarm temporarily.
     *
     * Temporarily silences an alarm in the AlarmAggregate.
     *
     * @param alarmId Alarm identifier
     * @param durationMs Silence duration in milliseconds
     * @return true if silence succeeded, false otherwise
     */
    bool silenceAlarm(const QString &alarmId, int64_t durationMs);

    /**
     * @brief Get active alarms.
     *
     * Retrieves all currently active alarms from AlarmAggregate.
     *
     * @return Vector of active alarm snapshots
     */
    std::vector<AlarmSnapshot> getActiveAlarms() const;

    /**
     * @brief Get alarm history.
     *
     * Retrieves alarm history from repository.
     *
     * @param patientMrn Patient MRN (empty for all patients)
     * @param startTimeMs Start time in milliseconds (epoch)
     * @param endTimeMs End time in milliseconds (epoch)
     * @return Vector of alarm snapshots (most recent first)
     */
    std::vector<AlarmSnapshot> getAlarmHistory(const QString &patientMrn, int64_t startTimeMs, int64_t endTimeMs) const;

signals:
    /**
     * @brief Signal emitted when a vital record is processed.
     *
     * @param vital Processed vital record
     */
    void vitalProcessed(const VitalRecord &vital);

    /**
     * @brief Signal emitted when vitals are updated (for UI controllers).
     *
     * Emitted after vital is processed and cached. Controllers should connect
     * to this signal to update Q_PROPERTY values.
     */
    void vitalsUpdated();

    /**
     * @brief Signal emitted when an alarm is raised.
     *
     * @param alarmId Alarm identifier
     * @param alarmType Alarm type (e.g., "HR_HIGH", "SPO2_LOW")
     * @param priority Alarm priority (1=high, 2=medium, 3=low)
     */
    void alarmRaised(const QString &alarmId, const QString &alarmType, int priority);

    /**
     * @brief Signal emitted when an alarm is acknowledged.
     *
     * @param alarmId Alarm identifier
     */
    void alarmAcknowledged(const QString &alarmId);

    /**
     * @brief Signal emitted when an alarm is cleared/resolved.
     *
     * @param alarmId Alarm identifier
     */
    void alarmCleared(const QString &alarmId);

    /**
     * @brief Signal emitted when a telemetry batch is ready for transmission.
     *
     * @param batchId Batch identifier
     */
    void telemetryBatchReady(const QString &batchId);

private slots:
    /**
     * @brief Slot called when sensor data source emits a vital record.
     *
     * @param vital Vital record from sensor
     */
    void onVitalReceived(const VitalRecord &vital);

    /**
     * @brief Slot called when sensor data source emits a waveform sample.
     *
     * @param sample Waveform sample from sensor
     */
    void onWaveformSampleReceived(const WaveformSample &sample);

    /**
     * @brief Slot called when sensor data source emits an error.
     *
     * @param error Sensor error
     */
    void onSensorError(const SensorError &error);

private:
    // Repository dependencies
    std::shared_ptr<IPatientRepository> m_patientRepo;
    std::shared_ptr<ITelemetryRepository> m_telemetryRepo;
    std::shared_ptr<IAlarmRepository> m_alarmRepo;
    std::shared_ptr<IVitalsRepository> m_vitalsRepo;
    
    // Infrastructure dependencies
    std::shared_ptr<ISensorDataSource> m_sensorDataSource;
    std::shared_ptr<VitalsCache> m_vitalsCache;
    std::shared_ptr<WaveformCache> m_waveformCache;

    // Domain aggregates
    std::shared_ptr<PatientAggregate> m_currentPatient;
    std::shared_ptr<AlarmAggregate> m_alarmAggregate;
    std::shared_ptr<TelemetryBatch> m_currentBatch;

    // Helper methods
    void createNewBatch();
    void flushBatch();
    void evaluateAlarms(const VitalRecord &vital);
};
```

### 3.2 Key Methods

**`start()`**
- **Purpose:** Start the sensor data source and begin processing vitals
- **Parameters:** None
- **Returns:** `bool` - true if start succeeded, false otherwise
- **Thread Safety:** Safe to call from any thread
- **Performance:** Non-blocking, starts background processing

**`processVital(vital)`**
- **Purpose:** Process a vital record: update patient aggregate, evaluate alarms, add to telemetry batch
- **Parameters:** 
  - `vital`: VitalRecord containing HR, SpO2, RR, BP, Temp, etc.
- **Returns:** `void` (signals emitted on completion)
- **Thread Safety:** Called from Real-Time Processing Thread
- **Performance:** <5ms latency requirement (cache-first strategy)
- **Side Effects:**
  - Updates PatientAggregate vitals history
  - Writes to VitalsCache and WaveformCache
  - Evaluates alarm conditions via AlarmAggregate
  - Adds to TelemetryBatch for transmission
  - Emits `vitalsUpdated()` signal

**`acknowledgeAlarm(alarmId, userId)`**
- **Purpose:** Mark an alarm as acknowledged
- **Parameters:**
  - `alarmId`: QString - Alarm identifier (UUID)
  - `userId`: QString - User ID who acknowledged the alarm
- **Returns:** `bool` - true if acknowledgment succeeded
- **Thread Safety:** Safe to call from any thread
- **Side Effects:**
  - Updates AlarmAggregate alarm state
  - Persists to IAlarmRepository
  - Emits `alarmAcknowledged(alarmId)` signal

## 4. Implementation Details

**Source File:** `z-monitor/src/application/services/MonitoringService.cpp`

**Constructor Implementation:**
```cpp
MonitoringService::MonitoringService(
    std::shared_ptr<IPatientRepository> patientRepo,
    std::shared_ptr<ITelemetryRepository> telemetryRepo,
    std::shared_ptr<IAlarmRepository> alarmRepo,
    std::shared_ptr<IVitalsRepository> vitalsRepo,
    std::shared_ptr<ISensorDataSource> sensorDataSource,
    std::shared_ptr<VitalsCache> vitalsCache,
    std::shared_ptr<WaveformCache> waveformCache,
    QObject *parent)
    : QObject(parent),
      m_patientRepo(patientRepo),
      m_telemetryRepo(telemetryRepo),
      m_alarmRepo(alarmRepo),
      m_vitalsRepo(vitalsRepo),
      m_sensorDataSource(sensorDataSource),
      m_vitalsCache(vitalsCache),
      m_waveformCache(waveformCache),
      m_alarmAggregate(std::make_shared<AlarmAggregate>())
{
    // Connect sensor data source signals
    connect(m_sensorDataSource.get(), &ISensorDataSource::vitalReceived,
            this, &MonitoringService::onVitalReceived);
    connect(m_sensorDataSource.get(), &ISensorDataSource::waveformSampleReceived,
            this, &MonitoringService::onWaveformSampleReceived);
    connect(m_sensorDataSource.get(), &ISensorDataSource::error,
            this, &MonitoringService::onSensorError);
}
```

**Vital Processing Flow:**
```cpp
void MonitoringService::processVital(const VitalRecord &vital)
{
    // 1. Update patient aggregate
    if (m_currentPatient) {
        auto result = m_currentPatient->updateVitals(vital);
        if (!result.isSuccess()) {
            qWarning() << "Failed to update patient vitals:" << result.error().message;
            return;
        }
    }

    // 2. Cache vital (< 5ms requirement)
    m_vitalsCache->append(vital);

    // 3. Evaluate alarms
    evaluateAlarms(vital);

    // 4. Add to telemetry batch
    if (m_currentBatch) {
        m_currentBatch->addVital(vital);
    }

    // 5. Emit signal for UI update
    emit vitalsUpdated();
    emit vitalProcessed(vital);
}
```

## 5. Usage Examples

### 5.1 Instantiation and Startup

```cpp
// Create dependencies
auto patientRepo = std::make_shared<SQLitePatientRepository>(dbManager);
auto telemetryRepo = std::make_shared<SQLiteTelemetryRepository>(dbManager);
auto alarmRepo = std::make_shared<SQLiteAlarmRepository>(dbManager);
auto vitalsRepo = std::make_shared<SQLiteVitalsRepository>(dbManager);
auto sensorDataSource = std::make_shared<SharedMemorySensorDataSource>();
auto vitalsCache = std::make_shared<VitalsCache>();
auto waveformCache = std::make_shared<WaveformCache>();

// Create MonitoringService
auto monitoringService = std::make_shared<MonitoringService>(
    patientRepo, telemetryRepo, alarmRepo, vitalsRepo,
    sensorDataSource, vitalsCache, waveformCache
);

// Connect signals for UI updates
connect(monitoringService.get(), &MonitoringService::vitalsUpdated,
        this, &DashboardController::onVitalsUpdated);
connect(monitoringService.get(), &MonitoringService::alarmRaised,
        this, &DashboardController::onAlarmRaised);

// Start monitoring
if (!monitoringService->start()) {
    qCritical() << "Failed to start monitoring service";
}
```

### 5.2 Alarm Acknowledgment

```cpp
// Acknowledge alarm
QString alarmId = "alarm-uuid-12345";
QString userId = "nurse-smith";

if (monitoringService->acknowledgeAlarm(alarmId, userId)) {
    qInfo() << "Alarm acknowledged:" << alarmId << "by" << userId;
} else {
    qWarning() << "Failed to acknowledge alarm:" << alarmId;
}
```

## 6. Testing

### 6.1 Unit Tests

**Test File:** `z-monitor/tests/application/services/MonitoringServiceTest.cpp`

**Key Test Cases:**
- `test_ProcessVital_UpdatesPatientAndCache()` - Verify vital processing updates patient aggregate and vitals cache
- `test_ProcessVital_EvaluatesAlarms()` - Verify alarm evaluation occurs for each vital
- `test_ProcessVital_AddsTelemetryBatch()` - Verify vital is added to telemetry batch
- `test_AcknowledgeAlarm_PersistsToRepository()` - Verify alarm acknowledgment is persisted
- `test_Start_ConnectsSensorDataSource()` - Verify sensor data source signals are connected

### 6.2 Integration Tests

**Test File:** `z-monitor/tests/integration/MonitoringServiceIntegrationTest.cpp`

**Key Integration Scenarios:**
- End-to-end vital processing with real repositories
- Alarm raised → UI signal → controller update flow
- Telemetry batch creation → persistence → transmission flow

## 7. Performance Considerations

**Real-Time Requirements:**
- **Vital Processing Latency:** <5ms from sensor to cache
- **Cache Write Performance:** O(1) append to VitalsCache circular buffer
- **Alarm Evaluation:** <1ms per vital (threshold checks only)
- **UI Signal Emission:** Qt::QueuedConnection for cross-thread safety (minimal overhead)

**Memory Management:**
- **VitalsCache:** 3-day capacity, ~39 MB (ring buffer, oldest data auto-evicted)
- **WaveformCache:** 30-second capacity, ~0.1 MB (circular buffer, high-frequency samples)
- **Shared Pointers:** Used for aggregates to prevent memory leaks with Qt parent-child ownership

**Thread Safety:**
- **Thread Affinity:** Real-Time Processing Thread (high priority)
- **Cross-Thread Communication:** Qt::QueuedConnection for UI signals (safe, non-blocking)
- **Repository Calls:** Queued to Database I/O Thread (non-blocking for real-time thread)

## 8. Error Handling

**Error Categories:**
- **Sensor Errors:** SensorError emitted by ISensorDataSource, logged and UI notified
- **Repository Errors:** Result<T> error handling, logged with error code/message
- **Alarm Evaluation Errors:** Graceful degradation, alarm evaluation skipped but vital still processed
- **Cache Overflow:** Ring buffer auto-eviction, oldest data discarded

**Recovery Strategies:**
- **Sensor Error:** Continue processing, attempt reconnection
- **Repository Write Failure:** Cache maintains data, retry on next persistence cycle
- **Alarm Repository Failure:** Log error, continue alarm evaluation (in-memory state maintained)

## 9. Security Considerations

**Authentication/Authorization:**
- Alarm acknowledgment requires valid `userId` (no permission check in service layer, delegated to UI/controller)

**Data Encryption:**
- VitalsCache and WaveformCache in-memory only (no encryption required)
- TelemetryBatch signed before transmission (handled by TelemetryBatch aggregate)

**Audit Logging:**
- Alarm acknowledgment logged to IAlarmRepository with user ID and timestamp

## 10. Deployment

**Configuration:**
- No configuration required (dependencies injected via constructor)

**Dependencies:**
- IPatientRepository, ITelemetryRepository, IAlarmRepository, IVitalsRepository (infrastructure layer)
- ISensorDataSource (SharedMemorySensorDataSource or DeviceSimulator)
- VitalsCache, WaveformCache (infrastructure layer)

**Thread Affinity:**
- Must run on Real-Time Processing Thread (high priority, ~100Hz vitals processing)

**Integration Points:**
- UI Controllers: Connect to `vitalsUpdated()`, `alarmRaised()` signals
- Persistence Scheduler: Triggered via `telemetryBatchReady()` signal

## 11. Related Documentation

**Domain Layer:**
- DOC-COMP-002: PatientAggregate - Patient admission lifecycle
- DOC-COMP-004: TelemetryBatch - Telemetry data batching

**Repository Interfaces:**
- DOC-COMP-014: IPatientRepository - Patient persistence interface
- DOC-COMP-015: ITelemetryRepository - Telemetry persistence interface
- DOC-COMP-016: IAlarmRepository - Alarm persistence interface

**Architecture:**
- DOC-ARCH-002: System Architecture - Application Layer overview
- Thread Model Documentation - Real-Time Processing Thread design

## 12. Changelog

| Version | Date       | Author      | Changes                                        |
| ------- | ---------- | ----------- | ---------------------------------------------- |
| v1.0    | 2025-01-26 | Dustin Wind | Initial documentation from MonitoringService.h |
