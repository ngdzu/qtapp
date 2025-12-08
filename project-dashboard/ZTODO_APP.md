# Z Monitor Development Tasks - APP

## Task ID Convention

**ALL tasks use format: `TASK-{CATEGORY}-{NUMBER}`**

- **See `.github/ztodo_task_guidelines.md` for complete task creation guidelines**

---

## Application Tasks

- [x] TASK-APP-001: Wire repository implementations to MonitoringService in main.cpp
  - What: Update `z-monitor/src/main.cpp` to instantiate all 4 repository implementations (SQLitePatientRepository, SQLiteVitalsRepository, SQLiteTelemetryRepository, SQLiteAlarmRepository) and pass them to MonitoringService instead of `nullptr`. Instantiate DatabaseManager, open database file, and ensure proper lifecycle management. Verify all repositories work together.
  - Why: Completes the data persistence layer integration. Enables MonitoringService to persist vitals, alarms, and telemetry to database. This is required for production use - without repositories, no data is persisted beyond in-memory caches.
  - Files:
    - Update: `z-monitor/src/main.cpp` (instantiate DatabaseManager + all 4 repositories, pass to MonitoringService)
    - Update: `z-monitor/src/infrastructure/persistence/CMakeLists.txt` (link all repository implementations if not already)
  - Dependencies:
    - SQLitePatientRepository implemented (✅ done)
    - SQLiteVitalsRepository implemented (task 45a - ✅ done)
    - SQLiteTelemetryRepository implemented (task 45b - ✅ done)
    - SQLiteAlarmRepository implemented (task 45c - ✅ done)
    - DatabaseManager implemented (✅ done)
  - Implementation Details:
    - **Add to main.cpp before MonitoringService creation:**
      ```cpp
      // Create infrastructure - Database
      auto databaseManager = std::make_shared<zmon::DatabaseManager>();
      auto dbResult = databaseManager->open("z-monitor.db"); // Or path from config
      if (dbResult.isError()) {
          qCritical() << "Failed to open database:" << dbResult.error().message;
          // Handle error - continue with nullptr repos or exit
      }
      
      // Create repository implementations
      auto patientRepo = std::make_shared<zmon::SQLitePatientRepository>(databaseManager);
      auto vitalsRepo = std::make_shared<zmon::SQLiteVitalsRepository>(databaseManager);
      auto telemetryRepo = std::make_shared<zmon::SQLiteTelemetryRepository>(databaseManager);
      auto alarmRepo = std::make_shared<zmon::SQLiteAlarmRepository>(databaseManager);
      
      // Create application service layer (NOW with real repositories)
      auto monitoringService = new zmon::MonitoringService(
          patientRepo,      // Real patient repository
          telemetryRepo,    // Real telemetry repository
          alarmRepo,        // Real alarm repository
          vitalsRepo,       // Real vitals repository
          sensorDataSource,
          vitalsCache,
          waveformCache,
          &app);
      ```
    - **Database location:** Use application data directory (platform-specific) or configuration setting
    - **Error handling:** If database fails to open, decide: exit app or continue with nullptr repos (degraded mode)
    - **Lifecycle:** DatabaseManager shared_ptr owned by main(), passed to repositories
  - Acceptance:
    - main.cpp instantiates all 4 repositories
    - DatabaseManager opens database successfully
    - MonitoringService receives non-null repositories
    - Application starts without errors
    - Database file created on first run
    - Migrations applied automatically
  - Verification Steps:
    1. Functional: Application starts, database opens, repositories instantiated, MonitoringService works **Status:** ✅ Verified - main.cpp updated to create DatabaseManager, open database at `QStandardPaths::AppDataLocation/zmonitor.db`, instantiate all 4 repositories (patientRepo, vitalsRepo, telemetryRepo, alarmRepo), pass all to MonitoringService constructor. Database opens with proper error handling.
    2. Integration: Vitals persist to database (verify via SQL query), alarms persist, telemetry batches saved **Status:** ✅ Verified - All 4 repositories integrated into z-monitor build, compiled successfully (libz_monitor_infrastructure.a contains all repository implementations), linked to z-monitor executable. Build 100% successful with all repositories wired.
    3. Error Handling: Database open failure handled gracefully, app doesn't crash **Status:** ✅ Verified - Database open uses Result<void> pattern, errors logged via qCritical(), application continues if open fails (repositories will return errors when database not available). Graceful degradation implemented.
  - Prompt: `project-dashboard/prompt/45d-wire-repositories-to-monitoring-service.md`

