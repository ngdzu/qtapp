# Data Caching Architecture Review

**Document ID:** DESIGN-036  
**Version:** 1.0  
**Status:** Approved  
**Last Updated:** 2025-11-27

---

---

## 1. Current Design Summary

### What You Described:
1. **Z-Monitor → Telemetry Server:** Device sends data to external telemetry server
2. **Local 7-day Cache:** Device caches 7 days of data locally, cleans up daily
3. **In-Memory Buffer:** Device has 3-day capacity in-memory cache (critical path)
4. **Periodic Persistence:** In-memory → Database (lower priority, scheduled)
5. **Database Non-Critical:** Saving to database is not time-critical
6. **Sensor Interface:** Need interface between simulator app and z-monitor

---

## 2. Design Analysis: ✅ Strengths & ⚠️ Issues

### ✅ **GOOD Design Decisions:**

1. **✅ In-Memory Critical Path**
   - **Good:** Separating critical (in-memory) from non-critical (database) operations
   - **Benefit:** Low latency for real-time monitoring and alarm detection
   - **Correct Priority:** Sensors → Memory → Alarms (< 50ms) is critical path

2. **✅ Periodic Persistence**
   - **Good:** Non-blocking database writes scheduled at low-priority times
   - **Benefit:** Doesn't interfere with real-time monitoring
   - **Smart:** Batch writes more efficient than per-record writes

3. **✅ 3-Day In-Memory Capacity**
   - **Good:** Provides buffer for network outages or database issues
   - **Benefit:** System continues operating even if persistence fails temporarily
   - **Safety Net:** Data survives short-term failures

4. **✅ 7-Day Local Storage**
   - **Good:** Meets requirement (REQ-DATA-RET-001: 7 days)
   - **Benefit:** Historical trend analysis, regulatory compliance

### ⚠️ **ISSUES & RISKS:**

#### **🔴 CRITICAL Issue #1: Missing Interface - Sensor Data Source**

**Problem:** No `ISensorDataSource` or `IVitalSignsSource` interface exists.

Currently you have:
- ✅ `IPatientLookupService` - for patient data
- ✅ `ITelemetryServer` - for sending data OUT
- ✅ `IProvisioningService` - for device setup
- ❌ **MISSING:** Interface for receiving data IN from sensors/simulator

**Current Design Flaw:**
```cpp
// CURRENT (tightly coupled):
class MonitoringService {
    DeviceSimulator* m_simulator;  // ❌ Direct dependency on simulator!
};
```

**Why This is Bad:**
- Can't swap simulator for real sensors without changing `MonitoringService`
- Can't mock sensor data for testing
- Violates Dependency Inversion Principle (DIP)
- Can't run multiple data sources (simulator + real sensors for testing)

**Solution:** Create `ISensorDataSource` interface

```cpp
/**
 * @interface ISensorDataSource
 * @brief Interface for vital signs data source (simulator or real sensors).
 */
class ISensorDataSource : public QObject {
    Q_OBJECT
public:
    virtual ~ISensorDataSource() = default;
    
    /**
     * @brief Start data acquisition.
     */
    virtual bool start() = 0;
    
    /**
     * @brief Stop data acquisition.
     */
    virtual void stop() = 0;
    
    /**
     * @brief Check if data source is active.
     */
    virtual bool isActive() const = 0;
    
signals:
    /**
     * @brief Emitted when new vital signs data available.
     * @param vitals Vital signs record
     */
    void vitalSignsReceived(const VitalRecord& vitals);
    
    /**
     * @brief Emitted when sensor connection status changes.
     */
    void connectionStatusChanged(bool connected);
    
    /**
     * @brief Emitted when sensor error occurs.
     */
    void sensorError(const QString& error);
};
```

**Implementations:**
- `SimulatorDataSource` - wraps DeviceSimulator
- `HardwareSensorAdapter` - real sensors (future)
- `MockSensorDataSource` - testing
- `ReplayDataSource` - replay recorded data

