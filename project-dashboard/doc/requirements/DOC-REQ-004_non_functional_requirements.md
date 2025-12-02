---
doc_id: DOC-REQ-004
title: Non-Functional Requirements
category: REQ
status: approved
version: 1.0
last_updated: 2025-12-01
author: Z Monitor Team
related_docs:
  - DOC-REQ-003
  - DOC-REQ-002
  - DOC-ARCH-001
tags:
  - non-functional-requirements
  - performance
  - reliability
  - security
  - requirements
---

# Non-Functional Requirements

## 1. Overview

This document specifies quality attributes and constraints for the Z Monitor system. Non-functional requirements define **how well** the system must perform its functions.

**Related Documents:**
- **Functional Requirements:** [03_FUNCTIONAL_REQUIREMENTS.md](./03_FUNCTIONAL_REQUIREMENTS.md)
- **Use Cases:** [02_USE_CASES.md](./02_USE_CASES.md)
- **Architecture:** [../architecture_and_design/02_ARCHITECTURE.md](../architecture_and_design/02_ARCHITECTURE.md)

---

## 2. Requirement Categories

### 2.1 Performance (REQ-NFR-PERF-###)
Response time, throughput, latency, resource usage

### 2.2 Reliability (REQ-NFR-REL-###)
Uptime, fault tolerance, recovery, availability

### 2.3 Usability (REQ-NFR-USE-###)
Learnability, efficiency, errors, satisfaction

### 2.4 Maintainability (REQ-NFR-MAIN-###)
Modularity, testability, documentation, debuggability

### 2.5 Security (REQ-NFR-SEC-###)
Authentication, authorization, encryption, audit

### 2.6 Scalability (REQ-NFR-SCALE-###)
Concurrent users, data volume, network load

### 2.7 Portability (REQ-NFR-PORT-###)
Platform independence, deployment flexibility

### 2.8 Compatibility (REQ-NFR-COMPAT-###)
Interoperability, standards compliance

### 2.9 Hardware Resources (REQ-NFR-HW-###)
Memory, storage, CPU, device capabilities

---

## 3. Performance Requirements

### [REQ-NFR-PERF-001] UI Response Time - General

**Category:** Performance  
**Priority:** Must Have  
**Status:** Approved

**Description:**
The system shall respond to user interactions (button taps, screen navigation) within 500 milliseconds to maintain perceived responsiveness.

**Rationale:**
Users perceive delays > 500ms as sluggish. Medical environment requires quick interactions.

**Acceptance Criteria:**
- Button tap feedback: < 100ms (visual confirmation)
- Screen navigation: < 500ms (new view displayed)
- Form submission: < 1 second (processing + feedback)
- No UI freezing during background operations
- Loading indicators shown for operations > 500ms

**Measurement:**
- Automated UI tests measure tap-to-response latency
- 95th percentile must meet target
- Benchmarks run on target hardware

**Related Requirements:**
- REQ-NFR-PERF-100 (alarm response time)
- REQ-NFR-USE-001 (usability)

**Traces To:**
- Use Case: All interactive use cases
- Design: [12_THREAD_MODEL.md](../architecture_and_design/12_THREAD_MODEL.md) (UI thread never blocked)
- Test: Benchmark-PERF-001

**Notes:**
- Target hardware: Raspberry Pi 4 or equivalent (ARM Cortex-A72, 4GB RAM)

---

### [REQ-NFR-PERF-100] Alarm Detection Latency

**Category:** Performance  
**Priority:** Critical (Must Have)  
**Status:** Approved

**Description:**
The system shall detect vital sign threshold violations and trigger alarms within 50 milliseconds to ensure timely patient safety alerts.

**Rationale:**
**SAFETY CRITICAL.** IEC 60601-1-8 requires rapid alarm response. Delays could result in patient harm or death.

**Acceptance Criteria:**
- Vital sign read → threshold check → alarm trigger: < 50ms (end-to-end)
- Alarm thread runs at high priority (no starvation)
- No memory allocations in alarm detection path (pre-allocated)
- Alarm detection tested under maximum system load
- 99.99% of alarms trigger within 50ms (99th percentile)
- Zero missed alarms (100% detection rate)

**Measurement:**
- Timestamp vital sign generation
- Timestamp alarm trigger
- Calculate latency: trigger_time - vital_time
- Log all latencies to telemetry_metrics table
- Real-time monitoring of alarm latency

**Related Requirements:**
- REQ-FUN-ALARM-001 (alarm triggering)
- REQ-NFR-REL-100 (alarm reliability)
- REQ-REG-60601-001 (IEC compliance)

**Traces To:**
- Use Case: UC-AM-001, UC-ES-001
- Design: [12_THREAD_MODEL.md](../architecture_and_design/12_THREAD_MODEL.md) (Real-time thread, high priority)
- Design: [04_ALARM_SYSTEM.md](../architecture_and_design/04_ALARM_SYSTEM.md)
- Test: Test-PERF-100 (mandatory stress test)

**Notes:**
- **Most critical performance requirement in the system**
- Failure to meet this requirement is a safety hazard
- Regular performance testing mandatory
- Real hardware testing required (not just simulation)

---

### [REQ-NFR-PERF-101] Display Refresh Rate

**Category:** Performance  
**Priority:** Must Have  
**Status:** Approved

**Description:**
The system shall update vital signs display at minimum 1 Hz (once per second) and waveforms at 60 FPS for smooth rendering.

**Rationale:**
Vital signs must appear "live" to clinicians. Waveforms require smooth rendering for proper interpretation.

**Acceptance Criteria:**
- Numeric vital signs: Update 1 Hz minimum (every second)
- ECG waveform: Render at 60 FPS (smooth scrolling)
- Plethysmogram: Render at 60 FPS
- Trend sparklines: Update every 5 seconds
- No screen tearing or stuttering
- Frame drops < 1% under normal load

