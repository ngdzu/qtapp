# Task: Wire SharedMemorySensorDataSource to MonitoringService and Controllers

## Context

**Documentation:**
- Architecture: [37_SENSOR_INTEGRATION.md](../doc/z-monitor/architecture_and_design/37_SENSOR_INTEGRATION.md) - Integration checklist
- Implementation Guide: [44_SIMULATOR_INTEGRATION_GUIDE.md](../doc/z-monitor/architecture_and_design/44_SIMULATOR_INTEGRATION_GUIDE.md) - Phase 4
- Data Caching: [36_DATA_CACHING_STRATEGY.md](../doc/z-monitor/architecture_and_design/36_DATA_CACHING_STRATEGY.md)
- Thread Model: [12_THREAD_MODEL.md](../doc/z-monitor/architecture_and_design/12_THREAD_MODEL.md)

**Previous Work:**
- ✅ Simulator built and running
- ✅ SharedMemorySensorDataSource implemented and tested
- ⏳ Need to integrate with MonitoringService and controllers

**Dependencies:**
- `SharedMemorySensorDataSource` implemented and emitting signals
- `MonitoringService` exists with basic structure
- Controllers exist (DashboardController, WaveformController, TrendsController)
- `WaveformCache` and `VitalsCache` infrastructure (may need implementation)

---

## Objective

Integrate `SharedMemorySensorDataSource` into z-monitor's application layer. Update `MonitoringService` to instantiate and connect to `SharedMemorySensorDataSource`, receive real-time vitals and waveform data via Qt signals, update in-memory cache (`WaveformCache`, `VitalsCache`), and propagate data to UI controllers. Update controllers to expose live data as Q_PROPERTY for QML binding. Verify data flows from simulator → SharedMemorySensorDataSource → MonitoringService → Controllers → QML UI with < 50ms total latency.

---

## Architecture Overview

### Data Flow

```
Sensor Simulator
  ↓ (shared memory @ 60 Hz / 250 Hz)
SharedMemorySensorDataSource
  ↓ (Qt signals: vitalsReceived, waveformSampleReady)
MonitoringService
  ├─ VitalsCache::append() (in-memory, 3-day capacity)
  ├─ WaveformCache::append() (circular buffer, 30-second display-only)
  ├─ AlarmManager::processVitalSigns() (alarm detection)
  └─ TelemetryBatch::addVital() (network transmission queue)
  ↓ (Qt signals: vitalsUpdated, waveformUpdated)
Controllers (DashboardController, WaveformController)
  ↓ (Q_PROPERTY updates)
QML UI (VitalTile, WaveformPanel)
```

**Latency Budget:**
- Simulator → SharedMemorySensorDataSource: < 16ms
- MonitoringService processing: < 15ms
  - VitalsCache::append(): < 2ms
  - AlarmManager::processVitals(): < 8ms
  - UI update: < 3ms
  - Telemetry batch: < 2ms
- **Total: < 50ms** (simulator write → UI visible)

---

## Component Breakdown

### Component 1: VitalsCache

**Purpose:** In-memory cache for 3-day vitals capacity (~39 MB).

**File:** `z-monitor/src/infrastructure/caching/VitalsCache.h`

**Requirements (from 36_DATA_CACHING_STRATEGY.md):**
- Thread-safe (QReadWriteLock)
- 3-day capacity: ~259,200 records (60 Hz × 60 sec × 60 min × 24 hr × 3 days)
- Memory estimate: ~39 MB (150 bytes per record)
- Methods:
  - `append(VitalRecord)` - Add vital to cache
  - `getRange(start, end)` - Get vitals in time range
  - `getUnpersistedVitals()` - Get vitals not yet saved to database
  - `markAsPersisted(upToTimestamp)` - Mark vitals as saved

**Implementation:**
```cpp
class VitalsCache {
public:
    VitalsCache();
    
    void append(const VitalRecord& vital);
    QVector<VitalRecord> getRange(const QDateTime& start, const QDateTime& end) const;
    QVector<VitalRecord> getUnpersistedVitals() const;
    void markAsPersisted(const QDateTime& upToTimestamp);
    
    size_t size() const;
    void clear();
    
private:
    mutable QReadWriteLock m_lock;
    QVector<VitalRecord> m_vitals;
    QDateTime m_lastPersistedTimestamp;
    size_t m_maxCapacity;  // 259,200 records
};
```

