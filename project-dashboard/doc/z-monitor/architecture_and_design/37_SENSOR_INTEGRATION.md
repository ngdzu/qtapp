# Sensor Integration Summary

**Date:** 2025-11-27  
**Status:** Interface Documented, Implementation Task Added to ZTODO

---

## Overview

Z-Monitor will receive vital signs data from an external **Sensor Simulator** via WebSocket connection. The integration follows the Dependency Inversion Principle using the `ISensorDataSource` interface.

---

## Architecture

### Component Diagram

```
┌───────────────────────────────────────────────────────────────┐
│  Sensor Simulator (External Process)                         │
│  Location: project-dashboard/sensor-simulator/               │
│                                                               │
│  - WebSocket Server: ws://localhost:9002                     │
│  - Sends JSON vitals at 5 Hz (200ms intervals)               │
│  - Sends ECG waveforms at 250 Hz                             │
│  - Built with Qt (QWebSocketServer)                          │
└────────────────────────┬──────────────────────────────────────┘
                         │ WebSocket (JSON)
                         ↓
┌───────────────────────────────────────────────────────────────┐
│  Z-Monitor Application                                        │
│                                                               │
│  ┌──────────────────────────────────────────────────────┐   │
│  │  ISensorDataSource Interface                         │   │
│  │  (Abstraction for sensor data acquisition)           │   │
│  └────────────┬─────────────────────────────────────────┘   │
│               │ implements                                    │
│  ┌────────────▼──────────────────────────────────────────┐  │
│  │  WebSocketSensorDataSource                            │  │
│  │  - Connects to ws://localhost:9002                    │  │
│  │  - Parses JSON vitals messages                        │  │
│  │  - Emits vitalSignsReceived() signal                  │  │
│  │  - Handles reconnection with backoff                  │  │
│  └────────────┬──────────────────────────────────────────┘  │
│               │ signals                                       │
│  ┌────────────▼──────────────────────────────────────────┐  │
│  │  MonitoringService                                     │  │
│  │  - Receives vitals via signals                         │  │
│  │  - Caches to in-memory (VitalsCache)                   │  │
│  │  - Evaluates alarms (AlarmManager)                     │  │
│  │  - Batches for telemetry (TelemetryBatch)             │  │
│  └────────────────────────────────────────────────────────┘  │
└───────────────────────────────────────────────────────────────┘
```

---

## Sensor Simulator Details

### Location
```
project-dashboard/sensor-simulator/
```

### WebSocket Endpoint
```
ws://localhost:9002
```

### JSON Message Format

**Vitals Message (5 Hz):**
```json
{
  "type": "vitals",
  "timestamp_ms": 1234567890,
  "hr": 75,
  "spo2": 98,
  "rr": 16,
  "waveform": {
    "channel": "ecg",
    "sample_rate": 250,
    "start_timestamp_ms": 1234567890,
    "values": [100, 102, 98, ...]
  }
}
```

### Features
- **Real-time Vitals:** Heart Rate, SpO2, Respiration Rate
- **ECG Waveform:** Realistic PQRST complex at 250 Hz
- **Interactive Controls:** Manual trigger for alarms
- **Demo Sequence:** Automated alarm scenarios
- **QML UI:** Modern UI with live updates

### Running the Simulator

**Local:**
```bash
cd project-dashboard/sensor-simulator
mkdir -p build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Release
cmake --build . --config Release
./sensor_simulator
```

**Docker:**
```bash
cd project-dashboard
docker compose -f docker-compose.simulator.yml up --build
```

---

## Z-Monitor Integration

### Interface: ISensorDataSource

**Location:** `doc/z-monitor/architecture_and_design/interfaces/ISensorDataSource.md`

**Purpose:**
- Abstract sensor data source (simulator vs hardware vs mock)
- Enable dependency inversion (MonitoringService depends on interface, not implementation)
- Support testing with mock data sources
- Allow future hardware sensor integration

**Key Methods:**
```cpp
class ISensorDataSource : public QObject {
    Q_OBJECT
public:
    virtual bool start() = 0;
    virtual void stop() = 0;
    virtual bool isActive() const = 0;
    virtual DataSourceInfo getInfo() const = 0;

signals:
    void vitalSignsReceived(const VitalRecord& vital);
    void waveformSampleReceived(const WaveformSample& waveform);
    void connectionStatusChanged(bool connected, const QString& sensorType);
    void sensorError(const SensorError& error);
};
```

