---
doc_id: DOC-API-003
title: ISensorDataSource Interface
version: 1.0
category: API
subcategory: Data Acquisition
status: Approved
created: 2025-11-27
updated: 2025-11-27
tags: [api, interface, sensor, data-source, vitals, waveform, simulator]
related_docs:
  - DOC-COMP-010 # MonitoringService
  - DOC-COMP-023 # VitalsCache
  - DOC-COMP-024 # WaveformCache
  - DOC-ARCH-004 # Thread Model
  - DOC-ARCH-005 # Data Flow and Caching
authors:
  - Z Monitor Team
reviewers:
  - Architecture Team
---

# ISensorDataSource Interface

## 1. Overview

The `ISensorDataSource` interface defines the contract for acquiring vital signs data from various sources (simulator, real hardware sensors, mock data, or replay).

**Purpose:**
- Abstract data source (simulator vs real sensors vs mock)
- Enable dependency inversion (MonitoringService depends on interface, not implementation)
- Support testing with mock/replay data sources
- Allow future hardware sensor integration without changing application code

**Key Characteristics:**
- **Event-Driven:** All data emitted via Qt signals (no polling)
- **High-Frequency:** Supports both 1 Hz vitals and 125-500 Hz waveforms
- **Real-Time Thread:** Runs on Real-Time Processing Thread (high priority)
- **Start/Stop Lifecycle:** Explicit start() and stop() for acquisition control

---

## 2. Interface Definition

### 2.1 C++ Header

```cpp
/**
 * @interface ISensorDataSource
 * @brief Interface for vital signs data acquisition.
 * 
 * This interface abstracts the source of vital signs data, enabling
 * multiple implementations (simulator, real hardware, mock, replay).
 * 
 * @note All data is emitted via signals (Qt event-driven model)
 * @note Runs on Real-Time Processing Thread (high priority)
 * @see VitalRecord, WaveformSample, MonitoringService
 * @ingroup Infrastructure
 */
class ISensorDataSource : public QObject {
    Q_OBJECT

public:
    virtual ~ISensorDataSource() = default;

    /**
     * @brief Start data acquisition.
     * 
     * Begins acquiring vital signs data and emitting signals.
     * 
     * @return true if started successfully, false otherwise
     * 
     * @note Non-blocking (data emitted via signals)
     * @note Call stop() before destroying object
     */
    virtual bool start() = 0;

    /**
     * @brief Stop data acquisition.
     * 
     * Stops acquiring data and stops emitting signals.
     * 
     * @note Graceful shutdown (flushes pending data)
     */
    virtual void stop() = 0;

    /**
     * @brief Check if data source is currently active.
     * 
     * @return true if actively acquiring data
     */
    virtual bool isActive() const = 0;

    /**
     * @brief Get data source metadata (name, type, version).
     * 
     * @return DataSourceInfo Metadata about this data source
     */
    virtual DataSourceInfo getInfo() const = 0;

    /**
     * @brief Get current sampling rate (Hz).
     * 
     * @return Sampling rate in Hz (e.g., 1.0 for 1 Hz vitals, 500.0 for ECG)
     */
    virtual double getSamplingRate() const = 0;

signals:
    /**
     * @brief Emitted when new vital signs data available.
     * 
     * @param vital Vital signs record (HR, SpO2, RR, etc.)
     * 
     * @note Emitted at 1 Hz (once per second) for vitals
     * @note CRITICAL PATH: < 50ms from sensor reading to this signal
     */
    void vitalSignsReceived(const VitalRecord& vital);

    /**
     * @brief Emitted when waveform sample available.
     * 
     * @param waveform Waveform sample (ECG, SpO2 pleth, etc.)
     * 
     * @note Emitted at high frequency (125-500 Hz depending on waveform type)
     * @note Only for display (not persisted to database)
     */
    void waveformSampleReceived(const WaveformSample& waveform);

    /**
     * @brief Emitted when sensor connection status changes.
     * 
     * @param connected true if sensors connected, false if disconnected
     * @param sensorType Type of sensor (e.g., "ECG", "SpO2", "NIBP")
     */
    void connectionStatusChanged(bool connected, const QString& sensorType);

    /**
     * @brief Emitted when sensor error occurs.
     * 
     * @param error Error details
     */
    void sensorError(const SensorError& error);

    void started();
    void stopped();
};
```

---

## 3. Data Structures

### 3.1 VitalRecord

