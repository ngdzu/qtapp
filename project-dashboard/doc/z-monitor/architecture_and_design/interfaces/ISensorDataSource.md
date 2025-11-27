# ISensorDataSource Interface

**Document ID:** INTERFACE-004  
**Version:** 1.0  
**Status:** Approved  
**Last Updated:** 2025-11-27

---

## 1. Overview

The `ISensorDataSource` interface defines the contract for acquiring vital signs data from various sources (simulator, real hardware sensors, mock data, or replay).

**Purpose:**
- Abstract data source (simulator vs real sensors vs mock)
- Enable dependency inversion (MonitoringService depends on interface, not implementation)
- Support testing with mock/replay data sources
- Allow future hardware sensor integration without changing application code

**Related Documents:**
- **Requirements:** [REQ-FUN-VITAL-001](../../requirements/03_FUNCTIONAL_REQUIREMENTS.md), [REQ-FUN-DEV-010](../../requirements/03_FUNCTIONAL_REQUIREMENTS.md)
- **Architecture:** [02_ARCHITECTURE.md](../02_ARCHITECTURE.md)
- **Data Caching:** [36_DATA_CACHING_STRATEGY.md](../36_DATA_CACHING_STRATEGY.md)
- **Thread Model:** [12_THREAD_MODEL.md](../12_THREAD_MODEL.md)

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

    /**
     * @brief Emitted when data source starts.
     */
    void started();

    /**
     * @brief Emitted when data source stops.
     */
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
 * 
 * Represents a complete set of vital signs at a specific timestamp.
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
    
    /**
     * @brief Validate vital signs are within physiological limits.
     * @return true if all values are valid
     */
    bool isValid() const {
        return heartRate >= 0 && heartRate <= 300 &&
               spo2 >= 0.0 && spo2 <= 100.0 &&
               respirationRate >= 0 && respirationRate <= 60 &&
               signalQuality != "DISCONNECTED";
    }
    
    /**
     * @brief Serialize to JSON.
     */
    QJsonObject toJson() const;
    
    /**
     * @brief Calculate estimated data size in bytes.
     */
    static constexpr int estimatedSize() { return 150; }  // ~150 bytes/record
};
```

### 3.2 WaveformSample

```cpp
/**
 * @struct WaveformSample
 * @brief Single waveform sample (high-frequency data: 125-500 Hz).
 * 
 * Represents a single sample from a physiological waveform.
 */
struct WaveformSample {
    QDateTime timestamp;        ///< Sample time (microsecond precision)
    QString waveformType;       ///< "ECG_LEAD_I", "ECG_LEAD_II", "ECG_LEAD_III", "SPO2_PLETH"
    double value;               ///< Sample value (mV for ECG, arbitrary units for pleth)
    int sampleRate;             ///< Sampling rate (Hz): 125, 250, or 500
    
    /**
     * @brief Calculate estimated data size in bytes.
     */
    static constexpr int estimatedSize() { return 32; }  // ~32 bytes/sample
};
```

### 3.3 DataSourceInfo

```cpp
/**
 * @struct DataSourceInfo
 * @brief Metadata about data source.
 */
struct DataSourceInfo {
    QString name;               ///< Name (e.g., "Device Simulator", "Philips Monitor")
    QString type;               ///< Type: "SIMULATOR", "HARDWARE", "MOCK", "REPLAY"
    QString version;            ///< Version (e.g., "1.0.0")
    QStringList capabilities;   ///< Capabilities (e.g., ["HR", "SPO2", "ECG", "NIBP"])
    bool supportsWaveforms;     ///< true if provides waveform data
};
```

### 3.4 SensorError

```cpp
/**
 * @struct SensorError
 * @brief Sensor error information.
 */
struct SensorError {
    ErrorCode code;             ///< Error code
    QString message;            ///< Human-readable error message
    QString sensorType;         ///< Affected sensor (e.g., "ECG", "SpO2")
    QDateTime timestamp;        ///< When error occurred
    bool recoverable;           ///< true if error is recoverable (retry possible)
};

/**
 * @enum ErrorCode
 * @brief Sensor error codes.
 */
