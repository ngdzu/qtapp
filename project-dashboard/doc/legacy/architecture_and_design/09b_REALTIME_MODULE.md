# Real-Time Processing Module: Class Designs

**Document ID:** DESIGN-009b  
**Version:** 1.0  
**Status:** Approved  
**Last Updated:** 2025-11-27

---

This document provides detailed class designs for the **Real-Time Processing Module**, which handles the critical path for sensor data processing and alarm detection on the RT Thread.

> **📋 Related Documents:**
> - [Class Designs Overview (09_CLASS_DESIGNS_OVERVIEW.md)](./09_CLASS_DESIGNS_OVERVIEW.md) - High-level module architecture
> - [Thread Model (12_THREAD_MODEL.md)](./12_THREAD_MODEL.md) - Thread architecture (Section 4.2: Real-Time Processing Thread)
> - [Data Caching Strategy (36_DATA_CACHING_STRATEGY.md)](./36_DATA_CACHING_STRATEGY.md) - Caching architecture
> - [Sensor Integration (37_SENSOR_INTEGRATION.md)](./37_SENSOR_INTEGRATION.md) - Sensor data source interface

---

## 1. Module Overview

**Thread:** Real-Time Processing Thread  
**Priority:** High/Real-Time (`SCHED_FIFO` on Linux, QoS User Interactive on macOS)  
**Component Count:** 12 components

**Purpose:**
- Receive sensor data at high frequency (250 Hz waveforms, 5 Hz vitals)
- Cache vitals in-memory for fast alarm detection
- Evaluate alarm conditions (< 50ms latency target)
- Build telemetry batches for transmission

**Critical Path:** Sensor → Cache → Alarm Detection → UI (< 50ms)

---

## 2. Module Diagram

[View Real-Time Module Diagram (Mermaid)](./09b_REALTIME_MODULE.mmd)  
[View Real-Time Module Diagram (SVG)](./09b_REALTIME_MODULE.svg)

---

## 3. Infrastructure Components

### 3.1. SharedMemorySensorDataSource

**Responsibility:** Maps the simulator's shared-memory ring buffer (memfd) and streams 60 Hz vitals / 250 Hz waveform batches with microsecond latency.

**Thread:** Real-Time Processing Thread

**Interface:** `ISensorDataSource`

**Key Properties:**
- `socketPath`: `QString` – Unix-domain control socket path (`unix://run/zmonitor-sim.sock`)
- `ringName`: `QString` – Shared-memory ring identifier (`zmonitor-sim-ring`)
- `readIndex`: `uint32_t` – Reader position within the ring buffer
- `heartbeatTimeoutMs`: `int` – Stall threshold (default 250 ms)

**Key Methods:**
- `start()`: Connects to control socket, receives `memfd`, `mmap`s header + slots, and primes polling loop.
- `stop()`: Unmaps shared memory and closes control socket.
- `pollFrames()`: Checks `writeIndex`, copies new frames into scratch buffer, emits vitals/waveform signals.
- `handleHeartbeat()`: Updates internal watchdog; emits `sensorError` if heartbeat stalls.
- `isActive()`: Returns whether data source currently has a valid mapped buffer.

**Signals:**
- `vitalSignsReceived(const VitalSigns& vitals)` @ 60 Hz.
- `waveformSamplesReceived(const QList<WaveformSample>& samples)` @ 250 Hz (batched).
- `connectionStatusChanged(ConnectionStatus status)` – Reflects control socket + heartbeat state.
- `errorOccurred(const QString& error)` – Corruption, stall, or control-channel failure.

**Data Flow:**
```
Sensor Simulator (shared-memory writer)
    ↓ Shared Memory Frame
SharedMemorySensorDataSource::pollFrames()
    ↓ Decode binary payload
    ↓ Emit signal
MonitoringService::onVitalSignsReceived() (same thread, direct call)
```