---

### Component 2: WaveformCache

**Purpose:** Circular buffer for 30 seconds of waveform data (~0.1 MB), display-only.

**File:** `z-monitor/src/infrastructure/caching/WaveformCache.h`

**Requirements:**
- Circular buffer (overwrites oldest)
- 30 seconds × 250 Hz = 7,500 samples per channel
- Channels: ECG, Pleth, Respiration
- Total: ~22,500 samples (~0.1 MB)
- Not persisted to database (display-only)

**Implementation:**
```cpp
class WaveformCache {
public:
    WaveformCache(size_t capacity = 22500);
    
    void append(const WaveformSample& sample);
    QVector<WaveformSample> getLastSeconds(int seconds) const;
    QVector<WaveformSample> getChannelSamples(const QString& channel, int seconds) const;
    
    void clear();
    
private:
    mutable QReadWriteLock m_lock;
    QVector<WaveformSample> m_samples;
    size_t m_capacity;
    size_t m_writeIndex;  // Circular buffer index
};
```

---

### Component 3: MonitoringService Integration

**Purpose:** Orchestrate data flow from sensor to caches to controllers.

**File:** `z-monitor/src/application/services/MonitoringService.h`

**Updated Class:**
```cpp
class MonitoringService : public QObject {
    Q_OBJECT
public:
    explicit MonitoringService(ISensorDataSource* sensorDataSource,
                               VitalsCache* vitalsCache,
                               WaveformCache* waveformCache,
                               AlarmManager* alarmManager,
                               QObject* parent = nullptr);
    ~MonitoringService() override;
    
    void start();
    void stop();
    bool isActive() const;
    
signals:
    void vitalsUpdated(const VitalRecord& vital);
    void waveformUpdated(const WaveformSample& sample);
    void sensorStatusChanged(bool connected, const QString& sensorType);
    void error(const QString& errorMessage);
    
private slots:
    void onVitalsReceived(const VitalRecord& vital);
    void onWaveformSampleReady(const WaveformSample& sample);
    void onSensorError(const SensorError& error);
    void onSensorStatusChanged(bool connected, const QString& sensorType);
    
private:
    ISensorDataSource* m_sensorDataSource;
    VitalsCache* m_vitalsCache;
    WaveformCache* m_waveformCache;
    AlarmManager* m_alarmManager;
    bool m_active;
};
```

---

### Component 4: DashboardController Updates

**Purpose:** Expose vitals as Q_PROPERTY for QML binding.

**File:** `z-monitor/src/interface/controllers/DashboardController.h`

**Updated Class:**
```cpp
class DashboardController : public QObject {
    Q_OBJECT
    Q_PROPERTY(int heartRate READ heartRate NOTIFY vitalsChanged)
    Q_PROPERTY(int spo2 READ spo2 NOTIFY vitalsChanged)
    Q_PROPERTY(int respirationRate READ respirationRate NOTIFY vitalsChanged)
    Q_PROPERTY(QString nibp READ nibp NOTIFY vitalsChanged)
    Q_PROPERTY(double temperature READ temperature NOTIFY vitalsChanged)
    Q_PROPERTY(QDateTime lastUpdate READ lastUpdate NOTIFY vitalsChanged)
    
public:
    explicit DashboardController(MonitoringService* monitoringService, QObject* parent = nullptr);
    
    int heartRate() const { return m_heartRate; }
    int spo2() const { return m_spo2; }
    int respirationRate() const { return m_respirationRate; }
    QString nibp() const { return m_nibp; }
    double temperature() const { return m_temperature; }
    QDateTime lastUpdate() const { return m_lastUpdate; }
    
signals:
    void vitalsChanged();
    
private slots:
    void onVitalsUpdated(const VitalRecord& vital);
    
private:
    MonitoringService* m_monitoringService;
    int m_heartRate;
    int m_spo2;
    int m_respirationRate;
    QString m_nibp;
    double m_temperature;
    QDateTime m_lastUpdate;
};
```

