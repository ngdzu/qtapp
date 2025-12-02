---
doc_id: DOC-COMP-014
title: IPatientRepository
version: v1.0
category: Component
subcategory: Application Layer / Repository Interface
status: Draft
owner: Application Layer Team
reviewers: 
  - Architecture Team
  - Domain Layer Team
last_reviewed: 2025-01-26
next_review: 2026-01-26
related_docs:
  - DOC-ARCH-002  # System architecture
  - DOC-COMP-001  # PatientAggregate
related_tasks:
  - TASK-3B-001  # Phase 3B Migration
tags:
  - repository
  - persistence
  - patient
  - ddd
diagram_files: []
---

# DOC-COMP-014: IPatientRepository

## 1. Overview

**Purpose:** Repository interface for persisting and retrieving PatientAggregate domain entities, following Domain-Driven Design (DDD) repository pattern to abstract persistence layer from application logic.

**Responsibilities:**
- Save patient aggregate to persistence store (SQLite database)
- Find patient by Medical Record Number (MRN)
- Retrieve patient admission history (all admissions across time)
- List all patients
- Remove patient aggregate from persistence store

**Layer:** Application Layer (Repository Interface)

**Module:** `z-monitor/src/domain/repositories/IPatientRepository.h`

**Implementation:** SQLitePatientRepository (infrastructure layer)

**Thread Affinity:** Database I/O Thread (single writer thread for SQLite thread safety)

## 2. Public API

```cpp
class IPatientRepository {
public:
    virtual ~IPatientRepository() = default;

    /**
     * @brief Find patient by Medical Record Number (MRN).
     * @param mrn Medical Record Number (unique patient identifier)
     * @return Result<std::shared_ptr<PatientAggregate>> Patient aggregate or error
     */
    virtual Result<std::shared_ptr<PatientAggregate>, Error> findByMrn(const QString& mrn) const = 0;

    /**
     * @brief Save patient aggregate.
     * @param patient Patient aggregate to save (insert or update)
     * @return Result<void, Error> Success or error
     */
    virtual Result<void, Error> save(std::shared_ptr<PatientAggregate> patient) = 0;

    /**
     * @brief Get admission history for all patients.
     * @return Result<QList<AdmissionRecord>> List of admission records or error
     */
    virtual Result<QList<AdmissionRecord>, Error> getAdmissionHistory() const = 0;

    /**
     * @brief Find all patients.
     * @return Result<QList<std::shared_ptr<PatientAggregate>>> List of all patients or error
     */
    virtual Result<QList<std::shared_ptr<PatientAggregate>>, Error> findAll() const = 0;

    /**
     * @brief Remove patient from repository.
     * @param mrn Medical Record Number of patient to remove
     * @return Result<void, Error> Success or error
     */
    virtual Result<void, Error> remove(const QString& mrn) = 0;
};
```

## 3. Implementation Details

**SQLitePatientRepository Implementation:**
- Database table: `patients` (columns: mrn, patient_name, date_of_birth, gender, weight_kg, blood_type, created_at, updated_at)
- Insert/Update: Uses SQLite UPSERT (`INSERT OR REPLACE`) for atomic save
- Query: MRN is primary key, indexed for O(1) lookup
- Thread Safety: All database operations executed on single Database I/O Thread (Qt::QueuedConnection)

**Error Handling:**
- Uses `Result<T, Error>` type for all operations (no exceptions)
- Database connection errors: Returns `Error::DATABASE_CONNECTION_FAILED`
- MRN not found: Returns `Error::PATIENT_NOT_FOUND`
- SQLite errors: Returns `Error::DATABASE_QUERY_FAILED` with error message

## 4. Usage Examples

### 4.1 Find Patient by MRN

```cpp
auto result = patientRepo->findByMrn("MRN-12345");
if (result.isSuccess()) {
    auto patient = result.value();
    qInfo() << "Found patient:" << patient->name();
} else {
    qWarning() << "Patient not found:" << result.error().message();
}
```

### 4.2 Save Patient

```cpp
auto patient = std::make_shared<PatientAggregate>(
    PatientIdentity("MRN-12345", "John Doe", birthDate, Gender::Male));
patient->setWeight(70.5);
patient->setBloodType(BloodType::A_Positive);

auto result = patientRepo->save(patient);
if (result.isSuccess()) {
    qInfo() << "Patient saved successfully";
} else {
    qWarning() << "Save failed:" << result.error().message();
}
```

## 5. Testing

**Unit Tests (Mock):**
- `test_FindByMrn_ExistingPatient_ReturnsPatient()` - Verify findByMrn returns patient
- `test_FindByMrn_NonExistentMrn_ReturnsError()` - Verify error handling
- `test_Save_NewPatient_InsertsPatient()` - Verify insert operation
- `test_Save_ExistingPatient_UpdatesPatient()` - Verify update operation

**Integration Tests (SQLite):**
- End-to-end save/retrieve cycle verification
- Admission history query verification
- Database constraint enforcement (unique MRN)

## 6. Related Documentation

- DOC-COMP-001: PatientAggregate - Patient domain entity
- SQLitePatientRepository Implementation - Infrastructure layer persistence

## 7. Changelog

| Version | Date       | Author      | Changes                                         |
| ------- | ---------- | ----------- | ----------------------------------------------- |
| v1.0    | 2025-01-26 | Dustin Wind | Initial documentation from IPatientRepository.h |