```cpp
/**
 * @struct VitalRecord
 * @brief Single vital signs measurement (1 Hz sampling).
 */
struct VitalRecord {
    QDateTime timestamp;        ///< Measurement time (Unix milliseconds)
    
    // Core vitals (always present)
    int heartRate;              ///< Heart rate (bpm), range: 0-300
    double spo2;                ///< SpO2 (%), range: 0-100
    int respirationRate;        ///< Respiration rate (rpm), range: 0-60
    
    // NIBP (non-invasive blood pressure, periodic)
    int systolic;               ///< Systolic BP (mmHg), -1 if not measured
    int diastolic;              ///< Diastolic BP (mmHg), -1 if not measured
    int meanArterialPressure;   ///< MAP (mmHg), -1 if not measured
    
    // Temperature (periodic)
    double temperature;         ///< Temperature (°C or °F), NaN if not measured
    
    // Signal quality
    QString signalQuality;      ///< GOOD, FAIR, POOR, DISCONNECTED
    
    bool isValid() const {
        return heartRate >= 0 && heartRate <= 300 &&
               spo2 >= 0.0 && spo2 <= 100.0 &&
               respirationRate >= 0 && respirationRate <= 60 &&
               signalQuality != "DISCONNECTED";
    }
    
    static constexpr int estimatedSize() { return 150; }  // ~150 bytes/record
};
```

### 3.2 WaveformSample

```cpp
/**
 * @struct WaveformSample
 * @brief Single waveform sample (high-frequency data: 125-500 Hz).
 */
struct WaveformSample {
    QDateTime timestamp;        ///< Sample time (microsecond precision)
    QString waveformType;       ///< "ECG_LEAD_I", "ECG_LEAD_II", "SPO2_PLETH"
    double value;               ///< Sample value (mV for ECG, arbitrary units for pleth)
    int sampleRate;             ///< Sampling rate (Hz): 125, 250, or 500
    
    static constexpr int estimatedSize() { return 32; }  // ~32 bytes/sample
};
```

---

## 4. Implementations

### 4.1 SimulatorDataSource (Production)

**Technology:** Qt Timer + random number generation

**Features:**
- Generates realistic simulated vital signs and waveforms
- Configurable simulation parameters (baseline HR, SpO2, variability)
- Wraps DeviceSimulator for compatibility

### 4.2 HardwareSensorAdapter (Future)

**Technology:** Serial/USB communication with medical sensors

**Features:**
- Communicates with actual medical device sensors via serial/USB
- Requires hardware-specific drivers
- Same interface as simulator for easy swapping

### 4.3 MockSensorDataSource (Testing)

**Technology:** In-memory (no I/O)

**Features:**
- Returns predefined vital signs data
- Deterministic testing (no randomness)
- No external dependencies

### 4.4 ReplayDataSource (Development)

**Technology:** Read from recorded data file

**Features:**
- Replays recorded vital signs data from CSV/JSON file
- Configurable replay speed (1x, 2x, etc.)
- Useful for debugging specific scenarios

---

## 5. Usage Example

```cpp
// Create simulator data source
auto* dataSource = new SimulatorDataSource(this);

// Connect signals
connect(dataSource, &ISensorDataSource::vitalSignsReceived,
        this, &MonitoringService::onVitalsReceived);

connect(dataSource, &ISensorDataSource::sensorError,
        this, &MonitoringService::onSensorError);

// Start acquisition
if (dataSource->start()) {
    m_logService->info("Data acquisition started");
} else {
    m_logService->critical("Failed to start data acquisition");
}
```

---

## 6. Performance Characteristics

**Critical Path Timing:**
```
ISensorDataSource::vitalSignsReceived signal
  ↓ < 50ms TOTAL (real-time requirement)
  ├─ VitalsCache::append()         < 5ms
  ├─ AlarmManager::processVitals() < 20ms
  ├─ UI update                     < 10ms
  └─ Telemetry batch queue         < 5ms
```

**Data Rates:**
- **Vitals:** 1 Hz × 150 bytes = 0.15 KB/s = 13 MB/day
- **Waveforms:** ECG 500 Hz × 3 leads × 32 bytes = 48 KB/s = 4.5 GB/day (display-only)

---

## 7. Related Documents

- **DOC-COMP-010:** MonitoringService - Consumes vital signs data
- **DOC-COMP-023:** VitalsCache - Caches vitals in memory
- **DOC-COMP-024:** WaveformCache - Caches waveforms for display
- **DOC-ARCH-004:** Thread Model - Real-Time Processing Thread
- **DOC-ARCH-005:** Data Flow and Caching - Data flow architecture

---

## 8. Changelog

| Version | Date       | Author         | Changes                                        |
| ------- | ---------- | -------------- | ---------------------------------------------- |
| 1.0     | 2025-11-27 | Z Monitor Team | Migrated from INTERFACE-004, added frontmatter |

---

*This interface decouples the MonitoringService from specific sensor implementations, enabling testability, future hardware integration, and flexible data sources.*