enum class ErrorCode {
    None,                       ///< No error
    SensorDisconnected,         ///< Sensor physically disconnected
    SignalTooNoisy,             ///< Signal quality too poor to measure
    CalibrationFailed,          ///< Sensor calibration failed
    HardwareFailure,            ///< Hardware malfunction
    CommunicationError,         ///< Communication with sensor failed
    UnknownError                ///< Unexpected error
};
```

---

## 4. Implementations

### 4.1 Production Implementation: SimulatorDataSource

**Technology:** Qt Timer + random number generation

```cpp
/**
 * @class SimulatorDataSource
 * @brief Simulator implementation for testing and development.
 * 
 * Generates realistic simulated vital signs and waveforms.
 * Wraps DeviceSimulator for compatibility.
 */
class SimulatorDataSource : public ISensorDataSource {
    Q_OBJECT

public:
    /**
     * @brief Constructor.
     * @param parent Parent QObject
     */
    SimulatorDataSource(QObject* parent = nullptr);

    // ISensorDataSource interface
    bool start() override;
    void stop() override;
    bool isActive() const override;
    DataSourceInfo getInfo() const override;
    double getSamplingRate() const override { return 1.0; }  // 1 Hz for vitals

    /**
     * @brief Configure simulation parameters.
     * @param params Simulation parameters (baseline HR, SpO2, variability)
     */
    void setSimulationParameters(const SimulationParameters& params);

private slots:
    void generateVitals();      // Called every 1 second
    void generateWaveform();    // Called at high frequency (500 Hz)

private:
    QTimer* m_vitalsTimer;      // 1 Hz timer
    QTimer* m_waveformTimer;    // 500 Hz timer
    SimulationParameters m_params;
    bool m_active = false;
};
```

### 4.2 Future Implementation: HardwareSensorAdapter

**Technology:** Serial/USB communication with medical sensors

```cpp
/**
 * @class HardwareSensorAdapter
 * @brief Adapter for real hardware sensors (future implementation).
 * 
 * Communicates with actual medical device sensors via serial/USB.
 * 
 * @note Requires hardware-specific drivers
 * @note Implements same interface as simulator for easy swapping
 */
class HardwareSensorAdapter : public ISensorDataSource {
    Q_OBJECT

public:
    /**
     * @brief Constructor.
     * @param portName Serial port (e.g., "/dev/ttyUSB0", "COM3")
     */
    HardwareSensorAdapter(const QString& portName, QObject* parent = nullptr);

    // ISensorDataSource interface
    bool start() override;
    void stop() override;
    bool isActive() const override;
    DataSourceInfo getInfo() const override;
    double getSamplingRate() const override { return 1.0; }

private slots:
    void onSerialDataReceived();
    void onSerialError();

private:
    QSerialPort* m_serialPort;
    QString m_portName;
    bool m_active = false;
};
```

### 4.3 Testing Implementation: MockSensorDataSource

**Technology:** In-memory (no I/O)

```cpp
/**
 * @class MockSensorDataSource
 * @brief Mock implementation for unit testing.
 * 
 * Returns predefined vital signs data without actual sensor I/O.
 * Useful for deterministic testing.
 */
class MockSensorDataSource : public ISensorDataSource {
    Q_OBJECT

public:
    MockSensorDataSource(QObject* parent = nullptr);

    /**
     * @brief Enqueue vitals to be emitted.
     * @param vitals List of vital records to emit (one per call to emitNext())
     */
    void enqueueVitals(const QList<VitalRecord>& vitals);

    /**
     * @brief Emit next queued vital.
     * @return true if vital emitted, false if queue empty
     */
    bool emitNext();

    /**
     * @brief Simulate sensor error.
     */
    void simulateError(ErrorCode code, const QString& message);

    // ISensorDataSource interface
    bool start() override;
    void stop() override;
    bool isActive() const override;
    DataSourceInfo getInfo() const override;
    double getSamplingRate() const override { return 1.0; }

private:
    QQueue<VitalRecord> m_vitalsQueue;
    bool m_active = false;
};
```

### 4.4 Development Implementation: ReplayDataSource

**Technology:** Read from recorded data file

```cpp
/**
 * @class ReplayDataSource
 * @brief Replays recorded vital signs data from file.
 * 
 * Useful for debugging specific scenarios or edge cases.
 */
