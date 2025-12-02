---
doc_id: DOC-COMP-011
title: AdmissionService
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
  - DOC-COMP-005  # PatientIdentity
  - DOC-COMP-009  # BedLocation
  - DOC-COMP-014  # IPatientRepository
related_tasks:
  - TASK-3B-001  # Phase 3B Migration
related_requirements:
  - REQ-ADM-001  # Admission requirements
  - REQ-FUN-ADT-001  # ADT workflow
tags:
  - application-service
  - use-case-orchestration
  - admission
  - adt-workflow
  - patient-management
diagram_files:
  - DOC-COMP-011_admissionservice_dependencies.mmd
  - DOC-COMP-011_admissionservice_dependencies.svg
  - DOC-COMP-011_adt_workflow.mmd
  - DOC-COMP-011_adt_workflow.svg
---

# DOC-COMP-011: AdmissionService

## 1. Overview

**Purpose:** Application service orchestrating patient admission, discharge, and transfer (ADT) workflow, ensuring business rules are enforced and events are logged for audit purposes.

**Responsibilities:**
- Execute patient admission workflow (manual entry, barcode scan, central station push)
- Execute patient discharge workflow
- Execute patient transfer workflow (to another device)
- Track current admission state (NotAdmitted, Admitted, Discharged)
- Log admission events to IActionLogRepository for audit
- Emit Qt signals for UI updates (patientAdmitted, patientDischarged, patientTransferred, admissionStateChanged)
- Enforce business rule: "Only one patient can be admitted at a time"

**Layer:** Application Layer

**Module:** `z-monitor/src/application/services/AdmissionService.h`

**Thread Affinity:** Application Services Thread (normal priority)

**Dependencies:**
- **Domain Value Objects:** PatientIdentity (DOC-COMP-005), BedLocation (DOC-COMP-009)
- **Repository Interfaces:** IActionLogRepository (action logging for audit)
- **Result Type:** Result<T, Error> for error handling
- **Settings:** SettingsManager (device label retrieval)

## 2. Architecture

<!-- TODO: Add dependencies diagram -->
<!-- TODO: Add ADT workflow diagram -->

**Key Design Decisions:**
- **Decision 1: State Machine Pattern** - Admission state tracked via AdmissionState enum (NotAdmitted, Admitted, Discharged), enforcing valid state transitions
- **Decision 2: Audit Logging** - All admission events logged to IActionLogRepository with event type, MRN, name, bed location, admission source, and timestamp for regulatory compliance
- **Decision 3: Result<T> Error Handling** - Operations return Result<void> to communicate success/failure with error details, avoiding exceptions
- **Decision 4: Signal-Based UI Updates** - Emits Qt signals (patientAdmitted, patientDischarged) for UI controllers to update QML views

**Design Patterns Used:**
- **Application Service Pattern:** Coordinates ADT use cases without containing business logic (business rules enforced in PatientAggregate)
- **State Machine Pattern:** AdmissionState enum with explicit state transitions (NotAdmitted → Admitted → Discharged)
- **Observer Pattern (Qt Signals/Slots):** Emits signals for UI updates and audit logging
- **Dependency Injection:** Constructor injection of IActionLogRepository for testability

## 3. Public API

**Interface Specification:** QObject-based Qt service with state machine and signal/slot communication.

### 3.1 Key Classes

