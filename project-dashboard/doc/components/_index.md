# Component Documentation

**Category:** COMP
**Total Documents:** 30
**Generated:** 2025-12-01 16:39:41

## Documents

### Application Layer / Repository Interface

#### [DOC-COMP-014: IPatientRepository](DOC-COMP-014_ipatientrepository.md)

- **Version:** v1.0
- **Status:** Draft
- **Owner:** Application Layer Team
- **Tags:** repository, persistence, patient, ddd
- **Related:** DOC-ARCH-002, DOC-COMP-001

#### [DOC-COMP-015: ITelemetryRepository](DOC-COMP-015_itelemetryrepository.md)

- **Version:** v1.0
- **Status:** Draft
- **Owner:** Application Layer Team
- **Tags:** repository, persistence, telemetry, vitals
- **Related:** DOC-ARCH-002, DOC-COMP-004, DOC-COMP-010

#### [DOC-COMP-016: IAlarmRepository](DOC-COMP-016_ialarmrepository.md)

- **Version:** v1.0
- **Status:** Draft
- **Owner:** Application Layer Team
- **Tags:** repository, persistence, alarm, safety
- **Related:** DOC-ARCH-002, DOC-COMP-005, DOC-COMP-010

#### [DOC-COMP-017: IProvisioningRepository](DOC-COMP-017_iprovisioningrepository.md)

- **Version:** v1.0
- **Status:** Draft
- **Owner:** Application Layer Team
- **Tags:** repository, persistence, provisioning, device
- **Related:** DOC-ARCH-002, DOC-COMP-003, DOC-COMP-012

### Application Layer / Use Case Orchestration

#### [DOC-COMP-010: MonitoringService](DOC-COMP-010_monitoringservice.md)

- **Version:** v1.0
- **Status:** Draft
- **Owner:** Application Layer Team
- **Tags:** application-service, use-case-orchestration, monitoring, vitals, telemetry, alarms
- **Related:** DOC-ARCH-002, DOC-COMP-002, DOC-COMP-004, DOC-COMP-014, DOC-COMP-015, DOC-COMP-016

#### [DOC-COMP-011: AdmissionService](DOC-COMP-011_admissionservice.md)

- **Version:** v1.0
- **Status:** Draft
- **Owner:** Application Layer Team
- **Tags:** application-service, use-case-orchestration, admission, adt-workflow, patient-management
- **Related:** DOC-ARCH-002, DOC-COMP-002, DOC-COMP-005, DOC-COMP-009, DOC-COMP-014

#### [DOC-COMP-012: ProvisioningService](DOC-COMP-012_provisioningservice.md)

- **Version:** v1.0
- **Status:** Draft
- **Owner:** Application Layer Team
- **Tags:** application-service, provisioning, qr-code, device-configuration, certificate-management
- **Related:** DOC-ARCH-002, DOC-COMP-003, DOC-COMP-017

#### [DOC-COMP-013: SecurityService](DOC-COMP-013_securityservice.md)

- **Version:** v1.0
- **Status:** Draft
- **Owner:** Application Layer Team
- **Tags:** application-service, authentication, authorization, session-management, rbac, security
- **Related:** DOC-ARCH-002

### Component

#### [DOC-COMP-001: Test Component Component Specification](DOC-COMP-001_test_component.md)

- **Version:** v1.0
- **Status:** Draft
- **Owner:** Test Team
- **Related:** DOC-API-XXX, DOC-ARCH-XXX

### Domain Layer / Admission (Value Object)

#### [DOC-COMP-005: PatientIdentity](DOC-COMP-005_patientidentity.md)

- **Version:** v1.0
- **Status:** Draft
- **Owner:** Domain Layer Team
- **Related:** DOC-API-XXX, DOC-ARCH-XXX

#### [DOC-COMP-009: BedLocation](DOC-COMP-009_bedlocation.md)

- **Version:** v1.0
- **Status:** Draft
- **Owner:** Domain Layer Team
- **Related:** DOC-API-XXX, DOC-ARCH-XXX

### Domain Layer / Monitoring

#### [DOC-COMP-002: PatientAggregate](DOC-COMP-002_patientaggregate.md)

- **Version:** v1.0
- **Status:** Draft
- **Owner:** Domain Layer Team
- **Related:** DOC-API-XXX, DOC-ARCH-XXX

#### [DOC-COMP-004: TelemetryBatch](DOC-COMP-004_telemetrybatch.md)

- **Version:** v1.0
- **Status:** Draft
- **Owner:** Domain Layer Team
- **Related:** DOC-API-XXX, DOC-ARCH-XXX

### Domain Layer / Monitoring (Value Object)

#### [DOC-COMP-007: MeasurementUnit](DOC-COMP-007_measurementunit.md)

- **Version:** v1.0
- **Status:** Draft
- **Owner:** Domain Layer Team
- **Related:** DOC-API-XXX, DOC-ARCH-XXX

#### [DOC-COMP-008: AlarmThreshold](DOC-COMP-008_alarmthreshold.md)

- **Version:** v1.0
- **Status:** Draft
- **Owner:** Domain Layer Team
- **Related:** DOC-API-XXX, DOC-ARCH-XXX

### Domain Layer / Provisioning

#### [DOC-COMP-003: DeviceAggregate](DOC-COMP-003_deviceaggregate.md)

- **Version:** v1.0
- **Status:** Draft
- **Owner:** Domain Layer Team
- **Related:** DOC-API-XXX, DOC-ARCH-XXX

### Domain Layer / Provisioning (Value Object)

#### [DOC-COMP-006: DeviceSnapshot](DOC-COMP-006_devicesnapshot.md)