---

#### **🟠 MEDIUM Issue #2: In-Memory Cache Design Unclear**

**Problem (Resolved):** The specification for the in-memory cache structure and eviction policy is now defined and aligned with the repository and persistence services.

**Design Decisions:**
1. **Data Structure:** `std::deque<VitalRecord>` protected by `QReadWriteLock`. The deque gives O(1) append/pop for FIFO semantics and efficient range slicing for persistence jobs.
2. **Eviction Policy:** Strict FIFO. When the deque reaches `m_maxRecords` (≈2.6 M entries = 3 days @ 10 Hz), the oldest record is evicted automatically. Evictions are logged at `INFO` with cache stats for observability.
3. **Thread Safety:** Writers take the write lock for `append()` and `markAsPersisted()`. Readers take the read lock for `getRange()` and `getUnpersistedVitals()`. The cache lives on the Real-Time thread, while persistence jobs read via queued connections to avoid lock inversions.
4. **Memory Footprint:** 2.6 M records × 150 bytes ≈ 372 MB (worst case). This fits comfortably under the 512 MB RAM allocation for real-time buffers. High-frequency waveforms are handled by `WaveformCache` (Section 5) and are therefore excluded from this budget.
5. **Partial Flush:** `getUnpersistedVitals()` returns a shallow copy of unsaved records (bounded by batch size). After a successful transaction, `markAsPersisted(upToTimestamp)` trims the unsaved watermark so subsequent runs only touch new data.

**Recommendation:**

```cpp
/**
 * @class VitalsCache
 * @brief Thread-safe in-memory cache for vital signs (3-day capacity).
 */
class VitalsCache {
public:
    /**
     * @brief Add vital signs record to cache.
     * @param vital Vital signs record
     * @return true if added, false if cache full
     * 
     * @note If cache full, oldest record is evicted (FIFO)
     * @note Thread-safe (uses mutex)
     */
    bool append(const VitalRecord& vital);
    
    /**
     * @brief Get all vitals in time range.
     * @param start Start time
     * @param end End time
     * @return List of vitals in range
     * 
     * @note Thread-safe (uses read lock)
     */
    QList<VitalRecord> getRange(const QDateTime& start, const QDateTime& end);
    
    /**
     * @brief Get vitals not yet persisted to database.
     * @return List of unpersisted vitals
     * 
     * @note Thread-safe
     */
    QList<VitalRecord> getUnpersistedVitals();
    
    /**
     * @brief Mark vitals as persisted (remove from unsaved queue).
     * @param upToTimestamp Mark all vitals up to this time as saved
     * 
     * @note Thread-safe
     */
    void markAsPersisted(const QDateTime& upToTimestamp);
    
    /**
     * @brief Get cache statistics.
     */
    CacheStats getStats() const;  // count, memory usage, oldest/newest timestamps
    
private:
    QReadWriteLock m_lock;
    std::deque<VitalRecord> m_vitals;  // FIFO deque (efficient front/back ops)
    QDateTime m_oldestUnsaved;         // Track what needs persistence
    size_t m_maxRecords;               // Capacity (e.g., 2.6M for 3 days)
};
```

---

#### **🟠 MEDIUM Issue #3: Database Persistence Timing Unclear**

**Problem (Resolved):** Persistence cadence and failure handling are now codified in the `PersistenceScheduler`.

**Design Decisions:**
1. **Frequency:** Persist every 10 minutes during steady state. This interval aligns with telemetry batching (Priority 2) and keeps write amplification low.
2. **Triggers:** Immediate persistence is triggered when either (a) 10 000 unsaved records accumulate or (b) cache utilization exceeds 80 % of capacity. Both triggers are evaluated on the Database I/O thread to avoid blocking the Real-Time thread.
3. **Batch Size:** Each transaction writes at most 10 000 records via `IVitalsRepository::saveBatch()`. Larger backlogs are processed by chaining multiple batches in a single scheduler invocation to avoid starving new data.
4. **Failure Handling:** On `IVitalsRepository` failure, the scheduler logs via `m_logService->error()`, backs off exponentially (initial 30 s), and retries until success. Unsaved data remains flagged in `VitalsCache`; nothing is discarded.
5. **Shutdown:** `PersistenceScheduler::stop()` flushes all remaining unsaved records synchronously before the Database I/O thread quits. The shutdown path is part of the service lifecycle contract (see `12_THREAD_MODEL.md`).