```cpp
class AdmissionService : public QObject {
    Q_OBJECT

public:
    /**
     * @brief Admission source types.
     */
    enum class AdmissionSource {
        Manual,          ///< Manual entry by clinician
        Barcode,         ///< Barcode scan
        CentralStation   ///< Central station push
    };
    Q_ENUM(AdmissionSource)

    /**
     * @brief Admission state.
     */
    enum class AdmissionState {
        NotAdmitted,     ///< No patient admitted
        Admitted,        ///< Patient currently admitted
        Discharged       ///< Patient discharged
    };
    Q_ENUM(AdmissionState)

    /**
     * @brief Patient admission information.
     */
    struct AdmissionInfo {
        QString mrn;                    ///< Medical Record Number
        QString name;                   ///< Patient name
        QString bedLocation;            ///< Bed/room location
        QDateTime admittedAt;           ///< Admission timestamp
        QDateTime dischargedAt;         ///< Discharge timestamp (NULL if admitted)
        AdmissionSource admissionSource; ///< Source of admission
        QString deviceLabel;             ///< Device that admitted patient
    };

    /**
     * @brief Constructor with dependency injection.
     *
     * @param actionLogRepo Action log repository for logging patient management actions
     * @param parent Parent QObject for Qt ownership
     */
    explicit AdmissionService(IActionLogRepository* actionLogRepo = nullptr, QObject* parent = nullptr);

    /**
     * @brief Destructor.
     */
    ~AdmissionService() override = default;

    // === Admission Workflow ===

    /**
     * @brief Admits a patient to the device.
     *
     * @param patientIdentity Patient identity information (MRN, name, DOB, sex)
     * @param bedLocation Bed/room location (e.g., "ICU-4-Bed-2")
     * @param admissionSource Source of admission (Manual, Barcode, CentralStation)
     * @return Result<void> Success or error with details if admission fails
     * @throws None (uses Result<T> for error handling)
     * @note Enforces business rule: "Only one patient can be admitted at a time"
     */
    Result<void> admitPatient(const PatientIdentity& patientIdentity,
                              const BedLocation& bedLocation,
                              AdmissionSource admissionSource);
    
    /**
     * @brief Discharges a patient from the device.
     *
     * @param mrn Medical Record Number of patient to discharge
     * @return Result<void> Success or error with details if discharge fails
     * @note Sets admission state to Discharged, records discharge timestamp
     */
    Result<void> dischargePatient(const QString& mrn);
    
    /**
     * @brief Transfers a patient to another device.
     *
     * @param mrn Medical Record Number of patient to transfer
     * @param targetDeviceLabel Target device label (e.g., "ICU-MON-05")
     * @return Result<void> Success or error with details if transfer fails
     * @note Logs transfer event with source and target device labels
     */
    Result<void> transferPatient(const QString& mrn, const QString& targetDeviceLabel);

    // === Queries ===

    /**
     * @brief Gets the current admission information.
     *
     * @return Current admission info or empty if no patient admitted
     */
    AdmissionInfo getCurrentAdmission() const;

    /**
     * @brief Checks if a patient is currently admitted.
     *
     * @return true if patient is admitted, false otherwise
     */
    bool isPatientAdmitted() const;

    /**
     * @brief Gets the admission state.
     *
     * @return Current admission state (NotAdmitted, Admitted, Discharged)
     */
    AdmissionState admissionState() const;

signals:
    /**
     * @brief Emitted when a patient is admitted.
     *
     * @param mrn Medical Record Number
     * @param name Patient name
     * @param bedLocation Bed location
     */
    void patientAdmitted(const QString& mrn, const QString& name, const QString& bedLocation);

    /**
     * @brief Emitted when a patient is discharged.
     *
     * @param mrn Medical Record Number
     */
    void patientDischarged(const QString& mrn);

    /**
     * @brief Emitted when a patient is transferred.
     *
     * @param mrn Medical Record Number
     * @param targetDeviceLabel Target device label
     */
    void patientTransferred(const QString& mrn, const QString& targetDeviceLabel);

    /**
     * @brief Emitted when admission state changes.
     *
     * @param state New admission state
     */
    void admissionStateChanged(AdmissionState state);

private:
    /**
     * @brief Logs an admission event to the database.
     *
     * @param eventType Event type ("admission", "discharge", "transfer")
     * @param mrn Medical Record Number
     * @param name Patient name
     * @param bedLocation Bed location
     * @param admissionSource Admission source
     * @param details Additional event details (JSON)
     */
    void logAdmissionEvent(const QString& eventType,
                           const QString& mrn,
                           const QString& name,
                           const QString& bedLocation,
                           AdmissionSource admissionSource,
                           const QString& details = QString());

    /**
     * @brief Gets the device label from settings.
     *
     * @return Device label string (e.g., "ICU-MON-04")
     */
    QString getDeviceLabel() const;

    IActionLogRepository* m_actionLogRepo;  ///< Action log repository (for dependency injection)
    AdmissionState m_admissionState = AdmissionState::NotAdmitted;
    AdmissionInfo m_currentAdmission;
};
```

### 3.2 Key Methods

**`admitPatient(patientIdentity, bedLocation, admissionSource)`**
- **Purpose:** Admit a patient to the device, transitioning from NotAdmitted to Admitted state
- **Parameters:** 
  - `patientIdentity`: PatientIdentity value object (MRN, name, DOB, sex, allergies)
  - `bedLocation`: BedLocation value object (room/bed identifier)
  - `admissionSource`: AdmissionSource enum (Manual, Barcode, CentralStation)