- **Version:** v1.0
- **Status:** Draft
- **Owner:** Domain Layer Team
- **Related:** DOC-API-XXX, DOC-ARCH-XXX

### General

#### [DOC-COMP-032: Query Registry Pattern Implementation](DOC-COMP-032_query_registry.md)

- **Version:** 1.0
- **Status:** Active
- **Owner:** N/A
- **Tags:** database, query, type-safety, constants, prepared-statements
- **Related:** DOC-GUIDE-014, DOC-PROC-009, DOC-ARCH-017

### Infrastructure

#### [DOC-COMP-026: Data Caching Strategy](DOC-COMP-026_data_caching_strategy.md)

- **Version:** 1.0
- **Status:** Draft
- **Owner:** Infrastructure Team
- **Tags:** caching, vitals, waveforms, persistence, memory
- **Related:** DOC-ARCH-001_software_architecture.md, DOC-ARCH-005_data_flow_and_caching.md, DOC-ARCH-011_thread_model.md, DOC-COMP-027_sensor_integration.md

#### [DOC-COMP-027: Sensor Integration Architecture](DOC-COMP-027_sensor_integration.md)

- **Version:** 1.0
- **Status:** Draft
- **Owner:** Infrastructure Team
- **Tags:** sensors, shared-memory, ipc, memfd, ring-buffer
- **Related:** DOC-ARCH-001_software_architecture.md, DOC-ARCH-011_thread_model.md, DOC-COMP-026_data_caching_strategy.md, DOC-COMP-028_waveform_display.md

#### [DOC-COMP-029: Async Logging Architecture](DOC-COMP-029_async_logging.md)

- **Version:** 1.0
- **Status:** Draft
- **Owner:** Infrastructure Team
- **Tags:** logging, async, lock-free, spdlog, thread-safety
- **Related:** DOC-ARCH-001_software_architecture.md, DOC-ARCH-011_thread_model.md, DOC-GUIDE-012_logging_guidelines.md

### Infrastructure Layer / Adapters

#### [DOC-COMP-022: SettingsManager](DOC-COMP-022_settingsmanager.md)

- **Version:** v1.0
- **Status:** Draft
- **Owner:** Infrastructure Team
- **Tags:** infrastructure, settings, configuration, sqlite, singleton
- **Related:** DOC-COMP-018, DOC-COMP-012, DOC-COMP-013

### Infrastructure Layer / Caching

#### [DOC-COMP-023: VitalsCache](DOC-COMP-023_vitalscache.md)

- **Version:** v1.0
- **Status:** Draft
- **Owner:** Infrastructure Team
- **Tags:** infrastructure, caching, vitals, memory, thread-safe
- **Related:** DOC-COMP-010, DOC-ARCH-005

#### [DOC-COMP-024: WaveformCache](DOC-COMP-024_waveformcache.md)

- **Version:** v1.0
- **Status:** Draft
- **Owner:** Infrastructure Team
- **Tags:** infrastructure, caching, waveform, memory, circular-buffer
- **Related:** DOC-COMP-010, DOC-ARCH-005

### Infrastructure Layer / Logging

#### [DOC-COMP-025: LogService](DOC-COMP-025_logservice.md)

- **Version:** v1.0
- **Status:** Draft
- **Owner:** Infrastructure Team
- **Tags:** infrastructure, logging, asynchronous, non-blocking, queue
- **Related:** DOC-COMP-018, DOC-ARCH-002

### Infrastructure Layer / Persistence

#### [DOC-COMP-018: DatabaseManager](DOC-COMP-018_databasemanager.md)

- **Version:** v1.0
- **Status:** Draft
- **Owner:** Infrastructure Team
- **Tags:** infrastructure, database, sqlite, connection-management, transaction, orm
- **Related:** DOC-ARCH-002, DOC-COMP-014, DOC-COMP-015

#### [DOC-COMP-019: SQLitePatientRepository](DOC-COMP-019_sqlitepatientrepository.md)

- **Version:** v1.0
- **Status:** Draft
- **Owner:** Infrastructure Team
- **Tags:** infrastructure, repository, sqlite, orm, patient, persistence
- **Related:** DOC-ARCH-002, DOC-COMP-001, DOC-COMP-014, DOC-COMP-018

#### [DOC-COMP-020: SQLiteTelemetryRepository](DOC-COMP-020_sqlitetelemetryrepository.md)

- **Version:** v1.0
- **Status:** Draft
- **Owner:** Infrastructure Team
- **Tags:** infrastructure, repository, telemetry, sqlite, persistence
- **Related:** DOC-COMP-015, DOC-COMP-018, DOC-COMP-010

#### [DOC-COMP-021: SQLiteAlarmRepository](DOC-COMP-021_sqlitealarmrepository.md)

- **Version:** v1.0
- **Status:** Draft
- **Owner:** Infrastructure Team
- **Tags:** infrastructure, repository, alarm, sqlite, persistence
- **Related:** DOC-COMP-016, DOC-COMP-018, DOC-COMP-010

### UI/Visualization

#### [DOC-COMP-028: Waveform Display Implementation](DOC-COMP-028_waveform_display.md)

- **Version:** 1.0
- **Status:** Draft
- **Owner:** UI/UX Team
- **Tags:** waveforms, canvas, qml, ecg, pleth, rendering
- **Related:** DOC-ARCH-007_ui_ux_guide.md, DOC-ARCH-011_thread_model.md, DOC-COMP-026_data_caching_strategy.md, DOC-COMP-027_sensor_integration.md

---

[Back to Master Index](../00_INDEX.md)
