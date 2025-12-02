---
doc_id: DOC-ARCH-006
title: System Overview - Z Monitor
version: v1.0
category: Architecture
subcategory: System Design
status: Approved
owner: Architecture Team
reviewers: 
  - Architecture Team
  - Product Team
last_reviewed: 2025-12-08
next_review: 2026-03-08
related_docs:
  - DOC-ARCH-001 # System Architecture
  - DOC-ARCH-002 # System Architecture (duplicate check needed)
  - DOC-ARCH-004 # Technology Stack
  - DOC-ARCH-007 # UI/UX Guide
  - DOC-ARCH-008 # Alarm System
  - DOC-ARCH-010 # Security
  - DOC-GUIDE-001 # Code Organization
  - DOC-REF-001 # Glossary
related_tasks:
  - PHASE-6A # Architecture document migration
related_requirements:
  - REQ-SYS-001 # System requirements
  - REQ-HW-001 # Hardware requirements
tags:
  - architecture
  - system-overview
  - embedded
  - medical-device
  - patient-monitoring
  - real-time
source: 01_OVERVIEW.md (z-monitor/architecture_and_design)
---

# System Overview - Z Monitor

## Document Purpose

This document provides a comprehensive high-level overview of the Z Monitor project, describing its purpose, core features, architecture approach, technology stack, and key design principles. This is the foundational document for understanding the system's scope and capabilities.

---

## 1. Introduction

### 1.1 Project Summary

The **Z Monitor** is a modern, real-time patient monitoring system designed for embedded touch-screen medical devices. It serves as a sophisticated reference implementation demonstrating best practices in:

- Software architecture (Domain-Driven Design)
- UI/UX design for medical devices
- Security and data protection
- System-level integration using Qt/C++ and QML
- Real-time clinical data processing

### 1.2 Document Scope

This overview covers:
- Target hardware platform
- Core feature set
- Architecture approach
- Technology stack
- Key design principles
- Clinical use cases

For detailed requirements, see the requirements documentation in `z-monitor/requirements/`.

---

## 2. Target Hardware

### 2.1 Device Specification

| Specification   | Value                    |
| --------------- | ------------------------ |
| **Device Type** | Embedded Medical Monitor |
| **Screen Size** | 8-inch Touch Screen      |
| **Resolution**  | 1280x800 pixels (fixed)  |
| **Orientation** | Landscape                |
| **Input**       | Capacitive touch         |

### 2.2 Additional Requirements

Detailed hardware requirements (memory, storage, CPU, thermal, power) are specified in:
- `z-monitor/requirements/04_NON_FUNCTIONAL_REQUIREMENTS.md`

### 2.3 Deployment Target

- **Primary:** Embedded Linux ARM device
- **Development:** Desktop (Linux, macOS, Windows) for rapid prototyping
- **Containerized:** Docker environment for CI/CD and testing

---

## 3. Core Features

### 3.1 Patient Monitoring

#### Real-time Vitals Display

Continuous display of critical patient physiological data:

**Hemodynamics:**
- ECG waveform (real-time rendering)
- Heart Rate (BPM) with trend indicators
- ST-Segment analysis
- Premature Ventricular Contractions (PVCs) count

**Respiratory:**
- Plethysmograph waveform (SpO2 pulse wave)
- SpO2 (oxygen saturation) percentage
- Respiration Rate (breaths/min)

**Infusion:**
- TCI (Target Controlled Infusion) pump status
- Flow rate (mL/hr)
- Volume infused/remaining
- Drug name and concentration

#### Historical Trends

Dedicated view for analyzing patient data over time:
- **Time Windows:** 1 hour, 8 hours, 24 hours
- **Trend Plots:** Line charts for all vital signs
- **Clinical Analysis:** Identify patterns and changes
- **Export:** Capability to extract trend data (future)

#### Data Persistence

- Local encrypted storage (SQLCipher)
- Periodic synchronization with central server
- Offline operation capability

---

### 3.2 Alarms & Notifications

#### Prioritized Alarm System

Multi-level alarm system compliant with IEC 60601-1-8:

| Priority     | Use Case                                | Visual Indicator      | Audible Pattern         |
| ------------ | --------------------------------------- | --------------------- | ----------------------- |
| **Critical** | Life-threatening (e.g., Cardiac Arrest) | Full-screen red flash | Continuous high-pitched |
| **High**     | Severe condition (e.g., Low SpO2)       | Red banner + icon     | Rapid beep pattern      |
| **Medium**   | Warning (e.g., Heart Rate threshold)    | Yellow banner + icon  | Intermittent beep       |
| **Low**      | Advisory (e.g., Sensor disconnected)    | Blue indicator        | Single beep             |