- **Returns:** `Result<void>` - Success if admission succeeded, Error with details if failed
- **Errors:** "Patient already admitted", "Invalid patient identity", "Invalid bed location"
- **Side Effects:**
  - Sets `m_admissionState` to `Admitted`
  - Records admission timestamp in `m_currentAdmission.admittedAt`
  - Logs admission event to IActionLogRepository
  - Emits `patientAdmitted(mrn, name, bedLocation)` signal
  - Emits `admissionStateChanged(Admitted)` signal

**`dischargePatient(mrn)`**
- **Purpose:** Discharge a patient from the device, transitioning from Admitted to Discharged state
- **Parameters:**
  - `mrn`: QString - Medical Record Number of patient to discharge
- **Returns:** `Result<void>` - Success if discharge succeeded, Error if patient not admitted or MRN mismatch
- **Errors:** "No patient admitted", "MRN mismatch"
- **Side Effects:**
  - Sets `m_admissionState` to `Discharged`
  - Records discharge timestamp in `m_currentAdmission.dischargedAt`
  - Logs discharge event to IActionLogRepository
  - Emits `patientDischarged(mrn)` signal
  - Emits `admissionStateChanged(Discharged)` signal

**`transferPatient(mrn, targetDeviceLabel)`**
- **Purpose:** Transfer a patient to another device
- **Parameters:**
  - `mrn`: QString - Medical Record Number of patient to transfer
  - `targetDeviceLabel`: QString - Target device label (e.g., "ICU-MON-05")
- **Returns:** `Result<void>` - Success if transfer succeeded
- **Errors:** "No patient admitted", "MRN mismatch", "Invalid target device label"
- **Side Effects:**
  - Logs transfer event to IActionLogRepository with source and target device labels
  - Emits `patientTransferred(mrn, targetDeviceLabel)` signal
  - Sets admission state to Discharged (patient no longer on this device)

## 4. Implementation Details

**Source File:** `z-monitor/src/application/services/AdmissionService.cpp`

**Admission Workflow:**
```cpp
Result<void> AdmissionService::admitPatient(const PatientIdentity& patientIdentity,
                                             const BedLocation& bedLocation,
                                             AdmissionSource admissionSource)
{
    // 1. Enforce business rule: only one patient at a time
    if (m_admissionState == AdmissionState::Admitted) {
        return Result<void>::error("ALREADY_ADMITTED", "Patient already admitted. Discharge current patient first.");
    }

    // 2. Validate patient identity
    if (!patientIdentity.isValid()) {
        return Result<void>::error("INVALID_IDENTITY", "Invalid patient identity");
    }

    // 3. Validate bed location
    if (!bedLocation.isValid()) {
        return Result<void>::error("INVALID_BED_LOCATION", "Invalid bed location");
    }

    // 4. Update admission state
    m_admissionState = AdmissionState::Admitted;
    m_currentAdmission.mrn = QString::fromStdString(patientIdentity.getMrn());
    m_currentAdmission.name = QString::fromStdString(patientIdentity.getName());
    m_currentAdmission.bedLocation = QString::fromStdString(bedLocation.getLocationId());
    m_currentAdmission.admittedAt = QDateTime::currentDateTime();
    m_currentAdmission.admissionSource = admissionSource;
    m_currentAdmission.deviceLabel = getDeviceLabel();

    // 5. Log admission event
    logAdmissionEvent("admission", m_currentAdmission.mrn, m_currentAdmission.name,
                      m_currentAdmission.bedLocation, admissionSource);

    // 6. Emit signals
    emit patientAdmitted(m_currentAdmission.mrn, m_currentAdmission.name, m_currentAdmission.bedLocation);
    emit admissionStateChanged(AdmissionState::Admitted);

    return Result<void>::success();
}
```

## 5. Usage Examples

### 5.1 Manual Patient Admission

```cpp
// Create patient identity
PatientIdentity identity("12345678", "John Doe", QDate(1985, 5, 15), PatientSex::Male);

// Create bed location
BedLocation bedLocation("ICU-4-Bed-2");

// Admit patient
auto result = admissionService->admitPatient(identity, bedLocation, AdmissionService::AdmissionSource::Manual);

if (result.isSuccess()) {
    qInfo() << "Patient admitted successfully";
} else {
    qWarning() << "Admission failed:" << result.error().message;
}
```

