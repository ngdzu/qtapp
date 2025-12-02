---
doc_id: DOC-COMP-019
title: SQLitePatientRepository
version: v1.0
category: Component
subcategory: Infrastructure Layer / Persistence
status: Draft
owner: Infrastructure Team
reviewers: [Architecture Team, Domain Layer Team]
last_reviewed: 2025-12-01
next_review: 2026-12-01
related_docs: [DOC-ARCH-002, DOC-COMP-001, DOC-COMP-014, DOC-COMP-018]
related_tasks: [TASK-3C-001]
tags: [infrastructure, repository, sqlite, orm, patient, persistence]
diagram_files: []
---

# DOC-COMP-019: SQLitePatientRepository

## 1. Overview

**Purpose:** SQLite implementation of IPatientRepository interface using hybrid ORM + manual SQL approach (QxOrm for simple CRUD, manual SQL for complex queries).

**Responsibilities:**
- Persist PatientAggregate to `patients` table
- Find patient by MRN (Medical Record Number)
- Query admission history
- List all patients
- Delete patient by MRN
- Convert between PatientEntity (ORM/persistence) and PatientAggregate (domain)

**Layer:** Infrastructure Layer (Repository Implementation)

**Module:** `z-monitor/src/infrastructure/persistence/SQLitePatientRepository.h` (256 lines)

**Thread Affinity:** Database I/O Thread

**Dependencies:**
- **DatabaseManager:** Connection management, transactions
- **IPatientRepository:** Repository interface contract
- **PatientAggregate:** Domain aggregate
- **PatientEntity (Optional):** QxOrm entity (when USE_QXORM enabled)

## 2. Public API

```cpp
class SQLitePatientRepository : public QObject, public IPatientRepository {
    Q_OBJECT

public:
    explicit SQLitePatientRepository(DatabaseManager *dbManager, QObject *parent = nullptr);
    ~SQLitePatientRepository() override;

    Result<std::shared_ptr<PatientAggregate>> findByMrn(const std::string &mrn) override;
    Result<void> save(const PatientAggregate &patient) override;
    Result<std::vector<std::string>> getAdmissionHistory(const std::string &mrn) override;
    Result<std::vector<std::shared_ptr<PatientAggregate>>> findAll() override;
    Result<void> remove(const std::string &mrn) override;

private:
    DatabaseManager *m_dbManager;
    
    Result<std::shared_ptr<PatientAggregate>> entityToAggregate(const PatientEntity &entity);
    Result<std::shared_ptr<PatientAggregate>> queryToAggregate(const QSqlQuery &query);
    PatientEntity aggregateToEntity(const PatientAggregate &aggregate);
};
```

## 3. Implementation Strategy

**Hybrid Approach:**
- **ORM (QxOrm):** findByMrn, save, remove (simple CRUD operations)
- **Manual SQL:** findAll, getAdmissionHistory (complex queries with joins)

**Conversion:**
- `entityToAggregate()`: PatientEntity → PatientAggregate (ORM to domain)
- `aggregateToEntity()`: PatientAggregate → PatientEntity (domain to ORM)
- `queryToAggregate()`: QSqlQuery → PatientAggregate (manual SQL to domain)

## 4. Usage Example

```cpp
auto repo = new SQLitePatientRepository(dbManager);

// Find patient
auto result = repo->findByMrn("MRN-12345");
if (result.isSuccess()) {
    auto patient = result.value();
    qInfo() << "Found patient:" << patient->name();
}

// Save patient
PatientAggregate patient(...);
auto saveResult = repo->save(patient);
```

## 5. Related Documentation

- DOC-COMP-001: PatientAggregate
- DOC-COMP-014: IPatientRepository
- DOC-COMP-018: DatabaseManager

## 6. Changelog

| Version | Date       | Author      | Changes                                              |
| ------- | ---------- | ----------- | ---------------------------------------------------- |
| v1.0    | 2025-12-01 | Dustin Wind | Initial documentation from SQLitePatientRepository.h |