**Measurement:**
- Frame rate monitoring (QML profiler)
- Display update timestamps
- Benchmark on target hardware

**Related Requirements:**
- REQ-FUN-VITAL-001 (real-time display)
- REQ-FUN-VITAL-011 (waveforms)
- REQ-NFR-PERF-001 (UI responsiveness)

**Traces To:**
- Use Case: UC-VM-001
- Design: [12_THREAD_MODEL.md](../architecture_and_design/12_THREAD_MODEL.md) (Main/UI thread)
- Test: Benchmark-PERF-101

---

### [REQ-NFR-PERF-110] Database Query Performance

**Category:** Performance  
**Priority:** Must Have  
**Status:** Approved

**Description:**
The system shall execute database queries within performance targets to prevent UI blocking and ensure responsive trend display.

**Rationale:**
Slow queries block operations and degrade user experience. Time-series queries can be expensive if not optimized.

**Acceptance Criteria:**
- Simple queries (lookup by ID): < 10ms
- Trend queries (24 hours vitals): < 2 seconds
- Alarm history queries: < 1 second
- Patient search: < 500ms
- Write operations (single vital): < 5ms
- Batch writes (10 vitals): < 20ms
- Database maintains < 100MB size for 7 days data

**Measurement:**
- Query timing instrumentation
- Log slow queries (> 100ms)
- EXPLAIN QUERY PLAN analysis
- Regular performance testing

**Related Requirements:**
- REQ-FUN-VITAL-010 (trends)
- REQ-FUN-DATA-002 (data storage)
- REQ-NFR-PERF-111 (database optimization)

**Traces To:**
- Use Case: UC-VM-002 (trend queries)
- Design: [10_DATABASE_DESIGN.md](../architecture_and_design/10_DATABASE_DESIGN.md) (indices, WAL mode)
- Design: [30_DATABASE_ACCESS_STRATEGY.md](../architecture_and_design/30_DATABASE_ACCESS_STRATEGY.md)
- Test: Benchmark-PERF-110

**Notes:**
- WAL mode enabled for concurrent read/write
- Time-series indices critical for performance
- Prepared statement caching

---

### [REQ-NFR-PERF-111] Database Write Throughput

**Category:** Performance  
**Priority:** Must Have  
**Status:** Approved

**Description:**
The system shall sustain minimum 10 vital sign records per second write throughput without blocking the UI or real-time threads.

**Rationale:**
High-frequency monitoring generates substantial data. Database must keep up without impacting real-time performance.

**Acceptance Criteria:**
- Sustained write rate: 10 records/second minimum
- Peak write rate: 100 records/second (during catch-up sync)
- Database writes do not block UI thread
- Write queue depth < 100 records under normal load
- Transaction batching used (10 records per transaction)
- WAL mode enabled for concurrent access

**Measurement:**
- Write throughput monitoring
- Queue depth tracking
- Write latency histogram

**Related Requirements:**
- REQ-FUN-VITAL-002 (data recording)
- REQ-NFR-PERF-110 (query performance)

**Traces To:**
- Use Case: UC-VM-001 (data recording)
- Design: [30_DATABASE_ACCESS_STRATEGY.md](../architecture_and_design/30_DATABASE_ACCESS_STRATEGY.md) (Section 5)
- Design: [12_THREAD_MODEL.md](../architecture_and_design/12_THREAD_MODEL.md) (Database I/O thread)
- Test: Benchmark-PERF-111

---

### [REQ-NFR-PERF-200] Network Latency

**Category:** Performance  
**Priority:** Should Have  
**Status:** Approved

**Description:**
The system shall maintain network communication latency targets for data sync and patient lookup operations.

**Rationale:**
Low latency improves user experience and ensures timely data availability at central server.

**Acceptance Criteria:**
- Telemetry sync (device → server): < 500ms end-to-end
- Patient lookup (HIS query): < 5 seconds or timeout
- Alarm notification: < 1 second
- Network latency measured and logged (telemetry_metrics table)
- Latency classified: excellent (<100ms), good (<500ms), acceptable (<2s), slow (>2s)

**Measurement:**
- End-to-end timing: data_created_at → server_ack_at
- Network latency: transmitted_at → server_received_at
- Server processing latency: server_received_at → server_processed_at
- All timing tracked in telemetry_metrics table

**Related Requirements:**
- REQ-FUN-DATA-001 (sync)
- REQ-FUN-PAT-010 (patient lookup)
- REQ-NFR-PERF-201 (network throughput)

**Traces To:**
- Use Case: UC-DS-001, UC-PM-004
- Design: [10_DATABASE_DESIGN.md](../architecture_and_design/10_DATABASE_DESIGN.md) (telemetry_metrics table)
- Design: [09_CLASS_DESIGNS.md](../architecture_and_design/09_CLASS_DESIGNS.md) (NetworkManager)
- Test: Benchmark-PERF-200

**Notes:**
- Latency depends on network quality (WiFi vs Ethernet)
- Hospital network typically 10-50ms RTT

---

## 4. Hardware Resource Requirements

### [REQ-NFR-HW-001] Memory (RAM) Requirements

**Category:** Hardware Resources  
**Priority:** Must Have  
**Status:** Approved

**Description:**
The device shall have sufficient RAM to support real-time monitoring, in-memory caching, and application runtime without performance degradation.

**Rationale:**
Insufficient memory causes swapping, which degrades real-time performance and violates alarm latency requirements (< 50ms). Memory must accommodate critical path operations (alarm detection) and application runtime.