**Dependencies:**
- POSIX shared memory + `memfd_create`
- Unix-domain sockets for descriptor passing
- `MonitoringService` (consumes vitals/waveform signals)

**See:** [37_SENSOR_INTEGRATION.md](./37_SENSOR_INTEGRATION.md) for the shared-memory layout and handshake details.

---

### 3.2. DeviceSimulator (LEGACY)

**Responsibility:** **LEGACY FALLBACK** - Generates realistic simulated patient vital signs internally (replaced by `SharedMemorySensorDataSource` in production).

**Thread:** Real-Time Processing Thread

**Status:** Deprecated - Use `SharedMemorySensorDataSource` for production. Kept for fallback when external simulator unavailable.

**Key Methods:**
- `start()`: Begins the simulation timer
- `stop()`: Stops the simulation timer

**Signals:**
- `newData(const PatientData& data)`: Emitted when new simulated data is available

---

### 3.3. VitalsCache

**Responsibility:** In-memory cache for vitals data (3-day capacity, ~39 MB) for critical path alarm detection.

**Thread:** Real-Time Processing Thread

**Key Properties:**
- `maxCapacity`: `int` - Maximum number of records (3 days × 5 Hz × 86400 seconds = ~1.3M records)
- `currentSize`: `int` - Current number of records
- `memoryUsageMB`: `double` - Current memory usage in MB

**Key Methods:**
- `append(const VitalRecord& vital)`: Appends vital to cache (thread-safe with QReadWriteLock)
  - **Latency Target:** < 5ms **CRITICAL**
  - Automatically removes oldest records when capacity exceeded
  - **Returns:** `void` (non-blocking)
- `getRange(const QDateTime& startTime, const QDateTime& endTime)`: Retrieves vitals in time range
  - **Latency Target:** < 10ms
  - Returns `QList<VitalRecord>` (copy, safe for cross-thread access)
- `getLatest(int count)`: Retrieves latest N vitals
  - **Latency Target:** < 5ms
- `clear()`: Clears cache (used when patient discharged)
- `getMemoryUsageMB()`: Returns current memory usage

**Thread Safety:**
- Uses `QReadWriteLock` for thread-safe access
- Multiple readers (alarm detection, UI queries) can access concurrently
- Single writer (append operations) acquires write lock

**Memory Management:**
- Circular buffer behavior (oldest records removed when full)
- Pre-allocated memory (no heap allocations in critical path)
- Memory usage: ~39 MB for 3-day cache (1.3M records × 30 bytes/record)

**Dependencies:**
- `VitalRecord` - Value object for vitals data
- `QReadWriteLock` - Thread synchronization

**See:** [36_DATA_CACHING_STRATEGY.md](./36_DATA_CACHING_STRATEGY.md) for complete caching architecture.

---

### 3.4. WaveformCache

**Responsibility:** Circular buffer for waveform samples (30 seconds, ~0.1 MB) for display-only rendering.

**Thread:** Real-Time Processing Thread

**Key Properties:**
- `maxCapacity`: `int` - Maximum number of samples (30 seconds × 250 Hz = 7,500 samples)
- `currentSize`: `int` - Current number of samples

**Key Methods:**
- `append(const QList<WaveformSample>& samples)`: Appends waveform samples to buffer
  - Automatically removes oldest samples when capacity exceeded
  - **Latency Target:** < 1ms (non-critical, display-only)
- `getWindow(int windowSize)`: Retrieves last N samples for display
  - `windowSize`: Number of samples (e.g., 2,500 for 10-second display)
  - Returns `QList<WaveformSample>` (copy)
- `clear()`: Clears buffer

**Memory Management:**
- Circular buffer (oldest samples overwritten when full)
- Pre-allocated memory (no heap allocations)
- Memory usage: ~0.1 MB for 30-second buffer (7,500 samples × 16 bytes/sample)

**Dependencies:**
- `WaveformSample` - Value object for waveform data