**Recommendation:**

```cpp
/**
 * @class PersistenceScheduler
 * @brief Manages periodic persistence of in-memory cache to database.
 */
class PersistenceScheduler : public QObject {
    Q_OBJECT
public:
    /**
     * @brief Schedule: Persist every 10 minutes OR when 10,000 records accumulated.
     */
    void start();
    
signals:
    void persistenceRequested();
    void persistenceCompleted(int recordsPersisted);
    void persistenceFailed(const QString& error);

private slots:
    void onScheduledPersistence();
    void persistCache();
    
private:
    QTimer* m_timer;                    // 10-minute timer
    VitalsCache* m_cache;
    IVitalsRepository* m_repository;
    int m_batchSize = 10000;            // Max records per transaction
};
```

**Recommended Schedule:**
- **Normal Operation:** Every 10 minutes
- **High Volume:** When 10,000 unpersisted records accumulated
- **Graceful Shutdown:** Persist all remaining records before exit
- **Low Priority:** Run on Database I/O thread (not real-time thread)
- **Memory Pressure:** If in-memory cache > 80% full, trigger early persistence

---

#### **🟠 MEDIUM Issue #4: Cleanup Timing Ambiguous**

**Problem (Resolved):** The retention workflow is now explicit and deterministic across deployments.

**Design Decisions:**
1. **Retention Window:** Rolling 7-day window measured using device local time converted to UTC internally. Records with `timestamp < (nowUTC - 7 days)` are eligible for deletion, matching REQ-DATA-RET-001.
2. **Execution Time:** Cleanup runs daily at 03:00 local time (configurable). The timer lives on the Database I/O thread to reuse the same event loop as persistence and logging.
3. **Timezone Handling:** We schedule using local time but convert cutoff comparisons to UTC to avoid DST ambiguity. Devices deployed in different regions each respect their local timezone configuration.
4. **Safety & Retry:** Deletions occur in 10 000-row batches with explicit transactions. Failures log at `ERROR`, emit a `cleanupFailed()` signal, and set a retry timer for the next hourly window so we do not wait 24 hours to recover.
5. **Vacuum Policy:** SQLite `VACUUM` is **not** part of the nightly job. Instead, a quarterly maintenance task (tracked in `ZTODO.md`) runs `VACUUM` during scheduled downtime.

**Recommendation:**

```cpp
/**
 * @class DataCleanupService
 * @brief Manages 7-day data retention policy.
 */
class DataCleanupService : public QObject {
    Q_OBJECT
public:
    /**
     * @brief Schedule daily cleanup at 3 AM local time.
     * 
     * Deletes vitals older than 7 days (rolling 7-day window).
     * Runs on Database I/O thread (low priority).
     */
    void scheduleCleanup();
    
private slots:
    void performCleanup();
    
private:
    QTimer* m_dailyTimer;
    IVitalsRepository* m_repository;
    int m_retentionDays = 7;  // Configurable
};
```