class ReplayDataSource : public ISensorDataSource {
    Q_OBJECT

public:
    /**
     * @brief Constructor.
     * @param filename Path to recorded data file (CSV or JSON)
     */
    ReplayDataSource(const QString& filename, QObject* parent = nullptr);

    /**
     * @brief Load recorded data from file.
     * @return true if loaded successfully
     */
    bool loadFromFile(const QString& filename);

    /**
     * @brief Set replay speed multiplier.
     * @param speed Speed multiplier (1.0 = real-time, 2.0 = 2x speed)
     */
    void setReplaySpeed(double speed) { m_replaySpeed = speed; }

    // ISensorDataSource interface
    bool start() override;
    void stop() override;
    bool isActive() const override;
    DataSourceInfo getInfo() const override;
    double getSamplingRate() const override { return 1.0; }

private slots:
    void replayNext();

private:
    QList<VitalRecord> m_recordedVitals;
    int m_currentIndex = 0;
    QTimer* m_replayTimer;
    double m_replaySpeed = 1.0;
    bool m_active = false;
};
```

---

## 5. Usage Examples

### 5.1 Basic Usage (Simulator)

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
    qInfo() << "Data acquisition started";
} else {
    qCritical() << "Failed to start data acquisition";
}
```

### 5.2 MonitoringService Integration

```cpp
/**
 * @class MonitoringService
 * @brief Coordinates vital signs monitoring (critical path).
 */
class MonitoringService : public QObject {
    Q_OBJECT

public:
    /**
     * @brief Constructor with dependency injection.
     * @param dataSource Sensor data source (injected)
     * @param vitalsCache In-memory cache
     * @param alarmManager Alarm evaluation
     */
    MonitoringService(
        ISensorDataSource* dataSource,    // ✅ Depends on interface, not implementation
        VitalsCache* vitalsCache,
        AlarmManager* alarmManager,
        QObject* parent = nullptr
    ) : QObject(parent),
        m_dataSource(dataSource),
        m_vitalsCache(vitalsCache),
        m_alarmManager(alarmManager)
    {
        // Connect signals (critical path)
        connect(m_dataSource, &ISensorDataSource::vitalSignsReceived,
                this, &MonitoringService::onVitalsReceived);
    }

    void start() {
        if (!m_dataSource->start()) {
            emit startFailed("Data source failed to start");
            return;
        }
        qInfo() << "Monitoring started with data source:" << m_dataSource->getInfo().name;
    }

private slots:
    void onVitalsReceived(const VitalRecord& vital) {
        // CRITICAL PATH (< 50ms total)
        
        // 1. Validate vitals (< 1ms)
        if (!vital.isValid()) {
            qWarning() << "Invalid vital signs received";
            return;
        }
        
        // 2. Add to in-memory cache (< 5ms)
        m_vitalsCache->append(vital);
        
        // 3. Evaluate alarms (< 20ms)
        m_alarmManager->processVitalSigns(vital);
        
        // 4. Update UI (< 10ms)
        emit vitalsUpdated(vital);
        
        // 5. Queue for telemetry (< 5ms)
        m_telemetryBatch->addVital(vital);
    }

private:
    ISensorDataSource* m_dataSource;  // ✅ Interface (dependency inversion)
    VitalsCache* m_vitalsCache;
    AlarmManager* m_alarmManager;
    TelemetryBatch* m_telemetryBatch;
};
```

### 5.3 Dependency Injection Configuration

```cpp
/**
 * @brief Configure dependency injection container.
 */
void ServiceContainer::configure() {
    // Production mode: use simulator
    if (Settings::instance()->useSimulator()) {
        registerSingleton<ISensorDataSource>([]() {
            return new SimulatorDataSource();
        });
    }
    // Future: use real hardware
    else if (Settings::instance()->useHardwareSensors()) {
        registerSingleton<ISensorDataSource>([]() {
            return new HardwareSensorAdapter("/dev/ttyUSB0");
        });
    }
    // Testing: use mock
    else {
        registerSingleton<ISensorDataSource>([]() {
            return new MockSensorDataSource();
        });
    }
    
    // MonitoringService gets ISensorDataSource injected
    registerSingleton<MonitoringService>([](ServiceContainer* container) {
        return new MonitoringService(
            container->resolve<ISensorDataSource>(),  // ✅ Injected
            container->resolve<VitalsCache>(),
            container->resolve<AlarmManager>()
        );
    });
}
```