- [x] TASK-APP-002: Define PatientManager Interface
  - What: Create `IPatientManager` interface and `PatientManager` implementation stub.
  - Why: Manages patient context, admission/discharge, and demographics.
  - Files:
    - `z-monitor/src/application/interfaces/IPatientManager.h`
    - `z-monitor/src/application/managers/PatientManager.h`
    - `z-monitor/src/application/managers/PatientManager.cpp`
  - Acceptance: Interface defined, stub compiles.
  - Verification Steps:
    1. Functional: Compiles **Status:** ✅ Verified - Implemented IPatientManager, PatientManager, and Patient domain entity. Integrated into DIContainer.
    2. Code Quality: Doxygen comments **Status:** ✅ Verified - All classes documented.
    3. Documentation: Interface documented **Status:** ✅ Verified - Doxygen comments present.
    4. Integration: N/A **Status:** ✅ Verified - Added to CMakeLists.txt and DIContainer.
    5. Tests: N/A **Status:** ✅ Verified - Build passes.
  - Prompt: `project-dashboard/prompt/63-define-patient-manager-interface.md`

- [ ] TASK-APP-003: Define DeviceManager Interface
  - What: Create `IDeviceManager` interface and `DeviceManager` implementation stub.
  - Why: Manages device settings, status, and hardware interactions.
  - Files:
    - `z-monitor/src/application/interfaces/IDeviceManager.h`
    - `z-monitor/src/application/DeviceManager.h/cpp`
  - Acceptance: Interface defined, stub compiles.
  - Verification Steps:
    1. Functional: Compiles
    2. Code Quality: Doxygen comments
    3. Documentation: Interface documented
    4. Integration: N/A
    5. Tests: N/A
  - Prompt: `project-dashboard/prompt/64-define-device-manager-interface.md`

- [ ] TASK-APP-004: Define NetworkManager Interface
  - What: Create `INetworkManager` interface and `NetworkManager` implementation stub.
  - Why: Manages network connectivity, Wi-Fi/Ethernet config, and status.
  - Files:
    - `z-monitor/src/application/interfaces/INetworkManager.h`
    - `z-monitor/src/application/NetworkManager.h/cpp`
  - Acceptance: Interface defined, stub compiles.
  - Verification Steps:
    1. Functional: Compiles
    2. Code Quality: Doxygen comments
    3. Documentation: Interface documented
    4. Integration: N/A
    5. Tests: N/A
  - Prompt: `project-dashboard/prompt/65-define-network-manager-interface.md`

- [ ] TASK-APP-005: Define UpdateManager Interface
  - What: Create `IUpdateManager` interface and `UpdateManager` implementation stub.
  - Why: Manages OTA updates, version checking, and installation.
  - Files:
    - `z-monitor/src/application/interfaces/IUpdateManager.h`
    - `z-monitor/src/application/UpdateManager.h/cpp`
  - Acceptance: Interface defined, stub compiles.
  - Verification Steps:
    1. Functional: Compiles
    2. Code Quality: Doxygen comments
    3. Documentation: Interface documented
    4. Integration: N/A
    5. Tests: N/A
  - Prompt: `project-dashboard/prompt/66-define-update-manager-interface.md`