**Acceptance Criteria:**
- **Minimum RAM:** 2 GB
- **Recommended RAM:** 4 GB
- **Memory Allocation:**
  - In-memory cache: ~400 MB
    - 3-day vitals cache: ~390 MB (2.6M records × 150 bytes/record)
    - Waveform cache: ~10 MB (30-second circular buffer for display)
    - Purpose: Critical path for alarm detection (< 50ms latency requirement)
    - Thread: Real-Time Processing Thread (high priority)
  - Application runtime: ~500-800 MB
    - Qt/QML framework and rendering
    - Application services and controllers
    - UI components and graphics buffers
  - Diagnostics view buffer: ~5 MB
    - Last 1000 log entries (in-memory only, not persisted)
    - Used for Diagnostics View UI display
  - System overhead: ~200-300 MB
    - Operating system
    - Background services
    - Network buffers
- **Total Estimated RAM Usage:** ~1.1-1.5 GB (with 4 GB recommended for headroom)
- No memory allocations in critical path (pre-allocated buffers)
- Memory usage monitored and logged

**Measurement:**
- Memory profiling on target hardware
- Peak memory usage tracking
- Memory leak detection (Valgrind, AddressSanitizer)
- Real-time memory monitoring

**Related Requirements:**
- REQ-NFR-PERF-100 (alarm detection latency)
- REQ-FUN-ALARM-001 (alarm system)

**Traces To:**
- Design: [36_DATA_CACHING_STRATEGY.md](../architecture_and_design/36_DATA_CACHING_STRATEGY.md) (in-memory cache)
- Design: [12_THREAD_MODEL.md](../architecture_and_design/12_THREAD_MODEL.md) (memory allocation strategy)
- Test: Memory profiling and leak detection

---

### [REQ-NFR-HW-002] Storage (Disk/Flash) Requirements

**Category:** Hardware Resources  
**Priority:** Must Have  
**Status:** Approved

**Description:**
The device shall have sufficient storage capacity to store application data, logs, and system files while maintaining performance and data retention policies.

**Rationale:**
Storage must accommodate database (vitals, alarms, logs), application logs, and system files. Database size is limited to 500 MB to ensure performance and enable automatic cleanup. Insufficient storage causes data loss and system failures.

**Acceptance Criteria:**
- **Minimum Storage:** 2 GB
- **Recommended Storage:** 4 GB
- **Storage Allocation:**
  - **Database (SQLite with SQLCipher):**
    - Maximum size: 500 MB (hard limit, per CON-SW-003)
    - Warning threshold: 400 MB (80% of limit)
    - Critical threshold: 450 MB (90% of limit)
    - Automatic cleanup: Triggered at 500 MB limit
    - Contents:
      - Vitals data: 7-day retention (~200-300 MB typical)
      - Alarm history: 90-day retention (~50-100 MB typical)
      - Action logs: 90-day retention (~20-50 MB typical)
      - Security audit logs: 90-day retention (~20-50 MB typical)
      - Telemetry metrics: 90-day retention (~10-20 MB typical)
      - Patient data: Until discharge + 30 days (~5-10 MB typical)
      - Settings and configuration: ~1-2 MB
  - **Application Logs (File System):**
    - Location: `logs/z-monitor.log.*`
    - Retention: 7 days
    - Rotation: Daily at midnight, keep 7 files
    - Max file size: 10 MB per file
    - Total maximum: ~70 MB (7 files × 10 MB)
    - Format: Human-readable (development) or JSON (production)
  - **System Files:**
    - Application binary: ~50-100 MB
    - Qt libraries: ~200-300 MB
    - Certificates and keys: ~1-2 MB
    - Configuration files: ~1-2 MB
    - Temporary files: ~50-100 MB
- **Total Estimated Storage Usage:** ~600-800 MB (with 2 GB minimum recommended for headroom and future growth)
- Database size monitoring every 60 seconds
- Automatic cleanup when limits exceeded

**Storage Management:**
- **Automatic Cleanup:**
  - Daily cleanup: Runs at 3 AM, removes data older than retention periods
  - Emergency cleanup: Triggered when database exceeds 500 MB limit
    - Deletes vitals older than 3 days (instead of 7)
    - Archives resolved alarms older than 7 days
    - Compacts database (SQLite VACUUM)
- **Size Monitoring:**
  - Periodic check: Every 60 seconds
  - Cached size: File size cached for 60 seconds (low overhead)
  - Growth rate: Calculated daily (average of last 7 days)
  - Alerts:
    - Warning at 400 MB (80% of limit)
    - Critical at 450 MB (90% of limit)
    - Emergency cleanup at 500 MB (100% of limit)

**Data Retention Policies:**
- Vitals data: 7 days (configurable)
- Alarm history: 90 days (configurable)
- Action logs: 90 days minimum (configurable, required for compliance)
- Security audit logs: 90 days minimum (configurable, required for compliance)
- Telemetry metrics: 90 days (configurable)
- Application logs: 7 days (file rotation)
- Patient data: Until discharge + 30 days (configurable)

**Measurement:**
- Storage usage monitoring
- Database size tracking
- Cleanup effectiveness verification
- Growth rate analysis

**Related Requirements:**
- REQ-FUN-DATA-002 (data storage)
- REQ-NFR-PERF-111 (database write throughput)
- REQ-SEC-AUDIT-002 (audit log retention)

**Traces To:**
- Design: [10_DATABASE_DESIGN.md](../architecture_and_design/10_DATABASE_DESIGN.md) (database schema and retention)
- Design: [21_LOGGING_STRATEGY.md](../architecture_and_design/21_LOGGING_STRATEGY.md) (logging and storage)
- Design: [09_CLASS_DESIGNS.md](../architecture_and_design/09_CLASS_DESIGNS.md) (DatabaseManager size monitoring)
- Test: Storage capacity and cleanup tests

---