#### Visual Alerts

- **Full-Screen Flash:** Critical alarms override all UI
- **Banner Notifications:** High/Medium alarms display at top of screen
- **Localized Indicators:** Parameter-specific alarm icons
- **Color Coding:** Red (critical), Yellow (warning), Blue (info)

#### Audible Alerts

- **Distinct Patterns:** Each priority level has unique beep sequence
- **Volume Control:** Adjustable by user (with minimum safety threshold)
- **Silence/Pause:** Temporary alarm muting (2-minute auto-reset)

#### Notification Center

- **Non-intrusive:** Bell icon in status bar
- **Informational Messages:** System events, warnings, updates
- **Persistent Log:** Historical notification access
- **Badge Count:** Unread notification indicator

---

### 3.3 System & Security

#### Central Server Communication

Secure telemetry transmission to central monitoring station:

**Connection States:**
- **Online:** Active connection, data streaming
- **Offline:** No connection, data buffered locally
- **Connecting:** Attempting to establish connection
- **Error:** Connection failed, retry in progress

**Features:**
- Real-time status indicator in UI
- Automatic reconnection on network recovery
- Buffered telemetry during offline periods
- TLS 1.3 encrypted communication (mTLS)

#### User Authentication

PIN-based login system with role-based access control:

**User Roles:**
- **Clinician:** Full access (patient data, settings, alarms)
- **Technician:** Limited access (settings, calibration, no patient data)

**Security Features:**
- 4-6 digit PIN
- Account lockout after failed attempts
- Session timeout (configurable)
- Audit logging of authentication events

#### Data Security

End-to-end security for sensitive medical data:

**Encryption:**
- **In Transit:** mTLS (mutual TLS) for all network communication
- **At Rest:** SQLCipher for local database encryption
- **Key Management:** Secure key storage and rotation

**Compliance:**
- HIPAA data protection requirements
- IEC 62304 medical device software lifecycle
- IEC 60601-1-8 alarm system requirements

---

### 3.4 Advanced Features

#### Predictive Analytics (Simulated)

Forward-looking risk assessment for clinical conditions:

**Risk Scores:**
- **Sepsis Risk:** Early sepsis detection based on vital trends
- **Arrhythmia Risk:** Cardiac rhythm abnormality prediction

**Intervention Suggestions:**
- Clinical decision support recommendations
- Alert thresholds and escalation paths
- Evidence-based clinical guidelines

**Implementation:**
- Simulated algorithms (placeholder for real ML models)
- Configurable risk thresholds
- Audit trail of predictions

#### Internationalization (i18n)

Full UI translation support for global deployment:

**Supported Languages:**
- English (primary)
- Spanish (planned)
- German (planned)
- Chinese (planned)

**Translation System:**
- Qt Linguist for translation management
- QML translation bindings
- Dynamic language switching (no restart required)
- Date/time/number formatting per locale

---

## 4. Architecture Approach

### 4.1 Domain-Driven Design (DDD)

The codebase is structured using Domain-Driven Design principles:

**Layered Architecture:**
1. **Domain Layer:** Core business logic, aggregates, value objects
2. **Application Layer:** Use cases, application services, orchestration
3. **Infrastructure Layer:** Data access, external integrations, persistence
4. **Interface Layer:** UI components, view models, QML integration

See **DOC-GUIDE-001** (Code Organization) for detailed layer rules.

### 4.2 Bounded Contexts

Independent bounded contexts with clear boundaries:

| Context           | Responsibility                         | Key Aggregates                       |
| ----------------- | -------------------------------------- | ------------------------------------ |
| **Monitoring**    | Real-time vitals, waveforms, alarms    | `PatientAggregate`, `DeviceSnapshot` |
| **Provisioning**  | Device setup, certificate management   | `DeviceIdentity`, `Certificate`      |
| **Admission/ADT** | Patient admission, discharge, transfer | `PatientIdentity`, `BedLocation`     |
| **Security**      | Authentication, authorization, audit   | `UserCredentials`, `AuditLog`        |

See **29_SYSTEM_COMPONENTS.md** for complete context mapping.