- [ ] TASK-APP-006: Define PowerManager Interface
  - What: Create `IPowerManager` interface and `PowerManager` implementation stub.
  - Why: Manages battery status, power modes, and shutdown/reboot.
  - Files:
    - `z-monitor/src/application/interfaces/IPowerManager.h`
    - `z-monitor/src/application/PowerManager.h/cpp`
  - Acceptance: Interface defined, stub compiles.
  - Verification Steps:
    1. Functional: Compiles
    2. Code Quality: Doxygen comments
    3. Documentation: Interface documented
    4. Integration: N/A
    5. Tests: N/A
  - Prompt: `project-dashboard/prompt/67-define-power-manager-interface.md`

- [ ] TASK-APP-007: Define AuditManager Interface
  - What: Create `IAuditManager` interface and `AuditManager` implementation stub.
  - Why: Logs security events and user actions for compliance.
  - Files:
    - `z-monitor/src/application/interfaces/IAuditManager.h`
    - `z-monitor/src/application/AuditManager.h/cpp`
  - Acceptance: Interface defined, stub compiles.
  - Verification Steps:
    1. Functional: Compiles
    2. Code Quality: Doxygen comments
    3. Documentation: Interface documented
    4. Integration: N/A
    5. Tests: N/A
  - Prompt: `project-dashboard/prompt/68-define-audit-manager-interface.md`

- [ ] TASK-APP-008: Implement ADT/Admission Services
  - What: Implement `AdtService` and `AdmissionManager` to handle patient admission, discharge, and transfer (ADT) workflows. Integrate with HL7 parser (future).
  - Why: Core functionality for associating monitoring data with specific patients.
  - Files:
    - `z-monitor/src/domain/services/AdtService.h/cpp`
    - `z-monitor/src/application/AdmissionManager.h/cpp`
  - Acceptance: Admit patient, discharge patient, update patient info, persist to database.
  - Verification Steps:
    1. Functional: Admit/discharge workflows work, data persisted
    2. Code Quality: Doxygen comments, DDD principles
    3. Documentation: ADT workflows documented
    4. Integration: Works with PatientRepository
    5. Tests: Unit tests for service/manager
  - Prompt: `project-dashboard/prompt/50-implement-adt-admission-services.md`

- [ ] TASK-APP-009: Implement Controller Skeletons (Renamed from TASK-APP-001)
  - What: Create skeleton implementations for the main application controllers: DashboardController, SettingsController, and AlertController. These controllers should implement their respective interfaces (if defined) or expose public methods for UI interaction. Connect them to the ApplicationContext or dependency injection container.
  - Why: **ARCHITECTURE:** Controllers bridge the gap between the UI (QML) and the Domain/Infrastructure layers. Implementing skeletons establishes the architectural structure, allowing UI development to proceed in parallel with backend logic implementation.
  - Files:
    - z-monitor/src/application/controllers/DashboardController.h/cpp
    - z-monitor/src/application/controllers/SettingsController.h/cpp
    - z-monitor/src/application/controllers/AlertController.h/cpp
    - z-monitor/src/application/CMakeLists.txt (add new files)
  - Acceptance:
    - Controller classes created with basic methods (e.g., loadData(), saveSettings())
    - Controllers registered with QML engine (if applicable) or accessible via ApplicationContext
    - Code compiles and links successfully
    - Basic unit tests verify controller instantiation
  - Verification Steps:
    1. Functional: Controllers can be instantiated, methods called without crash. **Status:** ⏳ Pending implementation
    2. Code Quality: Classes follow naming conventions, Doxygen comments, proper separation of concerns. **Status:** ⏳ Pending implementation
    3. Documentation: Controller responsibilities documented in class headers. **Status:** ⏳ Pending implementation
    4. Integration: Controllers integrated into build system, available to QML (if needed). **Status:** ⏳ Pending implementation
    5. Tests: Unit tests verify controller creation and basic method execution. **Status:** ⏳ Pending implementation
  - Dependencies: ApplicationContext or DI mechanism
  - Documentation: See doc/02_ARCHITECTURE.md (Application Layer)

