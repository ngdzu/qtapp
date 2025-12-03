# DIContainer (Application Wiring)

## Overview

- Purpose: Centralizes construction and wiring of core services and repositories following DDD boundaries. Provides a single initialization point for database, caches, sensor source, repositories, and application services.
- Location: `src/application/di/DIContainer.h/cpp`
- Entry: Created in `src/main.cpp`, initialized via `initialize()`.

## Responsibilities

- Configure sensor source: `SharedMemorySensorDataSource` or `InMemorySensorDataSource` based on `AppConfig.sensorSource`.
- Initialize caches: `VitalsCache`, `WaveformCache` using values from `AppConfig`.
- Initialize database: `DatabaseManager::open(AppConfig.databasePath)`; run migrations; register prepared queries via `persistence::QueryCatalog::initializeQueries()`.
- Construct repositories: `SQLitePatientRepository`, `SQLiteVitalsRepository`, `SQLiteTelemetryRepository`, `SQLiteAlarmRepository`.
- Construct services: `MonitoringService` (with repositories, caches, sensor), `TelemetryService` (with `HttpTelemetryServerAdapter`).
- Provide accessors for all constructed components.

## Lifecycle

1. Construct: `DIContainer(AppConfig cfg, QObject* app)` sets up sensor/caches/DB manager.
2. Initialize: `initialize()` opens DB, runs migrations, registers queries, creates repositories and services, starts `TelemetryService`.
3. Access: Call getters (e.g., `monitoringService()`, `vitalsRepository()`) to use components.

## Configuration Inputs (AppConfig)

- `databasePath`: SQLite path (e.g., `~/Library/Application Support/z-monitor/zmonitor.db`).
- `sensorSource`: `SharedMemory` or `InMemory`.
- `sharedMemorySocket`: Path for shared-memory control socket (if `SharedMemory`).
- `vitalsCacheSeconds`: In-memory vitals cache retention.
- `waveformCacheSamples`: Waveform cache sample size.

## Data Flow (Mermaid)

```mermaid
flowchart LR
    subgraph Infra
    DB[(DatabaseManager)] --> QC[QueryCatalog]
    SM[SharedMemorySensorDataSource]
    IM[InMemorySensorDataSource]
    VC[VitalsCache]
    WC[WaveformCache]
    PR[SQLitePatientRepository]
    VR[SQLiteVitalsRepository]
    TR[SQLiteTelemetryRepository]
    AR[SQLiteAlarmRepository]
    end

    subgraph App
    MS[MonitoringService]
    TS[TelemetryService]
    end

    CFG[[AppConfig]] --> DI[DIContainer]
    DI --> DB
    DI --> VC
    DI --> WC
    DI --> PR
    DI --> VR
    DI --> TR
    DI --> AR
    DI --> MS
    DI --> TS
    CFG --> SM
    CFG --> IM
    SM -.or-. IM --> MS
    VR --> MS
    AR --> MS
    TR --> MS
    PR --> MS
    TS -->|HttpTelemetryServerAdapter| Net[(Network)]
```

## Usage Snippet

```cpp
#include "application/di/DIContainer.h"
#include "application/config/AppConfig.h"

int main(int argc, char** argv) {
    QGuiApplication app(argc, argv);

    AppConfig cfg;
    cfg.databasePath = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation) + "/zmonitor.db";
    cfg.sensorSource = SensorSourceMode::InMemory;
    cfg.vitalsCacheSeconds = 600;
    cfg.waveformCacheSamples = 2500;

    zmon::DIContainer container(cfg, &app);
    if (!container.initialize()) {
        return 1;
    }

    auto* monitoring = container.monitoringService();
    monitoring->start();
    return app.exec();
}
```

## Notes

- DIContainer is intentionally thin; business logic lives in services and repositories.
- TelemetryService is started automatically; MonitoringService start is controlled by the app.
- Keep migrations and query registration centralized here to ensure consistent startup.