---

### Component 5: WaveformController Updates

**Purpose:** Expose waveform data as Q_PROPERTY for QML Canvas rendering.

**File:** `z-monitor/src/interface/controllers/WaveformController.h`

**Updated Class:**
```cpp
class WaveformController : public QObject {
    Q_OBJECT
    Q_PROPERTY(QVariantList ecgSamples READ ecgSamples NOTIFY waveformChanged)
    Q_PROPERTY(QVariantList plethSamples READ plethSamples NOTIFY waveformChanged)
    Q_PROPERTY(QVariantList respirationSamples READ respirationSamples NOTIFY waveformChanged)
    
public:
    explicit WaveformController(MonitoringService* monitoringService,
                                WaveformCache* waveformCache,
                                QObject* parent = nullptr);
    
    QVariantList ecgSamples() const;
    QVariantList plethSamples() const;
    QVariantList respirationSamples() const;
    
signals:
    void waveformChanged();
    
private slots:
    void onWaveformUpdated(const WaveformSample& sample);
    void updateWaveformBuffer();
    
private:
    MonitoringService* m_monitoringService;
    WaveformCache* m_waveformCache;
    QTimer* m_updateTimer;  // 60 FPS update (16ms)
};
```

---

## Implementation Steps

### Step 1: Implement VitalsCache

**File:** `z-monitor/src/infrastructure/caching/VitalsCache.cpp`

```cpp
VitalsCache::VitalsCache()
    : m_maxCapacity(259200)  // 3 days at 60 Hz
{
    m_vitals.reserve(m_maxCapacity);
}

void VitalsCache::append(const VitalRecord& vital) {
    QWriteLocker lock(&m_lock);
    
    // Evict old records if at capacity
    if (m_vitals.size() >= m_maxCapacity) {
        // Remove oldest 10% to avoid frequent evictions
        m_vitals.remove(0, m_maxCapacity / 10);
    }
    
    m_vitals.append(vital);
}

QVector<VitalRecord> VitalsCache::getRange(const QDateTime& start, const QDateTime& end) const {
    QReadLocker lock(&m_lock);
    
    QVector<VitalRecord> result;
    for (const auto& vital : m_vitals) {
        if (vital.timestamp >= start && vital.timestamp <= end) {
            result.append(vital);
        }
    }
    return result;
}
```

---

### Step 2: Implement WaveformCache

**File:** `z-monitor/src/infrastructure/caching/WaveformCache.cpp`

```cpp
WaveformCache::WaveformCache(size_t capacity)
    : m_capacity(capacity)
    , m_writeIndex(0)
{
    m_samples.resize(m_capacity);
}

void WaveformCache::append(const WaveformSample& sample) {
    QWriteLocker lock(&m_lock);
    
    // Circular buffer - overwrite oldest
    m_samples[m_writeIndex % m_capacity] = sample;
    m_writeIndex++;
}

QVector<WaveformSample> WaveformCache::getLastSeconds(int seconds) const {
    QReadLocker lock(&m_lock);
    
    size_t sampleCount = std::min(static_cast<size_t>(seconds * 250), m_writeIndex);
    QVector<WaveformSample> result;
    result.reserve(sampleCount);
    
    size_t startIdx = m_writeIndex - sampleCount;
    for (size_t i = startIdx; i < m_writeIndex; i++) {
        result.append(m_samples[i % m_capacity]);
    }
    return result;
}
```

---

### Step 3: Update MonitoringService

**File:** `z-monitor/src/application/services/MonitoringService.cpp`