## 5. Reliability Requirements

### [REQ-NFR-REL-001] System Uptime

**Category:** Reliability  
**Priority:** Must Have  
**Status:** Approved

**Description:**
The system shall maintain 99.9% uptime (maximum 8.76 hours downtime per year) for patient monitoring functions.

**Rationale:**
Patient safety depends on continuous monitoring. Downtime could result in missed critical events.

**Acceptance Criteria:**
- Planned downtime < 1 hour per month (firmware updates, maintenance)
- Unplanned downtime < 8 hours per year
- System recovers automatically from crashes
- Uptime tracked and reported monthly
- Downtime incidents logged and reviewed

**Measurement:**
- Track system start/stop times
- Calculate uptime percentage
- Monthly uptime report

**Related Requirements:**
- REQ-NFR-REL-002 (fault tolerance)
- REQ-NFR-REL-010 (crash recovery)

**Traces To:**
- Use Case: All use cases (system availability)
- Design: [09_CLASS_DESIGNS.md](../architecture_and_design/09_CLASS_DESIGNS.md) (HealthMonitor)
- Test: Test-REL-001 (long-duration testing)

**Notes:**
- Target: 99.9% = 8h 45m downtime per year
- Industry standard for Class II medical devices

---

### [REQ-NFR-REL-002] Fault Tolerance

**Category:** Reliability  
**Priority:** Must Have  
**Status:** Approved

**Description:**
The system shall continue critical functions (alarm detection, vital signs display) even when non-critical subsystems fail.

**Rationale:**
Single component failure should not disable entire system. Graceful degradation maintains patient safety.

**Acceptance Criteria:**
- Network failure: Device continues local monitoring and alarming
- Database failure: Device continues displaying real-time vitals (in-memory)
- HIS unavailable: Device uses cached patient data
- Audio failure: Device escalates to visual-only alarms
- Sensor failure: Device triggers technical alarm but continues monitoring other sensors
- Each failure logged and reported

**Related Requirements:**
- REQ-NFR-REL-005 (graceful degradation)
- REQ-NFR-REL-010 (recovery)
- REQ-FUN-ALARM-001 (alarms always functional)

**Traces To:**
- Use Case: UC-ES-002 (network outage), UC-ES-004 (sensor failure)
- Design: [20_ERROR_HANDLING_STRATEGY.md](../architecture_and_design/20_ERROR_HANDLING_STRATEGY.md)
- Test: Test-REL-002 (fault injection testing)

---

### [REQ-NFR-REL-005] Graceful Degradation (Offline Operation)

**Category:** Reliability  
**Priority:** Must Have  
**Status:** Approved

**Description:**
The system shall operate in degraded mode when network connectivity is lost, maintaining core monitoring functions locally.

**Rationale:**
Network outages are inevitable. Device must continue monitoring patients safely without central server.

**Acceptance Criteria:**
- Device detects network loss within 30 seconds
- Device displays "OFFLINE MODE" indicator
- Core functions continue:
  - Vital signs monitoring and display
  - Alarm detection and notification
  - Local data recording
  - User authentication (local database)
- Degraded functions:
  - Patient lookup (uses cache)
  - Data sync (queued for later)
  - Central station communication
- Device automatically reconnects when network restored
- Queued data synced automatically after reconnection

**Related Requirements:**
- REQ-NFR-REL-002 (fault tolerance)
- REQ-FUN-DATA-002 (offline queuing)
- REQ-FUN-PAT-011 (patient cache)

**Traces To:**
- Use Case: UC-ES-002, UC-DS-004
- Design: [20_ERROR_HANDLING_STRATEGY.md](../architecture_and_design/20_ERROR_HANDLING_STRATEGY.md)
- Design: [09_CLASS_DESIGNS.md](../architecture_and_design/09_CLASS_DESIGNS.md) (NetworkManager)
- Test: Test-REL-005 (network disconnection test)

---

### [REQ-NFR-REL-010] Crash Recovery

**Category:** Reliability  
**Priority:** Must Have  
**Status:** Approved

**Description:**
The system shall automatically restart and recover state after unexpected crashes or power loss, with no manual intervention required.

**Rationale:**
Crashes are rare but possible (software bugs, hardware glitches). Automatic recovery minimizes downtime and data loss.

**Acceptance Criteria:**
- Application crashes detected by watchdog process
- Automatic restart within 10 seconds
- Patient admission state recovered from database
- User session restored (or prompt to re-login)
- Unsync data preserved and sync resumed
- Crash logged with stack trace for debugging
- Crash notification sent to administrator

**Related Requirements:**
- REQ-NFR-REL-001 (uptime)
- REQ-NFR-REL-002 (fault tolerance)
- REQ-FUN-DATA-002 (data preservation)

**Traces To:**
- Use Case: UC-ES-003 (power loss recovery)
- Design: [09_CLASS_DESIGNS.md](../architecture_and_design/09_CLASS_DESIGNS.md) (HealthMonitor)
- Test: Test-REL-010 (crash testing)

**Notes:**
- Watchdog monitors application health
- Database transactions ensure data consistency

---

### [REQ-NFR-REL-100] Alarm System Reliability

**Category:** Reliability  
**Priority:** Critical (Must Have)  
**Status:** Approved

**Description:**
The system shall achieve 99.99% alarm detection reliability with zero tolerance for missed critical alarms.

**Rationale:**
**SAFETY CRITICAL.** Missed alarms could result in patient death. Alarm system is the most critical safety function.

**Acceptance Criteria:**
- Alarm detection rate: 100% (zero missed alarms)
- False alarm rate: < 5% (per IEC 60601-1-8)
- Alarm system uptime: 99.99% (52 minutes downtime per year maximum)
- Alarm thread priority: Highest (never starved)
- Alarm functionality independent of other subsystems
- Alarm system self-test on startup
- Alarm failures trigger immediate escalation