### 4.3 Immutable Records

Value objects and domain entities use immutable structs:

**Benefits:**
- Thread safety (safe concurrent access)
- Business rule enforcement (validated at construction)
- Simplified testing (no mutable state)

**Examples:**
- `PatientIdentity` (MRN, name, demographics)
- `DeviceSnapshot` (timestamp, all vital signs)
- `VitalRecord` (single vital sign measurement)

**Pattern:**
```cpp
struct PatientIdentity {
    const QString mrn;           // Medical Record Number
    const QString firstName;
    const QString lastName;
    const QDate dateOfBirth;
    
    // Validated constructor
    static Result<PatientIdentity, Error> create(
        const QString& mrn,
        const QString& firstName,
        const QString& lastName,
        const QDate& dob
    );
};
```

---

## 5. Technology Stack

### 5.1 Core Technologies

| Component         | Technology         | Version | Purpose                                        |
| ----------------- | ------------------ | ------- | ---------------------------------------------- |
| **UI Framework**  | Qt with QML        | Qt 6.x  | Front-end rendering and interaction            |
| **Backend Logic** | C++                | C++17   | Core services, business logic, data processing |
| **Build System**  | CMake              | 3.25+   | Cross-platform build configuration             |
| **Database**      | SQLite + SQLCipher | 3.x     | Encrypted local data persistence               |
| **Networking**    | Qt Network         | Qt 6.x  | HTTPS communication (QNetworkAccessManager)    |
| **Documentation** | Doxygen            | 1.9+    | API documentation from source comments         |

### 5.2 Development Tools

| Tool                       | Purpose                                  |
| -------------------------- | ---------------------------------------- |
| **Qt Creator**             | Primary IDE for Qt/QML development       |
| **CLion / VS Code**        | Alternative C++ development environments |
| **CMake**                  | Build system generator                   |
| **Git**                    | Version control                          |
| **Docker**                 | Containerized development and CI/CD      |
| **Python (Flask/FastAPI)** | Simulated central server                 |

### 5.3 Testing & Quality

| Tool                  | Purpose                        |
| --------------------- | ------------------------------ |
| **Qt Test**           | Unit testing framework         |
| **Google Test**       | C++ unit testing (alternative) |
| **Valgrind**          | Memory leak detection          |
| **Address Sanitizer** | Runtime error detection        |
| **Clang-Tidy**        | Static analysis                |

See **DOC-ARCH-004** (Technology Stack) for detailed version requirements and rationale.

---

## 6. Key Design Principles

### 6.1 Clinical Safety

**Priority:** Patient safety is the highest priority in all design decisions.

**Principles:**
- Fail-safe alarm system (always audible/visible)
- Redundant critical data storage
- Graceful degradation on errors
- Offline operation capability

### 6.2 Real-Time Performance

**Target:** Sub-100ms latency for critical data updates.

**Strategies:**
- Lock-free data structures for waveform rendering
- Pre-allocated memory pools (no dynamic allocation in hot paths)
- Dedicated real-time thread for sensor data processing
- Asynchronous logging (no blocking on I/O)

See **DOC-GUIDE-016** (Performance Benchmarking) for measurement criteria.

### 6.3 Security by Design

**Approach:** Defense-in-depth with multiple security layers.

**Layers:**
1. **Authentication:** PIN-based login with role-based access
2. **Encryption (Transit):** mTLS for all network communication
3. **Encryption (Rest):** SQLCipher for local database
4. **Audit Logging:** Comprehensive action logging for compliance
5. **Input Validation:** All external inputs validated and sanitized

See **DOC-ARCH-010** (Security) for detailed security architecture.

### 6.4 Testability

**Goal:** 80%+ code coverage with automated tests.

**Strategies:**
- Dependency injection for testable components
- Interface-based design (mocking via abstract interfaces)
- Separation of concerns (business logic independent of UI)
- Test data builders for complex domain objects

### 6.5 Maintainability

**Principles:**
- Clear layer boundaries (no cross-layer violations)
- Consistent naming conventions
- Comprehensive Doxygen comments on all public APIs
- No hardcoded values (use configuration or constants)

See **DOC-GUIDE-001** (Code Organization) for detailed guidelines.

---

## 7. Clinical Use Cases

### 7.1 Continuous Patient Monitoring

**Scenario:** ICU patient requires 24/7 vital sign monitoring.