**Constructor:**
```cpp
MonitoringService::MonitoringService(ISensorDataSource* sensorDataSource,
                                     VitalsCache* vitalsCache,
                                     WaveformCache* waveformCache,
                                     AlarmManager* alarmManager,
                                     QObject* parent)
    : QObject(parent)
    , m_sensorDataSource(sensorDataSource)
    , m_vitalsCache(vitalsCache)
    , m_waveformCache(waveformCache)
    , m_alarmManager(alarmManager)
    , m_active(false)
{
    // Connect signals from sensor data source
    connect(m_sensorDataSource, &ISensorDataSource::vitalsReceived,
            this, &MonitoringService::onVitalsReceived);
    connect(m_sensorDataSource, &ISensorDataSource::waveformSampleReady,
            this, &MonitoringService::onWaveformSampleReady);
    connect(m_sensorDataSource, &ISensorDataSource::connectionStatusChanged,
            this, &MonitoringService::onSensorStatusChanged);
    connect(m_sensorDataSource, &ISensorDataSource::sensorError,
            this, &MonitoringService::onSensorError);
}
```

**start() Method:**
```cpp
void MonitoringService::start() {
    if (m_active) {
        qWarning() << "MonitoringService: Already active";
        return;
    }
    
    if (!m_sensorDataSource->start()) {
        qCritical() << "MonitoringService: Failed to start sensor data source";
        emit error("Failed to start sensor data source");
        return;
    }
    
    m_active = true;
    qInfo() << "MonitoringService: Started successfully";
}
```

**onVitalsReceived() Slot:**
```cpp
void MonitoringService::onVitalsReceived(const VitalRecord& vital) {
    // CRITICAL PATH - Keep this fast (< 15ms)
    
    // 1. Append to in-memory cache (< 2ms)
    m_vitalsCache->append(vital);
    
    // 2. Evaluate alarm rules (< 8ms)
    if (m_alarmManager) {
        m_alarmManager->processVitalSigns(vital);
    }
    
    // 3. Emit signal to UI controllers (< 1ms)
    emit vitalsUpdated(vital);
    
    // 4. Enqueue to telemetry batch (< 2ms)
    // (Background thread handles network transmission)
    enqueueTelemetry(vital);
}
```

**onWaveformSampleReady() Slot:**
```cpp
void MonitoringService::onWaveformSampleReady(const WaveformSample& sample) {
    // Append to waveform cache (display-only, circular buffer)
    m_waveformCache->append(sample);
    
    // Emit signal to WaveformController
    emit waveformUpdated(sample);
}
```

---

### Step 4: Update DashboardController

**File:** `z-monitor/src/interface/controllers/DashboardController.cpp`

**Constructor:**
```cpp
DashboardController::DashboardController(MonitoringService* monitoringService, QObject* parent)
    : QObject(parent)
    , m_monitoringService(monitoringService)
    , m_heartRate(0)
    , m_spo2(0)
    , m_respirationRate(0)
    , m_nibp("--/--")
    , m_temperature(0.0)
{
    // Connect to MonitoringService signals
    connect(m_monitoringService, &MonitoringService::vitalsUpdated,
            this, &DashboardController::onVitalsUpdated);
}
```

**onVitalsUpdated() Slot:**
```cpp
void DashboardController::onVitalsUpdated(const VitalRecord& vital) {
    // Update Q_PROPERTY values (QML binds to these)
    m_heartRate = vital.heartRate;
    m_spo2 = vital.spo2;
    m_respirationRate = vital.respirationRate;
    m_lastUpdate = QDateTime::currentDateTime();
    
    // Trigger QML UI update
    emit vitalsChanged();
}
```

---

### Step 5: Update WaveformController

**File:** `z-monitor/src/interface/controllers/WaveformController.cpp`

```cpp
WaveformController::WaveformController(MonitoringService* monitoringService,
                                       WaveformCache* waveformCache,
                                       QObject* parent)
    : QObject(parent)
    , m_monitoringService(monitoringService)
    , m_waveformCache(waveformCache)
{
    // Update buffer at 60 FPS (16ms interval)
    m_updateTimer = new QTimer(this);
    m_updateTimer->setInterval(16);
    connect(m_updateTimer, &QTimer::timeout, this, &WaveformController::updateWaveformBuffer);
    m_updateTimer->start();
}

QVariantList WaveformController::ecgSamples() const {
    QVector<WaveformSample> samples = m_waveformCache->getChannelSamples("ecg", 10);
    
    QVariantList result;
    for (const auto& sample : samples) {
        result.append(sample.value);
    }
    return result;
}

void WaveformController::updateWaveformBuffer() {
    // Trigger QML update
    emit waveformChanged();
}
```