**Measurement:**
- Track all threshold violations
- Track all alarms triggered
- Calculate detection rate
- Track false alarms (acknowledged without intervention)
- Monthly reliability report

**Related Requirements:**
- REQ-FUN-ALARM-001 (triggering)
- REQ-NFR-PERF-100 (alarm latency)
- REQ-REG-60601-001 (IEC compliance)

**Traces To:**
- Use Case: UC-AM-001, UC-ES-001
- Design: [04_ALARM_SYSTEM.md](../architecture_and_design/04_ALARM_SYSTEM.md)
- Design: [12_THREAD_MODEL.md](../architecture_and_design/12_THREAD_MODEL.md) (Real-time thread)
- Test: Test-REL-100 (extensive alarm testing)

**Notes:**
- **Zero tolerance for missed alarms**
- Extensive testing required (thousands of alarm scenarios)
- Independent verification recommended

---

## 6. Usability Requirements

### [REQ-NFR-USE-001] Learning Curve

**Category:** Usability  
**Priority:** Must Have  
**Status:** Approved

**Description:**
The system shall enable new users to become competent in core tasks (patient admission, alarm acknowledgment) within 2 hours of training.

**Rationale:**
High nursing turnover and time constraints require intuitive interface. Complex systems increase errors and training costs.

**Acceptance Criteria:**
- New users complete training in < 2 hours
- After training, users can:
  - Admit patient in < 60 seconds
  - Acknowledge alarm in < 10 seconds
  - Navigate all views without assistance
- User error rate < 5% after training
- User satisfaction rating > 4/5
- No critical errors (patient safety impact) from trained users

**Measurement:**
- Training time tracking
- Task completion time measurement
- Error rate tracking (first week)
- User satisfaction surveys

**Related Requirements:**
- REQ-NFR-USE-002 (efficiency)
- REQ-NFR-USE-010 (visibility)

**Traces To:**
- Use Case: All use cases
- Design: [03_UI_UX_GUIDE.md](../architecture_and_design/03_UI_UX_GUIDE.md)
- Test: Test-USE-001 (usability testing with new users)

**Notes:**
- Training materials and simulator mode support learning
- Onboarding process documented

---

### [REQ-NFR-USE-002] Task Efficiency

**Category:** Usability  
**Priority:** Must Have  
**Status:** Approved

**Description:**
The system shall minimize steps required for frequent tasks to improve efficiency in time-critical environments.

**Rationale:**
Nurses are busy and every second counts. Streamlined workflows reduce cognitive load and improve response time.

**Acceptance Criteria:**
- Patient admission: < 5 taps/clicks
- Alarm acknowledgment: 1-2 taps maximum
- View navigation: < 3 taps to any view
- Common tasks accessible within 2 taps from dashboard
- No unnecessary confirmation dialogs for routine actions
- Keyboard shortcuts available for power users

**Measurement:**
- Task analysis (count interactions)
- Time-motion studies
- User feedback

**Related Requirements:**
- REQ-NFR-USE-001 (learnability)
- REQ-NFR-PERF-001 (response time)

**Traces To:**
- Use Case: All interactive use cases
- Design: [03_UI_UX_GUIDE.md](../architecture_and_design/03_UI_UX_GUIDE.md)
- Test: Test-USE-002

---

### [REQ-NFR-USE-010] Visibility and Readability

**Category:** Usability  
**Priority:** Must Have  
**Status:** Approved

**Description:**
The system shall display critical information (vital signs, alarms) that is readable from 10 feet away to enable at-a-glance monitoring.

**Rationale:**
Nurses monitor multiple patients from nursing station. Must see status without approaching each device.

**Acceptance Criteria:**
- Vital signs: Readable from 10 feet (font size 48pt+)
- Patient name: Readable from 10 feet (font size 36pt+)
- Alarm indicators: Visible from 10 feet (large icons, colors)
- High contrast (readable in bright/dim lighting)
- Color-blind friendly (not color-only indicators)
- Dark mode available for night shifts

**Measurement:**
- Readability testing at 10-foot distance
- User feedback on visibility
- Accessibility testing (color blindness simulation)

**Related Requirements:**
- REQ-NFR-USE-001 (usability)
- REQ-FUN-VITAL-001 (display)

**Traces To:**
- Use Case: UC-VM-001, UC-AM-001
- Design: [03_UI_UX_GUIDE.md](../architecture_and_design/03_UI_UX_GUIDE.md) (font sizes, colors)
- Test: Test-USE-010

---

### [REQ-NFR-USE-020] Error Prevention and Recovery

**Category:** Usability  
**Priority:** Must Have  
**Status:** Approved

**Description:**
The system shall prevent user errors through clear UI design and enable easy recovery when errors occur.

**Rationale:**
Medical errors can have serious consequences. System must guide users toward correct actions and make errors difficult.

**Acceptance Criteria:**
- Confirmation required for critical actions (discharge, alarm silence)
- Input validation with helpful error messages
- Undo available for non-critical actions
- Clear visual feedback for all actions
- Disabled buttons grayed out (not hidden)
- Error messages actionable ("Invalid MRN format. Expected: MRN-XXXXX")
- Help text available inline (no separate manual needed)

**Measurement:**
- Error rate tracking
- User error analysis
- Support ticket analysis

**Related Requirements:**
- REQ-NFR-USE-001 (learnability)
- REQ-FUN-USER-004 (permissions prevent unauthorized actions)

**Traces To:**
- Use Case: All use cases (exception flows)
- Design: [03_UI_UX_GUIDE.md](../architecture_and_design/03_UI_UX_GUIDE.md)
- Design: [20_ERROR_HANDLING_STRATEGY.md](../architecture_and_design/20_ERROR_HANDLING_STRATEGY.md)
- Test: Test-USE-020 (error scenario testing)