**See:** [36_DATA_CACHING_STRATEGY.md](./36_DATA_CACHING_STRATEGY.md) and [41_WAVEFORM_DISPLAY_IMPLEMENTATION.md](./41_WAVEFORM_DISPLAY_IMPLEMENTATION.md).

---

## 4. Application Service

### 4.1. MonitoringService

**Responsibility:** Coordinates vitals ingestion, telemetry batching, and alarm detection coordination.

**Thread:** Real-Time Processing Thread

**Key Properties:**
- `currentPatientMrn`: `QString` - Current patient MRN (for telemetry association)
- `telemetryBatchSize`: `int` - Maximum records per batch (default: 100)

**Key Methods:**
- `onVitalSignsReceived(const VitalSigns& vitals)`: Slot for receiving vitals from sensor data source
  1. Appends vitals to `VitalsCache` (in-memory, critical path)
  2. Creates `VitalRecord` value objects
  3. Evaluates alarm rules via `AlarmAggregate` **using cached data**
  4. Builds `TelemetryBatch` with vitals + alarms
  5. Enqueues batch to Database Thread (non-critical, background)
  6. Emits signals to UI Thread (alarm events, vitals updates)
- `onWaveformSamplesReceived(const QList<WaveformSample>& samples)`: Slot for receiving waveform samples
  1. Appends samples to `WaveformCache` (display-only)
  2. Emits signal to `WaveformController` (UI Thread) with window of samples
- `setCurrentPatient(const QString& mrn)`: Sets current patient MRN (for telemetry association)
  - Called by `AdmissionService` when patient admitted
  - All subsequent vitals/alarms include this MRN

**Signals:**
- `vitalsUpdated(const VitalRecord& vital)`: Emitted when new vitals processed (to UI Thread)
- `alarmTriggered(const AlarmEvent& alarm)`: Emitted when alarm detected (to UI Thread)
- `waveformSamplesReady(const QVariantList& samples)`: Emitted when waveform window ready (to UI Thread)

**Dependencies:**
- `ISensorDataSource` - Sensor data input (SharedMemorySensorDataSource)
- `VitalsCache` - In-memory vitals cache
- `WaveformCache` - Waveform buffer
- `AlarmAggregate` - Alarm detection logic
- `TelemetryBatch` - Telemetry data collection
- `PatientAggregate` - Patient context

**Critical Path:**
```
Sensor Data → VitalsCache::append() → AlarmAggregate::evaluate() → UI Signal
< 50ms total latency target
```

---

## 5. Domain Aggregates

### 5.1. PatientAggregate

**Responsibility:** Patient admission state, vitals history, bed assignment.

**Thread:** Real-Time Processing Thread

**Key Properties:**
- `patientMrn`: `QString` - Medical Record Number
- `admissionState`: `AdmissionState` - Current admission state
- `admittedAt`: `QDateTime` - Admission timestamp

**Key Methods:**
- `admit(const PatientIdentity& identity)`: Admits patient to device
- `discharge()`: Discharges patient from device
- `updateVitals(const VitalRecord& vital)`: Updates vitals history

**Domain Events:**
- `PatientAdmitted` - Emitted when patient admitted
- `PatientDischarged` - Emitted when patient discharged

---

### 5.2. TelemetryBatch

**Responsibility:** Telemetry data collection, signing, validation.

**Thread:** Real-Time Processing Thread

**Key Properties:**
- `batchId`: `QString` - UUID for batch correlation
- `deviceId`: `QString` - Device identifier
- `patientMrn`: `QString` - Patient MRN (NULL if no patient admitted)
- `vitals`: `QList<VitalRecord>` - Vitals in batch
- `alarms`: `QList<AlarmSnapshot>` - Alarms in batch
- `createdAt`: `QDateTime` - Batch creation timestamp