### Implementation: WebSocketSensorDataSource

**Location:** `z-monitor/src/infrastructure/sensors/WebSocketSensorDataSource.cpp/h`

**Responsibilities:**
1. Connect to simulator's WebSocket server (ws://localhost:9002)
2. Parse JSON vitals messages
3. Convert JSON to `VitalRecord` and `WaveformSample` structs
4. Emit Qt signals for MonitoringService
5. Handle connection errors and reconnection with exponential backoff

**Key Features:**
- **Auto-reconnect:** Reconnects with exponential backoff (1s, 2s, 4s, 8s, max 30s)
- **Error Handling:** Graceful handling of network errors, JSON parse errors
- **Connection Status:** Reports connection status changes
- **Thread-Safe:** Runs on Real-Time Processing thread

### Data Flow

```
Sensor Simulator (External)
  ↓ (WebSocket JSON @ 5 Hz)
WebSocketSensorDataSource::onTextMessageReceived()
  ↓ (Parse JSON)
WebSocketSensorDataSource::parseVitalsMessage()
  ↓ (Create VitalRecord)
emit ISensorDataSource::vitalSignsReceived(VitalRecord)
  ↓ (Qt Signal)
MonitoringService::onVitalsReceived(VitalRecord)
  ↓ (< 50ms critical path)
  ├─ VitalsCache::append() (in-memory cache)
  ├─ AlarmManager::processVitalSigns() (alarm evaluation)
  ├─ UI update (DashboardController)
  └─ TelemetryBatch::addVital() (batch for server)
```

---

## Alternative Implementations

### 1. SimulatorDataSource (Legacy/Fallback)
- **Purpose:** Internal Qt Timer-based simulator (no external process)
- **Use Case:** When sensor-simulator is not available
- **Location:** `z-monitor/src/infrastructure/sensors/SimulatorDataSource.cpp/h`

### 2. MockSensorDataSource (Testing)
- **Purpose:** Deterministic testing with predefined data
- **Use Case:** Unit tests, integration tests
- **Location:** `z-monitor/src/infrastructure/sensors/MockSensorDataSource.cpp/h`

### 3. HardwareSensorAdapter (Future)
- **Purpose:** Real hardware sensors (serial/USB)
- **Use Case:** Production deployment with actual medical sensors
- **Location:** `z-monitor/src/infrastructure/sensors/HardwareSensorAdapter.cpp/h`

### 4. ReplayDataSource (Development)
- **Purpose:** Replay recorded data from file
- **Use Case:** Debugging specific scenarios
- **Location:** `z-monitor/src/infrastructure/sensors/ReplayDataSource.cpp/h`

---

## Dependency Injection

```cpp
// Configuration (based on settings)
void ServiceContainer::configure() {
    // Use WebSocket simulator (production/development default)
    if (Settings::instance()->useWebSocketSimulator()) {
        registerSingleton<ISensorDataSource>([]() {
            return new WebSocketSensorDataSource("ws://localhost:9002");
        });
    }
    // Use internal simulator (fallback)
    else if (Settings::instance()->useInternalSimulator()) {
        registerSingleton<ISensorDataSource>([]() {
            return new SimulatorDataSource();
        });
    }
    // Use mock (testing)
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

## ZTODO Task

**Task Added:** `Implement WebSocketSensorDataSource for Sensor Simulator Integration`

**Location in ZTODO:** After "Define public C++ service interfaces" task

**Key Points:**
- Implements `ISensorDataSource` interface
- Connects to ws://localhost:9002
- Parses JSON vitals messages
- Handles reconnection with exponential backoff
- Includes verification steps (functional, code quality, documentation, integration, tests)

**Acceptance Criteria:**
1. WebSocket connects to simulator
2. Parses JSON and emits signals
3. Connection errors handled gracefully
4. MonitoringService uses interface (not concrete class)

**Dependencies:**
- Qt WebSockets module
- Sensor simulator running externally
- ISensorDataSource interface defined

---

## Testing Strategy

### Unit Tests
```cpp
TEST(WebSocketSensorDataSource, ConnectToSimulator) {
    WebSocketSensorDataSource dataSource("ws://localhost:9002");
    QSignalSpy spy(&dataSource, &ISensorDataSource::vitalSignsReceived);
    
    ASSERT_TRUE(dataSource.start());
    QTest::qWait(1000);  // Wait for vitals
    
    EXPECT_GE(spy.count(), 1);  // At least 1 vital received
}