---

## 7. Maintainability Requirements

### [REQ-NFR-MAIN-001] Code Modularity

**Category:** Maintainability  
**Priority:** Must Have  
**Status:** Approved

**Description:**
The system shall be structured using Domain-Driven Design (DDD) with clear separation of concerns across layers (domain, application, infrastructure, interface).

**Rationale:**
Modular code is easier to understand, test, and modify. Supports long-term maintainability and team collaboration.

**Acceptance Criteria:**
- Code organized into DDD layers (domain, application, infrastructure, interface)
- Clear module boundaries with defined interfaces
- Dependencies flow inward (infrastructure → application → domain)
- No circular dependencies
- Each module has single responsibility
- Interface-based dependencies (not concrete classes)

**Measurement:**
- Static analysis (dependency graphs)
- Code review checklists
- Architectural conformance checks

**Related Requirements:**
- REQ-NFR-MAIN-002 (testability)
- REQ-NFR-MAIN-010 (documentation)

**Traces To:**
- Design: [29_SYSTEM_COMPONENTS.md](../architecture_and_design/29_SYSTEM_COMPONENTS.md)
- Design: [22_CODE_ORGANIZATION.md](../architecture_and_design/22_CODE_ORGANIZATION.md)
- Design: [13_DEPENDENCY_INJECTION.md](../architecture_and_design/13_DEPENDENCY_INJECTION.md)
- Test: Test-MAIN-001 (architectural tests)

---

### [REQ-NFR-MAIN-002] Testability

**Category:** Maintainability  
**Priority:** Must Have  
**Status:** Approved

**Description:**
The system shall be designed for testability with minimum 80% code coverage for critical components (alarm system, authentication, data sync).

**Rationale:**
Testing critical for medical device safety and regulatory compliance (IEC 62304). High coverage reduces defects.

**Acceptance Criteria:**
- Critical components: 90%+ code coverage
- Normal components: 80%+ code coverage
- Unit tests for all business logic
- Integration tests for component interactions
- End-to-end tests for critical workflows
- Tests run automatically in CI/CD
- Coverage reports generated and reviewed

**Measurement:**
- lcov/llvm-cov for coverage measurement
- Coverage tracked per component
- Coverage trends monitored (no decrease)

**Related Requirements:**
- REQ-NFR-MAIN-001 (modularity enables testing)
- REQ-REG-62304-010 (testing requirements)

**Traces To:**
- Design: [18_TESTING_WORKFLOW.md](../architecture_and_design/18_TESTING_WORKFLOW.md)
- Design: [13_DEPENDENCY_INJECTION.md](../architecture_and_design/13_DEPENDENCY_INJECTION.md) (mock objects)
- Test: All test suites

**Notes:**
- Critical: AlarmManager, AuthenticationService, NetworkManager, DatabaseManager
- Mock objects enable isolated unit testing

---

### [REQ-NFR-MAIN-010] API Documentation

**Category:** Maintainability  
**Priority:** Must Have  
**Status:** Approved

**Description:**
The system shall include comprehensive API documentation generated from code using Doxygen for all public classes and methods.

**Rationale:**
Documentation crucial for team collaboration, onboarding, and long-term maintenance. Generated docs stay synchronized with code.

**Acceptance Criteria:**
- All public classes have Doxygen class comments
- All public methods have Doxygen method comments
- Parameters, return values, exceptions documented
- Code examples provided for complex APIs
- API docs generated automatically in CI/CD
- Documentation coverage > 95% of public APIs
- HTML documentation published and accessible

**Measurement:**
- Doxygen coverage reports
- Documentation completeness checks
- Pre-commit hooks enforce documentation

**Related Requirements:**
- REQ-NFR-MAIN-001 (modularity)
- REQ-NFR-MAIN-002 (testability)

**Traces To:**
- Design: [26_API_DOCUMENTATION.md](../architecture_and_design/26_API_DOCUMENTATION.md)
- Design: [22_CODE_ORGANIZATION.md](../architecture_and_design/22_CODE_ORGANIZATION.md)
- Test: Test-MAIN-010 (documentation coverage check)

---

## 8. Security Requirements

### [REQ-NFR-SEC-001] Authentication Security

**Category:** Security  
**Priority:** Critical (Must Have)  
**Status:** Approved

**Description:**
The system shall implement secure authentication with cryptographic password hashing and brute force protection.

**Rationale:**
HIPAA requires authentication to protect PHI. Weak authentication enables unauthorized access to patient data.

**Acceptance Criteria:**
- PIN hashed using bcrypt or Argon2 (not plaintext or weak hash)
- Salt per user (prevents rainbow table attacks)
- Hashing parameters: bcrypt cost 12+ or Argon2 recommended params
- 3 failed attempts lock account for 10 minutes
- Account unlock requires administrator or timeout
- All authentication attempts logged (success/failure)

**Measurement:**
- Security audit of authentication code
- Penetration testing
- Password hash strength verification

**Related Requirements:**
- REQ-FUN-USER-001 (authentication)
- REQ-FUN-USER-005 (brute force protection)
- REQ-SEC-AUTH-001 (authentication requirements)

**Traces To:**
- Use Case: UC-UA-001
- Design: [06_SECURITY.md](../architecture_and_design/06_SECURITY.md) (Section 3)
- Design: [09_CLASS_DESIGNS.md](../architecture_and_design/09_CLASS_DESIGNS.md) (AuthenticationService)
- Test: Test-SEC-001 (security testing)

---

### [REQ-NFR-SEC-002] Encryption in Transit

**Category:** Security  
**Priority:** Critical (Must Have)  
**Status:** Approved