**Key Methods:**
- `addVital(const VitalRecord& vital)`: Adds vital to batch
- `addAlarm(const AlarmSnapshot& alarm)`: Adds alarm to batch
- `sign(const QByteArray& privateKey)`: Signs batch with device private key
- `validate()`: Validates batch integrity
- `toJson()`: Serializes batch to JSON for transmission

**Dependencies:**
- `SignatureService` - Data signing (via interface, may be on Network Thread)

---

### 5.3. AlarmAggregate / AlarmManager

**Responsibility:** Alarm lifecycle, state transitions, escalation logic, IEC 60601-1-8 compliance.

**Thread:** Real-Time Processing Thread

**Note:** `AlarmAggregate` is the domain aggregate for alarm logic. `AlarmManager` (if it exists as a separate service) would be an application service that coordinates alarm operations. Based on the thread model, alarm detection happens in the RT Module, so both would run on the RT Thread. For detailed `AlarmManager` class design, see the legacy content in [09_CLASS_DESIGNS.md](./09_CLASS_DESIGNS.md) section 2.2.

**Key Properties:**
- `alarmId`: `QString` - Unique alarm identifier
- `priority`: `AlarmPriority` - HIGH, MEDIUM, LOW
- `status`: `AlarmStatus` - ACTIVE, ACKNOWLEDGED, SILENCED, RESOLVED
- `escalationLevel`: `int` - Current escalation level (0-3)

**Key Methods:**
- `evaluate(const VitalRecord& vital, const AlarmThreshold& threshold)`: Evaluates alarm condition
  - **Latency Target:** < 10ms **CRITICAL**
  - Compares vital value against threshold
  - Implements hysteresis to prevent alarm flutter
  - Returns `AlarmCondition` (NO_ALARM, LOW_PRIORITY, MEDIUM_PRIORITY, HIGH_PRIORITY)
- `raise()`: Raises alarm (creates AlarmSnapshot)
- `acknowledge(const QString& userId)`: Acknowledges alarm
- `silence(int durationSeconds, const QString& userId)`: Silences alarm
- `escalate()`: Escalates alarm (increments escalation level)
- `resolve()`: Resolves alarm (condition cleared)

**Domain Events:**
- `AlarmRaised` - Emitted when alarm triggered
- `AlarmAcknowledged` - Emitted when alarm acknowledged
- `AlarmEscalated` - Emitted when alarm escalates
- `AlarmResolved` - Emitted when alarm resolved

**IEC 60601-1-8 Compliance:**
- Priority-based audio patterns (HIGH: 3 pulses, MEDIUM: 2 pulses, LOW: 1 pulse)
- Maximum silence duration: 600 seconds (10 minutes) for LOW priority
- Escalation rules: HIGH after 60s, MEDIUM after 120s

**Dependencies:**
- `AlarmThreshold` - Threshold configuration
- `VitalRecord` - Vital sign data

---

## 6. Value Objects

### 6.1. VitalRecord

**Responsibility:** Single vital sign measurement (immutable value object).

**Thread:** Thread-agnostic (immutable, can be passed between threads)

**Properties:**
- `timestamp`: `QDateTime` - Measurement timestamp
- `heartRate`: `int` - Heart rate (BPM)
- `spo2`: `int` - Oxygen saturation (%)
- `respirationRate`: `int` - Respiration rate (breaths/min)
- `perfusionIndex`: `double` - Perfusion index
- `patientMrn`: `QString` - Patient MRN (for association)

**Immutability:**
- All properties are `const` (set in constructor)
- No setter methods
- Safe for cross-thread passing (copy semantics)

---

### 6.2. WaveformSample

**Responsibility:** Single waveform sample (ECG, Pleth) (immutable value object).

**Thread:** Thread-agnostic (immutable)

**Properties:**
- `timestamp`: `QDateTime` - Sample timestamp
- `channel`: `QString` - Channel identifier ("ECG_LEAD_II", "PLETH")
- `value`: `double` - Sample value (millivolts for ECG, percentage for Pleth)
- `sampleRate`: `int` - Samples per second (250 Hz for ECG, 125 Hz for Pleth)