TEST(WebSocketSensorDataSource, ParseVitalsJSON) {
    QString json = R"({
        "type": "vitals",
        "timestamp_ms": 1234567890,
        "hr": 75,
        "spo2": 98.5,
        "rr": 16
    })";
    
    VitalRecord vital = parseVitalsMessage(json);
    
    EXPECT_EQ(vital.heartRate, 75);
    EXPECT_DOUBLE_EQ(vital.spo2, 98.5);
    EXPECT_EQ(vital.respirationRate, 16);
}
```

### Integration Tests
```cpp
TEST(MonitoringService, ReceivesVitalsFromWebSocket) {
    // Arrange
    WebSocketSensorDataSource* dataSource = new WebSocketSensorDataSource("ws://localhost:9002");
    VitalsCache cache;
    AlarmManager alarmMgr;
    MonitoringService service(dataSource, &cache, &alarmMgr);
    
    // Act
    service.start();
    QTest::qWait(2000);  // Wait 2 seconds
    
    // Assert
    EXPECT_GT(cache.size(), 0);  // Cache has vitals
}
```

---

## Performance Considerations

### Data Rate
- **Vitals:** 5 Hz (200ms intervals) = 5 messages/second
- **Waveforms:** 250 Hz = 250 samples/second (50 samples per packet)
- **Total Bandwidth:** ~52 KB/second

### Critical Path Timing
```
WebSocket message received (0ms)
  ↓
Parse JSON (< 5ms)
  ↓
Emit signal (< 1ms)
  ↓
MonitoringService processes (< 50ms total)
  ├─ VitalsCache::append() (< 5ms)
  ├─ AlarmManager::processVitals() (< 20ms)
  ├─ UI update (< 10ms)
  └─ Telemetry batch (< 5ms)
```

**Total: < 60ms from WebSocket to UI update** ✅

---

## Security Considerations

### WebSocket Security
- **Development:** ws:// (unencrypted) on localhost:9002 ✅
- **Production:** Should use wss:// (WebSocket Secure) with TLS
- **Future:** Add authentication token for WebSocket connection

### Data Validation
- **JSON Parsing:** Validate all fields before creating VitalRecord
- **Range Checking:** Ensure vitals within physiological limits
- **Error Handling:** Malformed JSON should not crash application

---

## Documentation Updates

### Created
1. ✅ `interfaces/ISensorDataSource.md` (694 lines) - Complete interface specification
2. ✅ [36_DATA_CACHING_STRATEGY.md](./36_DATA_CACHING_STRATEGY.md) (611 lines) - Caching strategy and architecture
3. ✅ [37_SENSOR_INTEGRATION.md](./37_SENSOR_INTEGRATION.md) (This document) - Integration overview

### Updated
1. ✅ `ZTODO.md` - Added WebSocketSensorDataSource implementation task
2. ✅ [35_REQUIREMENTS_ARCHITECTURE_ANALYSIS.md](./35_REQUIREMENTS_ARCHITECTURE_ANALYSIS.md) - Updated interface count (4/5, 80%)

### To Update (During Implementation)
1. `09_CLASS_DESIGNS.md` - Add WebSocketSensorDataSource class design
2. `02_ARCHITECTURE.md` - Update data flow diagram
3. `12_THREAD_MODEL.md` - Document sensor data source thread assignment

---

## Summary

### ✅ Completed
- ISensorDataSource interface documented (694 lines)
- Data caching architecture reviewed
- Integration strategy defined
- ZTODO task added

### ⏳ Next Steps (Implementation)
1. Implement `WebSocketSensorDataSource` class
2. Implement `MockSensorDataSource` for testing
3. Update `MonitoringService` to use `ISensorDataSource`
4. Write unit and integration tests
5. Update documentation

### 🎯 Benefits
- ✅ Decouples z-monitor from sensor implementation
- ✅ Enables testing without external simulator
- ✅ Supports future hardware sensor integration
- ✅ Follows Dependency Inversion Principle
- ✅ Interface-based design for flexibility

---

**Status:** Ready for Implementation  
**Interface Documentation:** Complete (4/5 interfaces documented)  
**Implementation Task:** Added to ZTODO.md