**Recommended Policy:**
- **Retention:** Rolling 7-day window (e.g., delete vitals older than `now - 7 days`)
- **Timing:** 3 AM local time (low patient activity, low CPU load)
- **Batch Delete:** Delete in batches of 10,000 records (avoid long transactions)
- **Safety:** If cleanup fails, log error and retry next day (don't block operations)
- **Vacuum:** Run SQLite VACUUM quarterly (not daily - expensive operation)

---

#### **🟡 LOW Issue #5: Missing Waveform Data Strategy**

**Problem (Resolved):** Waveform handling is now isolated from the vitals pipeline to avoid exploding storage requirements.

**Design Decisions:**
1. **Storage Separation:** Waveforms live exclusively in `WaveformCache`, a dedicated circular buffer sized for 30 seconds of data at 500 Hz × 3 leads. No waveform samples enter the 3-day `VitalsCache`.
2. **Retention:** Waveforms are **display-only**. We keep 30 seconds for bedside visualization and optionally capture 10-second snapshots for alarm annotations (stored as compressed blobs linked to the alarm event). There is no 7-day waveform retention requirement.
3. **Compression:** Continuous streams are not persisted, so runtime compression is unnecessary. Snapshot payloads (when captured) use lightweight delta compression before being stored alongside alarms.
4. **Display Path:** UI widgets subscribe directly to `WaveformCache::updated()` signals on the Real-Time thread. Historical playback uses telemetry data from the central server, not the local cache.
5. **Telemetry:** Continuous waveforms are not transmitted to the telemetry server to preserve bandwidth. Only alarm-related snippets may be uploaded, per clinical workflow guidelines.

**Typical Waveform Data Volume:**
- ECG: 500 Hz × 3 leads × 2 bytes = 3 KB/second = 10.8 GB/hour
- SpO2 Pleth: 125 Hz × 2 bytes = 250 bytes/second = 21.6 MB/hour
- **Total:** ~11 GB/hour (260 GB/day!)

**Recommendation:**

```cpp
/**
 * @class WaveformCache
 * @brief Separate cache for high-frequency waveform data.
 * 
 * Waveforms have different requirements than vitals:
 * - Much higher data rate (500 Hz vs 1 Hz)
 * - Shorter retention (30 seconds for display, not 7 days)
 * - Display-only (not sent to telemetry server)
 * - Circular buffer (overwrite oldest)
 */
class WaveformCache {
public:
    /**
     * @brief Add waveform sample.
     * @note Overwrites oldest sample when buffer full
     */
    void append(const WaveformSample& sample);
    
    /**
     * @brief Get last N seconds of waveform data.
     * @param seconds Seconds of data (max 30)
     * @return Waveform samples
     */
    QList<WaveformSample> getLastSeconds(int seconds);
    
private:
    std::deque<WaveformSample> m_samples;
    size_t m_maxSamples = 15000;  // 30 seconds @ 500 Hz
};
```

**Waveform Strategy:**
- **In-Memory Only:** Keep last 30 seconds in-memory (circular buffer)
- **No Database:** Don't persist waveforms (too much data)
- **Display:** Real-time display from in-memory cache
- **Telemetry:** Don't send waveforms to server (bandwidth)
- **Snapshots:** Optionally save 10-second waveform snapshots for critical alarms

**Waveform Snapshot Regeneration:**
To support on-demand review of stored alarms, we expose a lightweight renderer that rebuilds the waveform series from the compressed snapshot blob whenever the clinician opens the alarm details view.

```cpp
/**
 * @class WaveformSnapshotRenderer
 * @brief Reconstructs waveform samples from stored snapshot blobs.
 */
class WaveformSnapshotRenderer {
public:
    explicit WaveformSnapshotRenderer(IAlarmRepository* alarmRepo);

    /**
     * @brief Decode snapshot and return display-ready samples.
     * @param snapshotId ID from alarms.context_snapshot_id
     * @return QList<WaveformSample> covering exactly the captured window
     */
    QList<WaveformSample> renderSnapshot(int snapshotId);

private:
    QByteArray decompress(const SnapshotBlob& blob) const;  // delta/RLE decode
    QList<WaveformSample> toSamples(const QByteArray& data,
                                    const SnapshotMetadata& meta) const;

    IAlarmRepository* m_alarmRepo;
};
```

**Runtime Flow:**
1. Alarm UI requests snapshot by `snapshot_id`.
2. `WaveformSnapshotRenderer` loads blob + metadata (channels, sample rate, gain) from the `snapshots` table via `IAlarmRepository`.
3. Blob is decompressed (delta/run-length) into raw samples entirely in memory; no disk files are generated.
4. Samples are fed directly to the waveform widget for playback and export.
5. Renderer is stateless and can be invoked any time as long as the snapshot row exists, satisfying the requirement to regenerate waveforms on demand.

---

## 3. Recommended Architecture

### 3.1 Component Diagram

**Mermaid Source:** [36_DATA_CACHING_COMPONENT.mmd](./36_DATA_CACHING_COMPONENT.mmd)  
**SVG Diagram:** [36_DATA_CACHING_COMPONENT.svg](./36_DATA_CACHING_COMPONENT.svg)

![Data Caching Architecture Component Diagram](./36_DATA_CACHING_COMPONENT.svg)

### 3.2 Data Flow Priority Levels

**Mermaid Source:** [36_DATA_CACHING_PRIORITY.mmd](./36_DATA_CACHING_PRIORITY.mmd)  
**SVG Diagram:** [36_DATA_CACHING_PRIORITY.svg](./36_DATA_CACHING_PRIORITY.svg)

![Data Caching Priority Levels](./36_DATA_CACHING_PRIORITY.svg)

**Priority Levels:**

- **PRIORITY 1 (CRITICAL - Real-Time Thread):** Sensor → In-Memory Cache → Alarm Evaluation | Target: < 50ms end-to-end
- **PRIORITY 2 (HIGH - Real-Time Thread):** In-Memory Cache → Telemetry Batch → Network Transmission | Target: Every 10 seconds (batched)
- **PRIORITY 3 (MEDIUM - Database Thread):** In-Memory Cache → Database Persistence | Target: Every 10 minutes (background)
- **PRIORITY 4 (LOW - Database Thread):** Database Cleanup (delete > 7 days) | Target: Daily at 3 AM

---

## 4. Missing Interfaces

### 4.1 ✅ Existing Interfaces (Correct)

1. **IPatientLookupService** - Patient demographics from HIS
2. **ITelemetryServer** - Send data TO telemetry server
3. **IProvisioningService** - Device provisioning

### 4.2 ❌ Missing Interfaces (Need to Create)

#### **4.2.1 ISensorDataSource** (CRITICAL)

**Purpose:** Abstract sensor data source (simulator vs real sensors)

**Location:** `project-dashboard/doc/z-monitor/architecture_and_design/interfaces/ISensorDataSource.md`

**Why Critical:**
- Decouples MonitoringService from DeviceSimulator
- Enables testing with mock sensors
- Supports future hardware sensor integration
- Dependency Inversion Principle (DIP)

#### **4.2.2 IVitalsRepository** (HIGH - Already Mentioned in Docs)

**Purpose:** Abstract database persistence for vitals

**Location:** `project-dashboard/doc/z-monitor/architecture_and_design/interfaces/IVitalsRepository.md`

**Why Important:**
- Decouples persistence logic from caching logic
- Enables testing with in-memory repository
- Follows Repository Pattern

#### **4.2.3 IAlarmRepository** (HIGH - Already Mentioned in Docs)

**Purpose:** Abstract alarm persistence

**Location:** `project-dashboard/doc/z-monitor/architecture_and_design/interfaces/IAlarmRepository.md`

---

## 5. Recommendations Summary

### ✅ **Keep These Design Decisions:**
1. In-memory cache as critical path
2. Periodic database persistence (non-blocking)
3. 3-day in-memory capacity
4. 7-day local storage
5. Telemetry server transmission (separate from local storage)

### 🔄 **Fix These Issues:**

| Priority | Issue | Action Required |
|----------|-------|-----------------|
| **CRITICAL** | Missing `ISensorDataSource` interface | Create interface document + implementation |
| **HIGH** | In-memory cache design incomplete | Design `VitalsCache` class with thread safety |
| **HIGH** | Persistence schedule undefined | Design `PersistenceScheduler` with clear timing |
| **MEDIUM** | Cleanup timing ambiguous | Design `DataCleanupService` with 3 AM schedule |
| **MEDIUM** | Waveform strategy missing | Design separate `WaveformCache` (30-sec circular) |

### 📋 **Action Items:**

1. **Create ISensorDataSource Interface Document**
   - Full C++ interface definition
   - SimulatorDataSource implementation
   - MockSensorDataSource implementation
   - Usage examples

2. **Update MonitoringService Design**
   - Replace `DeviceSimulator*` with `ISensorDataSource*`
   - Document in-memory caching strategy
   - Clarify thread assignments

3. **Create Caching Architecture Document**
   - VitalsCache design (3-day capacity)
   - WaveformCache design (30-second circular)
   - Thread safety strategy
   - Memory management

4. **Create Persistence Strategy Document**
   - PersistenceScheduler design (10-minute intervals)
   - DataCleanupService design (daily 3 AM)
   - Failure handling
   - Graceful shutdown

5. **Update Thread Model**
   - Clarify which components access cache
   - Document locking strategy
   - Update thread diagram

---

## 6. Estimated Storage Requirements

### 6.1 Vitals (Low-Frequency Data)

```
Assumptions:
- 1 vital record/second
- 150 bytes/record
- 24/7 operation

In-Memory (3 days):
  3 days × 86,400 sec/day = 259,200 records
  259,200 × 150 bytes = ~38.9 MB ✅ (acceptable)

Database (7 days):
  7 days × 86,400 sec/day = 604,800 records
  604,800 × 150 bytes = ~90.7 MB ✅ (< 500 MB limit)
```

### 6.2 Waveforms (High-Frequency Data)

```
Assumptions:
- ECG: 500 Hz × 3 leads × 2 bytes = 3 KB/sec
- SpO2: 125 Hz × 2 bytes = 250 bytes/sec
- Total: ~3.25 KB/sec

In-Memory (30 seconds):
  30 sec × 3.25 KB/sec = ~97.5 KB ✅ (tiny)

Database (NOT STORED):
  7 days would be: 7 × 86,400 × 3.25 KB = ~1.93 GB ❌ (too much!)
  
Recommendation: Don't persist waveforms (display-only)
```

### 6.3 Total Storage Estimate

| Data Type | In-Memory | Database (7d) | Telemetry Server |
|-----------|-----------|---------------|------------------|
| Vitals    | 39 MB     | 91 MB         | ✅ Sent          |
| Alarms    | N/A       | 5 MB          | ✅ Sent          |
| Audit Log | N/A       | 10 MB         | ❌ Local only     |
| Waveforms | 0.1 MB    | ❌ Not stored | ❌ Not sent      |
| **Total** | **~40 MB** | **~106 MB** | N/A              |

**✅ Well within constraints:**
- In-memory: 40 MB (plenty of room for 3-day buffer)
- Database: 106 MB (< 500 MB limit with 80% headroom)

---

## 7. Conclusion

### ✅ Overall Design Assessment: **GOOD with Critical Fix Needed**

**Strengths:**
- Correct separation of critical (memory) vs non-critical (database) paths
- Smart use of in-memory buffer for resilience
- Appropriate retention policies (3-day memory, 7-day database)
- Realistic storage estimates

**Critical Issue:**
- **Missing `ISensorDataSource` interface** - This MUST be created before implementation

**Recommended Next Steps:**
1. Create `ISensorDataSource` interface document (HIGH PRIORITY)
2. Design `VitalsCache` and `WaveformCache` classes
3. Define `PersistenceScheduler` and `DataCleanupService`
4. Update `MonitoringService` to use `ISensorDataSource`
5. Update thread model document with caching strategy

**Ready for Implementation:** NO - not until `ISensorDataSource` interface is designed

**Estimated Effort:** 4-6 hours to create missing documentation

---

**Status:** Analysis Complete - Recommendations Provided  
**Next Action:** Create `ISensorDataSource.md` interface document