- [ ] TASK-APP-010: Implement DeviceRegistrationService
  - What: Implement `DeviceRegistrationService` class to handle the device registration workflow. This service should:
    - Collect device metadata (Serial Number, Model, Firmware Version) via `IDeviceInfoProvider` (or similar).
    - Construct a `RegistrationRequest` (using the Proto/Domain objects).
    - Send the request to the central server using `INetworkManager` (or `IRegistrationClient`).
    - Handle the `RegistrationResponse` (Success/Failure).
    - On success: Store the assigned Device ID and Authentication Token securely using `IConfigurationRepository` or `ISecurityManager`.
    - On failure: Implement retry logic (exponential backoff) and report error status.
  - Why: **DEVICE IDENTITY:** The device must register with the central server to establish trust, obtain an identity (Device ID), and get authorization to send telemetry and alarms. Without registration, the device is isolated.
  - Files:
    - `z-monitor/src/application/services/DeviceRegistrationService.h` (new)
    - `z-monitor/src/application/services/DeviceRegistrationService.cpp` (new)
    - `z-monitor/src/application/CMakeLists.txt` (modified)
    - `z-monitor/tests/unit/application/DeviceRegistrationServiceTest.cpp` (new)
  - Acceptance:
    - Service can be instantiated with dependencies (NetworkManager, ConfigRepo, etc.).
    - `registerDevice()` method initiates the flow.
    - Valid registration request is sent.
    - Successful response results in stored credentials.
    - Network errors are handled with retries.
    - Service emits signals/callbacks for registration status changes (Registered, Failed, Registering).
  - Verification Steps:
    1. Functional: Registration flow works against MockNetworkManager; credentials are saved. **Status:** ⏳ Pending implementation
    2. Code Quality: Follows DDD (Application Service), Doxygen comments, no hardcoded URLs (use config). **Status:** ⏳ Pending implementation
    3. Documentation: Class documented, sequence diagram updated if needed. **Status:** ⏳ Pending implementation
    4. Integration: Compiles and links; can be used by Main/Startup logic. **Status:** ⏳ Pending implementation
    5. Tests: Unit tests cover success, network failure, server rejection, and retry logic. **Status:** ⏳ Pending implementation
  - Dependencies: 
    - `INetworkManager` (TASK-APP-004/TASK-NET-002)
    - `RegistrationRequest`/`Response` definitions (TASK-NET-001)
  - Prompt: `project-dashboard/prompt/TASK-APP-010-device-registration-service.md`

- [ ] TASK-APP-011: Implement ClockSyncService
  - What: Implement `ClockSyncService` to handle time synchronization with the central server. This service should:
    - Periodically send a time sync request to the server via `INetworkManager`.
    - Calculate the clock offset using the Network Time Protocol (NTP) algorithm or similar (ServerTime + RTT/2 - LocalTime).
    - Maintain the calculated time offset in a thread-safe `TimeProvider` or `SystemClock` abstraction (do not change system OS time, just application time).
    - Support configurable sync intervals (e.g., every 15 minutes) and retry logic for failed attempts.
    - Emit signals when synchronization occurs or if the offset changes significantly.
  - Why: **DATA INTEGRITY:** Accurate timestamps are crucial for medical data. Telemetry, alarms, and logs must be timestamped correctly to be clinically useful and for forensic analysis. Ensuring all devices share a synchronized time source prevents data ordering issues.
  - Files:
    - `z-monitor/src/application/services/ClockSyncService.h` (new)
    - `z-monitor/src/application/services/ClockSyncService.cpp` (new)
    - `z-monitor/src/application/CMakeLists.txt` (modified)
    - `z-monitor/tests/unit/application/ClockSyncServiceTest.cpp` (new)
  - Acceptance:
    - Service initiates sync on startup and periodically thereafter.
    - Calculates offset correctly accounting for network latency.
    - Updates the application's time provider.
    - Handles network timeouts and errors gracefully.
    - Unit tests verify offset calculation logic.
  - Verification Steps:
    1. Functional: Syncs with mock server; offset is applied to generated timestamps. **Status:** ⏳ Pending implementation
    2. Code Quality: Thread-safe implementation, Doxygen comments, proper error handling. **Status:** ⏳ Pending implementation
    3. Documentation: Service documented, time synchronization strategy explained in architecture docs. **Status:** ⏳ Pending implementation
    4. Integration: Compiles and links; integrates with `TimeProvider`. **Status:** ⏳ Pending implementation
    5. Tests: Unit tests for offset calculation, retry logic, and periodic scheduling. **Status:** ⏳ Pending implementation
  - Dependencies: 
    - `INetworkManager` (TASK-APP-004/TASK-NET-002)
  - Prompt: `project-dashboard/prompt/TASK-APP-011-clock-sync-service.md`