---

## 6. Testing

### 6.1 Unit Test Example

```cpp
TEST(ISensorDataSource, MockDataSource) {
    // Arrange
    MockSensorDataSource dataSource;
    
    VitalRecord vital;
    vital.timestamp = QDateTime::currentDateTime();
    vital.heartRate = 75;
    vital.spo2 = 98.0;
    vital.respirationRate = 16;
    vital.signalQuality = "GOOD";
    
    dataSource.enqueueVitals({vital});
    
    // Spy on signal
    QSignalSpy spy(&dataSource, &ISensorDataSource::vitalSignsReceived);
    
    // Act
    dataSource.start();
    dataSource.emitNext();
    
    // Assert
    ASSERT_EQ(spy.count(), 1);
    VitalRecord emitted = spy.at(0).at(0).value<VitalRecord>();
    EXPECT_EQ(emitted.heartRate, 75);
    EXPECT_EQ(emitted.spo2, 98.0);
}

TEST(ISensorDataSource, SimulatorDataSource) {
    // Arrange
    SimulatorDataSource dataSource;
    QSignalSpy spy(&dataSource, &ISensorDataSource::vitalSignsReceived);
    
    // Act
    dataSource.start();
    QTest::qWait(2000);  // Wait 2 seconds (expect 2 vitals at 1 Hz)
    dataSource.stop();
    
    // Assert
    EXPECT_GE(spy.count(), 2);  // At least 2 vitals emitted
    
    for (int i = 0; i < spy.count(); ++i) {
        VitalRecord vital = spy.at(i).at(0).value<VitalRecord>();
        EXPECT_TRUE(vital.isValid());
    }
}
```

---

## 7. Performance Considerations

### 7.1 Critical Path Timing

```
ISensorDataSource::vitalSignsReceived signal
  ↓ < 50ms TOTAL (real-time requirement)
  ├─ VitalsCache::append()         < 5ms
  ├─ AlarmManager::processVitals() < 20ms
  ├─ UI update                     < 10ms
  └─ Telemetry batch queue         < 5ms
```

### 7.2 Data Rate

**Vitals (Low-Frequency):**
- 1 Hz (once per second)
- 150 bytes/record
- 150 bytes/second = 0.15 KB/s = 13 MB/day

**Waveforms (High-Frequency):**
- ECG: 500 Hz × 3 leads × 32 bytes = 48 KB/s
- SpO2: 125 Hz × 32 bytes = 4 KB/s
- Total: ~52 KB/s = 4.5 GB/day (display-only, not persisted)

---

## 8. Security Considerations

- **Data Validation:** Always validate vitals before processing (prevent malformed data)
- **Signal Quality:** Don't trust vitals with "DISCONNECTED" signal quality
- **Error Handling:** Sensor errors should not crash the application
- **Audit Logging:** Log sensor connection/disconnection events

---

## 9. Related Documents

- **Requirements:** [03_FUNCTIONAL_REQUIREMENTS.md](../../requirements/03_FUNCTIONAL_REQUIREMENTS.md) (REQ-FUN-VITAL-001)
- **Architecture:** [02_ARCHITECTURE.md](../02_ARCHITECTURE.md)
- **Class Designs:** [09_CLASS_DESIGNS.md](../09_CLASS_DESIGNS.md) (MonitoringService)
- **Thread Model:** [12_THREAD_MODEL.md](../12_THREAD_MODEL.md)
- **Data Caching Review:** [36_DATA_CACHING_STRATEGY.md](../36_DATA_CACHING_STRATEGY.md)

---

*This interface decouples the MonitoringService from specific sensor implementations, enabling testability and future hardware integration.*