### 5.2 Patient Discharge

```cpp
QString mrn = "12345678";

auto result = admissionService->dischargePatient(mrn);

if (result.isSuccess()) {
    qInfo() << "Patient discharged successfully";
} else {
    qWarning() << "Discharge failed:" << result.error().message;
}
```

### 5.3 Patient Transfer

```cpp
QString mrn = "12345678";
QString targetDevice = "ICU-MON-05";

auto result = admissionService->transferPatient(mrn, targetDevice);

if (result.isSuccess()) {
    qInfo() << "Patient transferred to" << targetDevice;
} else {
    qWarning() << "Transfer failed:" << result.error().message;
}
```

## 6. Testing

### 6.1 Unit Tests

**Test File:** `z-monitor/tests/application/services/AdmissionServiceTest.cpp`

**Key Test Cases:**
- `test_AdmitPatient_Success()` - Verify successful admission with valid identity and bed location
- `test_AdmitPatient_AlreadyAdmitted_Fails()` - Verify admission fails if patient already admitted
- `test_DischargePatient_Success()` - Verify successful discharge
- `test_DischargePatient_NoPatientAdmitted_Fails()` - Verify discharge fails if no patient admitted
- `test_TransferPatient_Success()` - Verify successful transfer with valid target device label
- `test_AdmissionStateChanged_Signal_Emitted()` - Verify admissionStateChanged signal emitted on state transitions

### 6.2 Integration Tests

**Test File:** `z-monitor/tests/integration/AdmissionServiceIntegrationTest.cpp`

**Key Integration Scenarios:**
- End-to-end admission → discharge workflow with real IActionLogRepository
- Admission event logging verification (audit trail)
- Signal emission → UI controller update flow

## 7. Performance Considerations

**Latency Requirements:**
- Admission workflow: <100ms (non-critical, user-initiated action)
- Discharge workflow: <100ms
- Transfer workflow: <100ms

**Memory Management:**
- `AdmissionInfo` struct: ~200 bytes (lightweight, single instance in memory)
- IActionLogRepository: Database writes (non-blocking, queued to DB thread)

**Thread Safety:**
- Thread Affinity: Application Services Thread (normal priority)
- No concurrent access expected (user-initiated actions, serialized)

## 8. Error Handling

**Error Categories:**
- **Business Rule Violations:** "Patient already admitted", "No patient admitted"
- **Validation Errors:** "Invalid patient identity", "Invalid bed location"
- **Repository Errors:** IActionLogRepository write failures (logged but do not block operation)

**Recovery Strategies:**
- Business rule violations: Return error to user, do not change state
- Validation errors: Return error to user with specific field details
- Repository errors: Log error, continue operation (audit log writes are best-effort)

## 9. Security Considerations

**Authentication/Authorization:**
- Admission actions do not require authentication (delegated to UI layer, assumes authenticated user)

**Audit Logging:**
- All admission events logged to IActionLogRepository with event type, MRN, timestamp, device label, and admission source for regulatory compliance

## 10. Deployment

**Configuration:**
- Requires `deviceLabel` in SettingsManager (e.g., "ICU-MON-04")

**Dependencies:**
- IActionLogRepository (infrastructure layer, optional for testing)
- SettingsManager (configuration)

**Thread Affinity:**
- Runs on Application Services Thread (normal priority)

**Integration Points:**
- UI Controllers: Connect to `patientAdmitted()`, `patientDischarged()`, `patientTransferred()`, `admissionStateChanged()` signals

## 11. Related Documentation

**Domain Layer:**
- DOC-COMP-002: PatientAggregate - Patient admission lifecycle (business rules)
- DOC-COMP-005: PatientIdentity - Patient demographic information value object
- DOC-COMP-009: BedLocation - Bed/room location value object

**Architecture:**
- DOC-ARCH-002: System Architecture - Application Layer overview
- ADT Workflow Documentation - Use case specifications

## 12. Changelog

| Version | Date       | Author      | Changes                                       |
| ------- | ---------- | ----------- | --------------------------------------------- |
| v1.0    | 2025-01-26 | Dustin Wind | Initial documentation from AdmissionService.h |