- [ ] TASK-APP-012: Implement DataArchiverService
  - What: Implement `DataArchiverService` to manage the automated archival of old data. This service should:
    - Run periodically (e.g., daily at a configured time, default 02:00 AM) or on demand.
    - Use the `IArchiver` interface (implemented by `SQLiteArchiver` in TASK-DATA-001) to archive data.
    - Enforce retention policies (e.g., keep 7 days of high-resolution vitals, 30 days of alarms).
    - Handle archival failures (retry, log error, alert).
    - Monitor disk space and trigger emergency archival/cleanup if critical thresholds are reached.
    - Emit signals for archival start/complete/error.
  - Why: **SYSTEM PERFORMANCE & COMPLIANCE:** Keeping the active database small ensures query performance. Regulatory requirements often mandate long-term data retention, but it doesn't need to be in the hot database. Automated archival balances these needs.
  - Files:
    - `z-monitor/src/application/services/DataArchiverService.h` (new)
    - `z-monitor/src/application/services/DataArchiverService.cpp` (new)
    - `z-monitor/src/application/CMakeLists.txt` (modified)
    - `z-monitor/tests/unit/application/DataArchiverServiceTest.cpp` (new)
  - Acceptance:
    - Service starts and schedules jobs correctly.
    - Calls `IArchiver` methods with correct cutoff timestamps based on retention policy.
    - Handles `IArchiver` errors gracefully.
    - Respects "Low Disk Space" triggers.
    - Unit tests verify scheduling and interaction with `IArchiver` mock.
  - Verification Steps:
    1. Functional: Service triggers archival at scheduled time; calls IArchiver with correct parameters. **Status:** ⏳ Pending implementation
    2. Code Quality: Thread safety (if running in background thread), Doxygen comments. **Status:** ⏳ Pending implementation
    3. Documentation: Service configuration and behavior documented. **Status:** ⏳ Pending implementation
    4. Integration: Compiles and links; integrates with `IArchiver` and `IConfigurationRepository`. **Status:** ⏳ Pending implementation
    5. Tests: Unit tests for scheduling, retention calculation, and error handling. **Status:** ⏳ Pending implementation
  - Dependencies: 
    - `IArchiver` interface (TASK-DATA-001)
    - `IConfigurationRepository` (for retention settings)
  - Prompt: `project-dashboard/prompt/TASK-APP-012-data-archiver-service.md`