---

## Testing

### Integration Test

**File:** `z-monitor/tests/integration/monitoring_service_sensor_integration_test.cpp`

```cpp
TEST(MonitoringService, ReceivesVitalsFromSharedMemory) {
    // Setup
    SharedMemoryTestHarness harness;
    SharedMemorySensorDataSource* dataSource =
        new SharedMemorySensorDataSource(harness.socketPath(), harness.ringName());
    VitalsCache cache;
    WaveformCache waveformCache;
    AlarmManager alarmMgr;
    MonitoringService service(dataSource, &cache, &waveformCache, &alarmMgr);
    
    // Act
    service.start();
    harness.writeFrames(120);  // 2 seconds of data at 60 Hz
    QTest::qWait(2100);  // Wait for processing
    
    // Assert
    EXPECT_GT(cache.size(), 100);  // Should have > 100 vitals
    
    // Cleanup
    service.stop();
}
```

---

## Troubleshooting

### No Data in UI

**Symptoms:** MonitoringService receives data but controllers don't update.

**Solution:**
1. Verify signal connections (MonitoringService → Controllers)
2. Check Q_PROPERTY NOTIFY signals are emitted
3. Add debug logging to controller slots

---

### High Latency

**Symptoms:** Latency > 50ms from simulator to UI.

**Solution:**
1. Profile MonitoringService::onVitalsReceived() (should be < 15ms)
2. Optimize cache operations (pre-allocate, avoid copies)
3. Check Qt signal/slot overhead (use direct connections if needed)

---

## Acceptance Criteria

- [ ] `MonitoringService` successfully instantiates and starts `SharedMemorySensorDataSource`
- [ ] Vitals data flows: Simulator → SensorDataSource → MonitoringService → VitalsCache → Controllers
- [ ] Waveform data flows: Simulator → SensorDataSource → MonitoringService → WaveformCache → Controllers
- [ ] Controllers expose data via Q_PROPERTY (QML can bind)
- [ ] Connection status visible in UI (connected, disconnected, stalled)
- [ ] Error handling works (connection errors, stalls, invalid data)
- [ ] Total latency (simulator write → UI update) < 50ms measured

---

## Verification Checklist

### 1. Functional
- [ ] Data appears in QML UI (vitals tiles update, waveforms render)
- [ ] Values match simulator output
- [ ] Connection status accurate
- [ ] Error states handled correctly

### 2. Code Quality
- [ ] Proper signal/slot connections
- [ ] No memory leaks
- [ ] Thread safety verified (if applicable)
- [ ] Doxygen comments updated

### 3. Documentation
- [ ] Data flow documented in `11_DATA_FLOW_AND_CACHING.md`
- [ ] Controller bindings documented

### 4. Integration
- [ ] End-to-end test with simulator shows live data in UI
- [ ] Latency measured (< 50ms)
- [ ] Caches work correctly

### 5. Tests
- [ ] Integration tests for MonitoringService + SharedMemorySensorDataSource
- [ ] Controller unit tests with mock data sources

---

## Latency Measurement

Add instrumentation to measure end-to-end latency:

```cpp
// In simulator: Add write timestamp
frame.timestampNs = QDateTime::currentMSecsSinceEpoch() * 1000000;

// In DashboardController: Measure latency
void DashboardController::onVitalsUpdated(const VitalRecord& vital) {
    auto nowNs = QDateTime::currentMSecsSinceEpoch() * 1000000;
    auto latencyMs = (nowNs - vital.timestampNs) / 1000000.0;
    
    if (latencyMs > 50.0) {
        qWarning() << "High latency:" << latencyMs << "ms";
    }
}
```

**Target:** < 50ms total (< 16ms simulator→signal, < 34ms signal→UI)

---

## Next Steps

After integration complete:
1. Proceed to **44e-update-qml-ui-live-data.md** (QML UI binding and Canvas rendering)
2. Verify live data displays correctly
3. Test with different data patterns (normal, alarms, edge cases)

---

**Estimated Time:** 2-3 hours (includes implementation, testing, debugging)