---

### 6.3. AlarmSnapshot

**Responsibility:** Alarm state at a point in time (immutable value object).

**Thread:** Thread-agnostic (immutable)

**Properties:**
- `alarmId`: `QString` - Unique alarm identifier
- `timestamp`: `QDateTime` - Alarm trigger time
- `priority`: `AlarmPriority` - HIGH, MEDIUM, LOW
- `alarmType`: `QString` - "HR_HIGH", "SPO2_LOW", etc.
- `value`: `double` - Vital sign value that triggered alarm
- `threshold`: `double` - Threshold value that was exceeded
- `patientMrn`: `QString` - Patient MRN

---

### 6.4. AlarmThreshold

**Responsibility:** Min/max values for alarm triggers (immutable value object).

**Thread:** Thread-agnostic (immutable)

**Properties:**
- `vitalType`: `QString` - "HR", "SPO2", "RR", etc.
- `lowLimit`: `double` - Low threshold (e.g., 60 for HR)
- `highLimit`: `double` - High threshold (e.g., 120 for HR)
- `hysteresis`: `double` - Hysteresis range (±5% typical)
- `priority`: `QString` - "HIGH", "MEDIUM", "LOW"
- `enabled`: `bool` - true if alarm enabled

---

## 7. Module Communication

### 7.1. Inbound (From Other Modules)

**From Sensor Simulator (External):**
- Shared-memory frames (binary) with vitals and waveform data

**From Application Services Module:**
- `Qt::QueuedConnection` signals for patient admission/discharge
- `Qt::QueuedConnection` signals for alarm acknowledgment/silence

### 7.2. Outbound (To Other Modules)

**To Interface Module (UI Thread):**
- `Qt::QueuedConnection` signals for vitals updates
- `Qt::QueuedConnection` signals for alarm events
- `Qt::QueuedConnection` signals for waveform samples

**To Database Module (Database I/O Thread):**
- `MPSC Queue` for telemetry batches (non-critical, background persistence)

**To Network Module (Network I/O Thread):**
- `MPSC Queue` for telemetry batches (for transmission)

---

## 8. Performance Requirements

**Critical Path Latency Targets:**
- Sensor read → sample enqueued: < 1 ms
- Sample → cached (`VitalsCache::append()`): < 5 ms **CRITICAL**
- Alarm detection (using cache) → UI visible: < 50 ms **CRITICAL**
- Cache → database persistence: < 5 seconds **NON-CRITICAL (background)**

**Memory Constraints:**
- Pre-allocated buffers only (no heap allocations in critical path)
- VitalsCache: ~39 MB (3-day capacity)
- WaveformCache: ~0.1 MB (30-second buffer)

**Thread Safety:**
- `VitalsCache` uses `QReadWriteLock` for thread-safe access
- Other components are single-threaded (RT Thread only)

---

## 9. Related Documents

- **[09_CLASS_DESIGNS_OVERVIEW.md](./09_CLASS_DESIGNS_OVERVIEW.md)** - High-level module architecture
- **[12_THREAD_MODEL.md](./12_THREAD_MODEL.md)** - Thread architecture (Section 4.2: Real-Time Processing Thread)
- **[36_DATA_CACHING_STRATEGY.md](./36_DATA_CACHING_STRATEGY.md)** - Caching architecture
- **[37_SENSOR_INTEGRATION.md](./37_SENSOR_INTEGRATION.md)** - Sensor data source interface
- **[41_WAVEFORM_DISPLAY_IMPLEMENTATION.md](./41_WAVEFORM_DISPLAY_IMPLEMENTATION.md)** - Waveform rendering guide

---

*This document provides detailed class designs for the Real-Time Processing Module. For other modules, see the module-specific documents listed in [09_CLASS_DESIGNS_OVERVIEW.md](./09_CLASS_DESIGNS_OVERVIEW.md).*