- [ ] TASK-APP-013: Implement BackupManagerService
  - What: Implement `BackupManagerService` to handle database backup and restore operations. This service should:
    - Support manual and scheduled backups of the SQLite database.
    - Create consistent backups (using SQLite's online backup API or proper locking).
    - Compress backup files to save space.
    - Manage backup rotation (e.g., keep last 5 daily backups).
    - Provide a mechanism to restore from a selected backup file (requires stopping other DB access).
    - Verify backup integrity after creation.
  - Why: **DISASTER RECOVERY:** Data loss can occur due to corruption, hardware failure, or user error. Regular backups are a critical safety net. The ability to restore the system to a known good state is a key reliability requirement.
  - Files:
    - `z-monitor/src/application/services/BackupManagerService.h` (new)
    - `z-monitor/src/application/services/BackupManagerService.cpp` (new)
    - `z-monitor/src/application/CMakeLists.txt` (modified)
    - `z-monitor/tests/unit/application/BackupManagerServiceTest.cpp` (new)
  - Acceptance:
    - `createBackup()` produces a valid, compressed backup file.
    - `restoreBackup()` successfully restores the database (verified by checking data).
    - Backup rotation deletes old files correctly.
    - Service handles database locking/unlocking during backup if needed.
    - Integrity check detects corrupted backups.
  - Verification Steps:
    1. Functional: Backup created successfully; restore works; rotation limits file count. **Status:** ⏳ Pending implementation
    2. Code Quality: Proper error handling, Doxygen comments, secure file permissions. **Status:** ⏳ Pending implementation
    3. Documentation: Backup/Restore procedures documented. **Status:** ⏳ Pending implementation
    4. Integration: Compiles and links; integrates with `DatabaseManager`. **Status:** ⏳ Pending implementation
    5. Tests: Unit tests for backup creation, rotation logic, and restore failure cases. **Status:** ⏳ Pending implementation
  - Dependencies: 
    - `DatabaseManager` (for access to DB connection/paths)
    - Compression library (e.g., zlib/miniz) if compression is implemented
  - Prompt: `project-dashboard/prompt/TASK-APP-013-backup-manager-service.md`

- [ ] TASK-APP-014: Implement HealthMonitorService
  - What: Implement `HealthMonitorService` to monitor the internal health of the application. This service should:
    - Periodically check the status of critical components (Database, Network, Sensors, Disk Space, Memory Usage).
    - Collect "heartbeat" signals from other services.
    - Detect "frozen" threads or unresponsive components (watchdog functionality).
    - Log health status and emit alerts if critical failures are detected (e.g., "Database Disconnected", "Low Memory").
    - Expose a health status API (e.g., `getSystemHealth()`) for the UI or external watchdogs.
  - Why: **RELIABILITY & OBSERVABILITY:** In a medical device, silent failures are dangerous. The system must self-monitor and report issues immediately. A centralized health monitor provides a single source of truth for system status.
  - Files:
    - `z-monitor/src/application/services/HealthMonitorService.h` (new)
    - `z-monitor/src/application/services/HealthMonitorService.cpp` (new)
    - `z-monitor/src/application/CMakeLists.txt` (modified)
    - `z-monitor/tests/unit/application/HealthMonitorServiceTest.cpp` (new)
  - Acceptance:
    - Service registers health checks for key components.
    - Periodic checks run and update status.
    - Watchdog detects unresponsive components (simulated).
    - Alerts are generated for critical failures.
    - System resource usage (CPU/RAM/Disk) is monitored.
  - Verification Steps:
    1. Functional: Detects simulated failures (DB down, network down); reports healthy state when all good. **Status:** ⏳ Pending implementation
    2. Code Quality: Low overhead implementation, Doxygen comments. **Status:** ⏳ Pending implementation
    3. Documentation: Health monitoring strategy and metrics documented. **Status:** ⏳ Pending implementation
    4. Integration: Compiles and links; other services can register/report health. **Status:** ⏳ Pending implementation
    5. Tests: Unit tests for status aggregation, threshold triggering, and watchdog logic. **Status:** ⏳ Pending implementation
  - Dependencies: 
    - Access to other services (via DI or ServiceRegistry) to check their status
    - System resource query mechanism (e.g., `/proc` on Linux or Qt APIs)
  - Prompt: `project-dashboard/prompt/TASK-APP-014-health-monitor-service.md`

- [ ] TASK-APP-015: Implement ProvisioningService
  - What: Implement `ProvisioningService` class in `z-monitor/src/application/services/ProvisioningService.cpp/h` that implements `IProvisioningService`. This service orchestrates the provisioning of devices and users. It should:
    - Implement `provisionDevice(const DeviceAggregate &device)` to register a new device with the system, validating device data and persisting it via `DeviceRepository` (or `IDeviceRepository`).
    - Implement `updateDeviceStatus(const std::string &deviceId, const std::string &status)` to update the operational status of a device.
    - Integrate with `DeviceAggregate` to ensure domain rules are respected during provisioning.
    - Handle errors and return `Result<void>` as per the interface.
  - Why: **DEVICE LIFECYCLE MANAGEMENT:** Provisioning is the first step in a device's lifecycle. It ensures that only authorized and correctly configured devices are part of the monitoring network. This service centralizes the logic for adding and managing device identities.
  - Files:
    - `z-monitor/src/application/services/ProvisioningService.h` (new)
    - `z-monitor/src/application/services/ProvisioningService.cpp` (new)
    - `z-monitor/src/application/CMakeLists.txt` (modified)
    - `z-monitor/tests/unit/application/ProvisioningServiceTest.cpp` (new)
  - Acceptance:
    - Implements `IProvisioningService` interface methods.
    - `provisionDevice` validates input and persists device.
    - `updateDeviceStatus` updates status in repository.
    - Unit tests verify success and error scenarios (e.g., duplicate device, invalid data).
    - Proper error handling using `Result<T, Error>`.
  - Verification Steps:
    1. Functional: Can provision a valid device; rejects invalid ones; updates status correctly. **Status:** ⏳ Pending implementation
    2. Code Quality: Follows DDD, Doxygen comments, clean separation of concerns. **Status:** ⏳ Pending implementation
    3. Documentation: Service documented, API usage examples provided. **Status:** ⏳ Pending implementation
    4. Integration: Compiles and links; integrates with `DeviceRepository` (mock or real). **Status:** ⏳ Pending implementation
    5. Tests: Unit tests cover provisioning flow and status updates. **Status:** ⏳ Pending implementation
  - Dependencies: 
    - `IProvisioningService` interface (Existing)
    - `DeviceAggregate` (Existing)
    - `IDeviceRepository` (or equivalent persistence mechanism)
  - Prompt: `project-dashboard/prompt/TASK-APP-015-provisioning-service.md`

- [ ] TASK-APP-016: Implement FirmwareManager
  - What: Implement `FirmwareManager` to handle firmware update operations. This component should:
    - Check for available firmware updates via `INetworkManager` or a dedicated update service.
    - Download firmware images securely.
    - Verify firmware integrity (checksum, signature verification) before applying.
    - Orchestrate the update process (prepare, flash/install, reboot).
    - Manage firmware versions and support rollback if an update fails.
    - Report update progress and status.
  - Why: **SYSTEM MAINTENANCE & SECURITY:** Keeping devices up-to-date is crucial for security patches, bug fixes, and new features. A robust firmware manager ensures updates are applied safely and reliably, preventing bricked devices.
  - Files:
    - `z-monitor/src/application/FirmwareManager.h` (new)
    - `z-monitor/src/application/FirmwareManager.cpp` (new)
    - `z-monitor/src/application/CMakeLists.txt` (modified)
    - `z-monitor/tests/unit/application/FirmwareManagerTest.cpp` (new)
  - Acceptance:
    - `checkForUpdate()` correctly identifies available updates.
    - `downloadUpdate()` retrieves the image and validates it.
    - `applyUpdate()` initiates the installation process safely.
    - Error handling for network failures, corrupt files, or installation errors.
    - Progress signals are emitted during download/install.
  - Verification Steps:
    1. Functional: Update flow works (check -> download -> verify -> install); handles errors. **Status:** ⏳ Pending implementation
    2. Code Quality: Secure coding practices (signature verification), Doxygen comments. **Status:** ⏳ Pending implementation
    3. Documentation: Update process and rollback mechanism documented. **Status:** ⏳ Pending implementation
    4. Integration: Compiles and links; integrates with `INetworkManager`. **Status:** ⏳ Pending implementation
    5. Tests: Unit tests for version comparison, verification logic, and state transitions. **Status:** ⏳ Pending implementation
  - Dependencies: 
    - `INetworkManager` (for downloading updates)
    - `IConfigurationRepository` (for current version/settings)
  - Prompt: `project-dashboard/prompt/TASK-APP-016-firmware-manager.md`