**Description:**
The system shall encrypt all network communication using TLS 1.2+ with mutual TLS (mTLS) for device-server authentication.

**Rationale:**
HIPAA requires encryption of PHI in transit. mTLS provides mutual authentication preventing man-in-the-middle attacks.

**Acceptance Criteria:**
- All HTTP communication uses HTTPS (TLS 1.2 or 1.3)
- Device authenticates to server using client certificate (mTLS)
- Server authenticates to device using server certificate
- Certificate validation enforced (not bypassed)
- Strong cipher suites only (AES-256-GCM, ChaCha20-Poly1305)
- Certificate pinning implemented (prevents CA compromise)
- Failed TLS handshakes logged

**Measurement:**
- Network traffic analysis (Wireshark)
- Security audit
- Penetration testing (man-in-the-middle attempts)

**Related Requirements:**
- REQ-FUN-DEV-002 (certificate management)
- REQ-SEC-ENC-001 (encryption requirements)
- REQ-REG-HIPAA-001 (encryption mandate)

**Traces To:**
- Use Case: UC-DS-001 (data sync)
- Design: [06_SECURITY.md](../architecture_and_design/06_SECURITY.md) (Section 6)
- Design: [15_CERTIFICATE_PROVISIONING.md](../architecture_and_design/15_CERTIFICATE_PROVISIONING.md)
- Test: Test-SEC-002 (TLS testing)

---

### [REQ-NFR-SEC-003] Encryption at Rest

**Category:** Security  
**Priority:** Critical (Must Have)  
**Status:** Approved

**Description:**
The system shall encrypt all patient data stored locally using AES-256 encryption (SQLCipher) to prevent unauthorized access to sensitive health information.

**Rationale:**
HIPAA requires encryption of PHI at rest. Device theft or unauthorized access must not expose patient data.

**Acceptance Criteria:**
- Database encrypted with SQLCipher (AES-256-CBC)
- Encryption key stored securely (HSM or protected memory)
- Key not hardcoded or stored in plaintext
- Database file unreadable without key
- Key rotation supported
- Encryption transparent to application (no code changes)

**Measurement:**
- Verify database file encrypted (hexdump analysis)
- Penetration testing (access without key)
- Key storage audit

**Related Requirements:**
- REQ-FUN-DATA-002 (data storage)
- REQ-SEC-ENC-002 (key management)
- REQ-REG-HIPAA-002 (encryption mandate)

**Traces To:**
- Design: [06_SECURITY.md](../architecture_and_design/06_SECURITY.md) (Section 2)
- Design: [10_DATABASE_DESIGN.md](../architecture_and_design/10_DATABASE_DESIGN.md) (SQLCipher)
- Test: Test-SEC-003

---

### [REQ-NFR-SEC-010] Audit Logging

**Category:** Security  
**Priority:** Must Have  
**Status:** Approved

**Description:**
The system shall log all security-relevant events (authentication, authorization, data access, configuration changes) for compliance and incident investigation.

**Rationale:**
HIPAA requires audit trails. Critical for compliance, security investigations, and user accountability.

**Acceptance Criteria:**
- Audit log includes: timestamp, event type, user ID, action, outcome, IP address
- Events logged:
  - Login/logout (success/failure)
  - Patient admission/discharge
  - Alarm acknowledgment
  - Settings changes
  - Data exports
  - Certificate operations
  - Permission denials
- Logs encrypted and tamper-evident
- Logs retained for 90 days minimum
- Logs synced to central server for centralized audit

**Measurement:**
- Audit log completeness review
- Compliance audit
- Penetration testing (verify all events logged)

**Related Requirements:**
- REQ-FUN-USER-001 (authentication logging)
- REQ-SEC-AUDIT-001 (audit requirements)
- REQ-REG-HIPAA-003 (audit trail mandate)

**Traces To:**
- Use Case: All use cases (all actions logged)
- Design: [06_SECURITY.md](../architecture_and_design/06_SECURITY.md) (Section 6.7)
- Design: [21_LOGGING_STRATEGY.md](../architecture_and_design/21_LOGGING_STRATEGY.md)
- Design: [10_DATABASE_DESIGN.md](../architecture_and_design/10_DATABASE_DESIGN.md) (security_audit_log table)
- Test: Test-SEC-010

---

## 9. Scalability Requirements

### [REQ-NFR-SCALE-001] Concurrent Patient Monitoring

**Category:** Scalability  
**Priority:** Must Have  
**Status:** Approved

**Description:**
The system shall support monitoring one patient per device with capability for central server to aggregate data from 100+ devices simultaneously.

**Rationale:**
Hospital ICU may have 20-100 beds, each with a monitoring device. Central server must handle all devices concurrently.

**Acceptance Criteria:**
- Single device: 1 patient at a time
- Central server: Support 100+ concurrent device connections
- Server: Handle 1,000 vital signs records per second (100 devices × 10 records/sec)
- No degradation in alarm latency with maximum device count
- Network bandwidth: < 10 kbps per device average
- Database: Support 7 days × 100 patients without performance degradation

**Measurement:**
- Load testing with simulated devices
- Central server performance monitoring
- Network bandwidth measurement

**Related Requirements:**
- REQ-NFR-PERF-200 (network latency)
- REQ-NFR-REL-001 (uptime at scale)

**Traces To:**
- Design: Central server architecture (to be documented)
- Design: [10_DATABASE_DESIGN.md](../architecture_and_design/10_DATABASE_DESIGN.md) (scalability)
- Test: Test-SCALE-001 (load testing)

**Notes:**
- Single device simpler than multi-patient
- Central server scalability separate design effort

---

## 10. Portability Requirements

### [REQ-NFR-PORT-001] Platform Independence