**Workflow:**
1. Patient admitted to bed (ADT workflow)
2. Monitor configured with patient demographics
3. Sensors connected (ECG, SpO2, BP cuff)
4. Continuous data collection and display
5. Alarms trigger on threshold violations
6. Telemetry streamed to central station
7. Clinician reviews historical trends

### 7.2 Critical Alarm Response

**Scenario:** Patient experiences cardiac event.

**Workflow:**
1. Heart rate drops below critical threshold
2. **Critical alarm triggered** (full-screen red flash + audible)
3. Clinician acknowledges alarm
4. Clinician reviews ECG waveform and vitals
5. Clinician takes clinical action
6. Alarm silenced after resolution
7. Event logged for audit trail

### 7.3 Offline Operation

**Scenario:** Network connectivity lost to central server.

**Workflow:**
1. Monitor detects connection loss
2. **Status indicator:** Online → Offline
3. Monitor continues local operation
4. Telemetry buffered locally (encrypted)
5. Network connectivity restored
6. **Status indicator:** Offline → Connecting → Online
7. Buffered telemetry uploaded to server

### 7.4 Shift Handoff

**Scenario:** Nurse shift change requires patient review.

**Workflow:**
1. Outgoing nurse logs in
2. Reviews patient vitals and trends (1h, 8h, 24h)
3. Reviews alarm history and events
4. Hands off to incoming nurse
5. Incoming nurse logs in
6. Reviews same patient data
7. Audit log records both user sessions

---

## 8. System Boundaries

### 8.1 In Scope

- Real-time patient vital sign monitoring
- Alarm system (visual and audible)
- Central server communication (simulated)
- User authentication and authorization
- Local encrypted data persistence
- Historical trend analysis
- Predictive analytics (simulated)
- Internationalization support

### 8.2 Out of Scope

- Real sensor hardware integration (simulated via software)
- Real central server (simulated via Python Flask/FastAPI)
- EMR/EHR integration (placeholder interfaces only)
- Real predictive ML models (simulated algorithms)
- Medication administration tracking (future enhancement)
- Printer support (future enhancement)
- Barcode scanning (future enhancement)

### 8.3 Future Enhancements

- Integration with real HL7/FHIR servers
- Advanced analytics with real ML models
- Remote alarm notification (mobile app)
- Multi-monitor aggregation dashboard
- Voice commands (accessibility)

---

## 9. Related Documentation

### 9.1 Architecture Documents

- **DOC-ARCH-001:** System Architecture (detailed layer design)
- **DOC-ARCH-004:** Technology Stack (version requirements and rationale)
- **DOC-ARCH-007:** UI/UX Guide (interface design principles)
- **DOC-ARCH-008:** Alarm System (alarm architecture and compliance)
- **DOC-ARCH-010:** Security (security architecture and threat model)

### 9.2 Guidelines

- **DOC-GUIDE-001:** Code Organization (layer rules and file structure)
- **DOC-GUIDE-011:** Error Handling (Result<T,Error> pattern)
- **DOC-GUIDE-012:** Logging (three logging types: debug, audit, clinical)

### 9.3 Reference

- **DOC-REF-001:** Glossary (medical and technical terminology)
- **DOC-REF-002:** Error Codes (complete error catalog)
- **DOC-REF-003:** Alarm Codes (alarm priorities and thresholds)

### 9.4 Requirements

- `z-monitor/requirements/04_NON_FUNCTIONAL_REQUIREMENTS.md`
- `z-monitor/requirements/` (all requirements documentation)

---

## 10. Document Metadata

| Field                     | Value                                              |
| ------------------------- | -------------------------------------------------- |
| **Original Document ID**  | DESIGN-001                                         |
| **Original Version**      | 1.0                                                |
| **Original Status**       | Approved                                           |
| **Original Last Updated** | 2025-11-27                                         |
| **Migration Date**        | 2025-12-08                                         |
| **Migrated From**         | `z-monitor/architecture_and_design/01_OVERVIEW.md` |
| **New Document ID**       | DOC-ARCH-006                                       |
| **Category**              | Architecture                                       |
| **Subcategory**           | System Design                                      |

---

## Revision History

| Version | Date       | Author            | Changes                                                                 |
| ------- | ---------- | ----------------- | ----------------------------------------------------------------------- |
| v1.0    | 2025-12-08 | Architecture Team | Migrated from 01_OVERVIEW.md, expanded sections, added cross-references |

---

**End of Document**