**Category:** Portability  
**Priority:** Should Have  
**Status:** Approved

**Description:**
The system shall run on Linux-based platforms (Ubuntu 20.04+, Raspberry Pi OS) with minimal platform-specific code.

**Rationale:**
Cross-platform support enables deployment flexibility and reduces vendor lock-in.

**Acceptance Criteria:**
- Code builds on Ubuntu 20.04, 22.04, Debian 11+
- Code builds on Raspberry Pi OS (ARM)
- Platform-specific code isolated in abstraction layers
- Qt provides cross-platform API
- No hardcoded paths (use QStandardPaths)
- Automated testing on all target platforms

**Measurement:**
- CI/CD builds on multiple platforms
- Automated testing on Ubuntu and RPi

**Related Requirements:**
- REQ-NFR-MAIN-001 (modularity)

**Traces To:**
- Design: [22_CODE_ORGANIZATION.md](../architecture_and_design/22_CODE_ORGANIZATION.md)
- Design: [07_SETUP_GUIDE.md](../architecture_and_design/07_SETUP_GUIDE.md)
- Test: Test-PORT-001

**Notes:**
- Primary target: Linux ARM (Raspberry Pi or similar)
- x86_64 support for development/testing

---

## 11. Compatibility Requirements

### [REQ-NFR-COMPAT-001] Standards Compliance

**Category:** Compatibility  
**Priority:** Must Have  
**Status:** Approved

**Description:**
The system shall comply with relevant medical device software standards (IEC 62304, IEC 60601-1-8) and healthcare data standards (HL7 FHIR if applicable).

**Rationale:**
Regulatory approval requires standards compliance. Interoperability with hospital systems requires data standards.

**Acceptance Criteria:**
- Software development follows IEC 62304 lifecycle
- Alarm system meets IEC 60601-1-8 requirements
- Security architecture aligns with IEC 62443
- Patient data format compatible with HL7 FHIR (if HIS integration required)
- All standards documented in design docs
- Compliance verified through testing and certification

**Measurement:**
- Regulatory audit
- Compliance checklist
- Third-party verification

**Related Requirements:**
- REQ-REG-62304-001 (IEC 62304)
- REQ-REG-60601-001 (IEC 60601-1-8)
- REQ-REG-62443-001 (IEC 62443)

**Traces To:**
- Design: [07_REGULATORY_REQUIREMENTS.md](./07_REGULATORY_REQUIREMENTS.md)
- Design: Multiple architecture documents
- Test: Compliance testing

---

## 12. Non-Functional Requirements Summary

### Total Requirements: 26 (of ~80-100 planned)

| Category           | Requirements | Critical | Must Have | Should Have |
| ------------------ | ------------ | -------- | --------- | ----------- |
| Performance        | 6            | 1        | 4         | 1           |
| Hardware Resources | 2            | 0        | 2         | 0           |
| Reliability        | 4            | 1        | 3         | 0           |
| Usability          | 4            | 0        | 4         | 0           |
| Maintainability    | 3            | 0        | 3         | 0           |
| Security           | 4            | 3        | 1         | 0           |
| Scalability        | 1            | 0        | 1         | 0           |
| Portability        | 1            | 0        | 0         | 1           |
| Compatibility      | 1            | 0        | 1         | 0           |
| **Total**          | **26**       | **5**    | **19**    | **2**       |

### Remaining Requirements (to be added):
- ~10 additional performance requirements (CPU, battery)
- ~10 reliability requirements (MTBF, MTTR, disaster recovery)
- ~10 usability requirements (accessibility, i18n)
- ~10 maintainability requirements (debugging, monitoring)
- ~15 security requirements (detailed crypto, key management)
- ~10 scalability requirements (data volume, concurrent operations)
- ~10 compatibility requirements (hardware, software versions)

---

## 13. Quality Attribute Scenarios

### Example: Performance (Alarm Detection)
- **Source:** Vital sign generator (DeviceSimulator)
- **Stimulus:** Heart rate exceeds threshold (125 bpm > 120 bpm)
- **Environment:** Normal load (1 patient, 10 vitals/sec)
- **Artifact:** AlarmManager
- **Response:** Alarm triggered and displayed
- **Measure:** Latency < 50ms (95th percentile)

### Example: Reliability (Network Failure)
- **Source:** Network infrastructure
- **Stimulus:** Network connection lost
- **Environment:** Active patient monitoring, unsync data queued
- **Artifact:** NetworkManager
- **Response:** Device continues local monitoring, queues data
- **Measure:** Zero data loss, automatic recovery when network restored

### Example: Usability (Patient Admission)
- **Source:** Clinical nurse
- **Stimulus:** Admits new patient
- **Environment:** Start of shift, unfamiliar with system
- **Artifact:** Admission UI
- **Response:** Patient admitted successfully
- **Measure:** Task completion < 60 seconds, < 5 taps, zero errors

---

## 14. Related Documents

- **Functional Requirements:** [03_FUNCTIONAL_REQUIREMENTS.md](./03_FUNCTIONAL_REQUIREMENTS.md)
- **Security Requirements:** [08_SECURITY_REQUIREMENTS.md](./08_SECURITY_REQUIREMENTS.md)
- **Regulatory Requirements:** [07_REGULATORY_REQUIREMENTS.md](./07_REGULATORY_REQUIREMENTS.md)
- **Thread Model:** [../architecture_and_design/12_THREAD_MODEL.md](../architecture_and_design/12_THREAD_MODEL.md)
- **Testing Workflow:** [../architecture_and_design/18_TESTING_WORKFLOW.md](../architecture_and_design/18_TESTING_WORKFLOW.md)

---

*Non-functional requirements define the quality attributes that make the system safe, reliable, and effective. They are as critical as functional requirements for medical device success.*

