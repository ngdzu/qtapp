# Constraints and Assumptions

**Document ID:** REQ-DOC-09  
**Version:** 0.1  
**Status:** In Progress  
**Last Updated:** 2025-11-27

---

## 1. Overview

This document specifies constraints, limitations, and assumptions for the Z Monitor system. Constraints define boundaries within which the system must operate. Assumptions are conditions believed to be true but not yet verified.

**Related Documents:**
- **Architecture:** [../architecture_and_design/02_ARCHITECTURE.md](../architecture_and_design/02_ARCHITECTURE.md)
- **Requirements:** All requirements documents
- **Setup Guide:** [../architecture_and_design/07_SETUP_GUIDE.md](../architecture_and_design/07_SETUP_GUIDE.md)

---

## 2. Technical Constraints

### 2.1 Hardware Constraints

#### [CON-HW-001] Target Hardware Platform

**Constraint:** The system shall run on Raspberry Pi 4 (or equivalent ARM/x86 Linux platform) with minimum 2GB RAM and quad-core processor.

**Rationale:** Cost-effective hardware suitable for medical device deployment. Raspberry Pi widely available and well-supported.

**Impact:**
- Performance: Limited compared to desktop hardware
- Graphics: Hardware-accelerated graphics available (VideoCore VI)
- Storage: SD card or USB storage (limited I/O performance)
- Network: Gigabit Ethernet, 802.11ac Wi-Fi

**Minimum Hardware Specifications:**
- **CPU:** ARM Cortex-A72 quad-core @ 1.5 GHz (or x86_64 equivalent)
- **RAM:** 2 GB minimum, 4 GB recommended
- **Storage:** 16 GB minimum, 32 GB recommended
- **Display:** HDMI output, 1920x1080 minimum
- **Network:** Ethernet (100Base-T or Gigabit) + Wi-Fi (802.11n/ac)
- **Audio:** 3.5mm audio jack or HDMI audio
- **Power:** 5V 3A USB-C power supply

**Mitigation:**
- Optimize performance for target hardware
- Use efficient algorithms and data structures
- Offload heavy processing to server when possible

**Traces To:**
- Design: [27_PROJECT_STRUCTURE.md](../architecture_and_design/27_PROJECT_STRUCTURE.md)
- Design: [07_SETUP_GUIDE.md](../architecture_and_design/07_SETUP_GUIDE.md)

---

#### [CON-HW-002] Display Resolution and Size

**Constraint:** The system shall support displays with minimum resolution 1280x720 (720p) and maximum 1920x1080 (1080p).

**Rationale:** Common display resolutions for medical monitors. Balance between readability and cost.

**Impact:**
- UI Design: Fixed or responsive layouts
- Font Sizes: Large fonts required for 10-foot readability
- Touch Targets: Minimum 48x48 pixels for gloved hands

**Supported Resolutions:**
- 1280x720 (720p) - Minimum
- 1920x1080 (1080p) - Recommended
- 1920x1200 (WUXGA) - Supported

**Mitigation:**
- Responsive UI design (QML anchors, layouts)
- Test on multiple resolutions
- Scalable fonts and icons

**Traces To:**
- Design: [03_UI_UX_GUIDE.md](../architecture_and_design/03_UI_UX_GUIDE.md)
- Requirement: REQ-INT-HW-002 (touchscreen interface)

---

#### [CON-HW-003] Network Connectivity

**Constraint:** The system requires Ethernet or Wi-Fi network connectivity for server communication but must operate offline for short periods.

**Rationale:** Hospital network connectivity expected but not guaranteed. Offline mode critical for reliability.

**Impact:**
- Network Dependency: Server communication for sync, HIS lookup
- Offline Mode: Local caching, data queuing required
- Failover: Ethernet → Wi-Fi failover supported

**Network Requirements:**
- **Bandwidth:** 10 kbps sustained per device (telemetry sync)
- **Latency:** < 500ms preferred, < 2s acceptable
- **Availability:** 99% uptime preferred
- **Protocols:** HTTPS (port 443), NTP (port 123)

**Mitigation:**
- Local data caching (patients, settings)
- Offline data queuing (vitals, alarms)
- Automatic network recovery
- Ethernet + Wi-Fi redundancy

**Traces To:**
- Requirement: REQ-NFR-REL-005 (graceful degradation)
- Requirement: REQ-FUN-DATA-002 (offline queuing)
- Design: [09_CLASS_DESIGNS.md](../architecture_and_design/09_CLASS_DESIGNS.md) (NetworkManager)

---

### 2.2 Software Constraints

#### [CON-SW-001] Operating System

**Constraint:** The system shall run on Linux-based operating systems (Ubuntu 20.04+, Raspberry Pi OS, Debian 11+).

**Rationale:** Qt framework well-supported on Linux. Open-source reduces licensing costs. Linux suitable for embedded medical devices.

**Impact:**
- Development: Linux-specific APIs (Qt abstracts most)
- Testing: Linux test environment required
- Deployment: Linux installation and configuration

**Supported Operating Systems:**
- Ubuntu 20.04 LTS (Focal Fossa) or later
- Raspberry Pi OS (Debian-based)
- Debian 11 (Bullseye) or later
- Other Debian/Ubuntu derivatives (untested)

**Mitigation:**
- Use Qt for cross-platform abstraction
- Avoid Linux-specific code where possible
- Test on multiple Linux distributions

**Traces To:**
- Requirement: REQ-NFR-PORT-001 (platform independence)
- Design: [07_SETUP_GUIDE.md](../architecture_and_design/07_SETUP_GUIDE.md)

---

#### [CON-SW-002] Qt Framework Version

**Constraint:** The system shall be developed using Qt 6.2+ (LTS) with C++17 language standard.

**Rationale:** Qt 6 is modern framework with long-term support. C++17 provides modern language features.

**Impact:**
- Development: Qt 6 APIs (different from Qt 5)
- Build System: CMake required (qmake deprecated)
- Dependencies: Qt 6 modules (Core, Widgets, Qml, Network, Sql)

**Qt Modules Used:**
- Qt Core (essentials)
- Qt Widgets (desktop UI - if used)
- Qt Qml / Qt Quick (declarative UI)
- Qt Network (HTTP, TLS)
- Qt Sql (SQLite)
- Qt Multimedia (audio alarms - optional)

**Mitigation:**
- Use Qt 6.2 LTS for stability
- Follow Qt 6 best practices
- Avoid deprecated APIs

**Traces To:**
- Design: [27_PROJECT_STRUCTURE.md](../architecture_and_design/27_PROJECT_STRUCTURE.md)
- Design: [07_SETUP_GUIDE.md](../architecture_and_design/07_SETUP_GUIDE.md)

---

#### [CON-SW-003] Database Size Limitations

**Constraint:** The local database size should not exceed 500 MB to manage storage capacity on resource-constrained devices.

**Rationale:** SD cards have limited capacity. Large databases impact performance and increase backup time.

**Impact:**
- Data Retention: 7 days for vitals, 90 days for alarms
- Storage: ~100 MB for 7 days of vitals (estimated)
- Cleanup: Automated cleanup required

**Storage Estimates (7 days, 1 patient):**
- Vitals: ~605,000 records × 150 bytes = ~90 MB
- Alarms: ~100 records × 500 bytes = ~0.05 MB
- Patients: ~10 records × 500 bytes = ~0.005 MB
- Audit Logs: ~1,000 records × 500 bytes = ~0.5 MB
- **Total:** ~91 MB (7 days)

**Mitigation:**
- Automated data cleanup (7-day retention)
- Efficient data structures
- Database compression (SQLite WAL mode)
- Monitor database size (alert if > 400 MB)

**Traces To:**
- Requirement: REQ-DATA-RET-001 (vitals retention)
- Design: [10_DATABASE_DESIGN.md](../architecture_and_design/10_DATABASE_DESIGN.md)

---

### 2.3 Regulatory Constraints

#### [CON-REG-001] Medical Device Classification

**Constraint:** The system is classified as IEC 62304 Class B software (medium risk - injury possible but not death).

**Rationale:** Alarm system failure could result in patient injury. Classification drives testing and documentation requirements.

**Impact:**
- Documentation: Comprehensive documentation required
- Testing: 90%+ code coverage for critical components
- Risk Management: ISO 14971 risk analysis required
- Lifecycle: Full IEC 62304 lifecycle compliance

**Class B Requirements:**
- Software Requirements Specification (SRS)
- Software Architecture Document (SAD)
- Software Detailed Design (SDD)
- Software Test Plan and Report
- Risk Management File
- Traceability Matrix

**Mitigation:**
- Follow IEC 62304 lifecycle processes
- Maintain comprehensive documentation
- Third-party verification recommended

**Traces To:**
- Requirement: REQ-REG-62304-001 (software lifecycle)
- Requirement: REQ-REG-14971-001 (risk management)

---

#### [CON-REG-002] HIPAA Compliance

**Constraint:** The system must comply with HIPAA Privacy Rule and Security Rule as it handles Protected Health Information (PHI).

**Rationale:** Legal requirement. HIPAA violations result in significant penalties ($100-$50,000 per violation).

**Impact:**
- Encryption: PHI encrypted at rest and in transit (mandatory)
- Access Control: Authentication and authorization required
- Audit Logging: Comprehensive audit trail required (6-year retention)
- Breach Notification: Procedures for breach response

**HIPAA Requirements:**
- Privacy Rule (45 CFR Part 160, 164 Subpart E)
- Security Rule (45 CFR Part 160, 164 Subpart C)
- Breach Notification Rule (45 CFR Part 164 Subpart D)

**Mitigation:**
- Implement all HIPAA security requirements
- Conduct annual security risk assessments
- Train staff on HIPAA compliance
- Maintain Business Associate Agreements (BAAs)

**Traces To:**
- Requirement: REQ-REG-HIPAA-001 through REQ-REG-HIPAA-005
- Design: [06_SECURITY.md](../architecture_and_design/06_SECURITY.md)

---

### 2.4 Deployment Constraints

#### [CON-DEP-001] Network Environment

**Constraint:** The system shall operate within hospital network environment with firewall restrictions, VLANs, and network segmentation.

**Rationale:** Hospital networks are highly secured. Devices must comply with IT policies.

**Impact:**
- Network Access: Limited outbound connections (HTTPS only)
- VLAN: Medical devices on separate VLAN
- Firewall: Outbound HTTPS allowed, inbound blocked
- VPN: May require VPN for remote access

**Hospital Network Typical Configuration:**
- Medical Device VLAN (isolated from general network)
- Firewall rules (whitelist outbound destinations)
- No Wi-Fi guest network access
- Certificate-based Wi-Fi authentication (WPA2-Enterprise)

**Mitigation:**
- Use standard protocols (HTTPS, NTP)
- Document required network access
- Coordinate with hospital IT during deployment
- Support both Ethernet and Wi-Fi

**Traces To:**
- Requirement: REQ-SEC-NET-002 (firewall configuration)
- Design: [17_DEVICE_PROVISIONING.md](../architecture_and_design/17_DEVICE_PROVISIONING.md)

---

#### [CON-DEP-002] Installation and Setup

**Constraint:** The system installation and configuration must be performed by trained biomedical technicians, not end users (nurses/physicians).

**Rationale:** Medical device installation requires technical expertise. Misconfiguration could impact patient safety.

**Impact:**
- User Documentation: Separate installation guide vs user guide
- Provisioning: Technician-only provisioning workflow
- Support: Technical support required for installation issues
- Training: Technician training program required

**Installation Steps:**
- Hardware assembly (device, display, power)
- Operating system installation (if not pre-installed)
- Application installation (package or image)
- Device provisioning (QR code workflow)
- Network configuration (Ethernet/Wi-Fi)
- Testing (display, audio, network)
- Commissioning (verification checklist)

**Mitigation:**
- Comprehensive installation documentation
- Automated provisioning (QR code)
- Installation verification checklist
- Remote support capability

**Traces To:**
- Design: [07_SETUP_GUIDE.md](../architecture_and_design/07_SETUP_GUIDE.md)
- Requirement: REQ-FUN-DEV-001 (device provisioning)

---

## 3. Assumptions

### 3.1 Infrastructure Assumptions

#### [ASM-INF-001] Hospital Network Availability

**Assumption:** Hospital network is available 99% of the time with occasional brief outages (< 1 hour).

**Validation:** Confirmed with hospital IT departments.

**Impact:** Device must handle network outages gracefully (offline mode).

**Risk if Invalid:** Frequent/long outages would cause extensive data queuing and sync delays.

**Mitigation:** Offline mode with local data queuing (REQ-FUN-DATA-002).

---

#### [ASM-INF-002] NTP Server Availability

**Assumption:** Hospital network provides access to NTP server for time synchronization.

**Validation:** Typical hospital networks have NTP servers.

**Impact:** Device relies on NTP for accurate timestamps.

**Risk if Invalid:** Clock drift causes data correlation issues across devices.

**Mitigation:** Fallback to server time from API responses. Manual time setting (last resort).

**Traces To:**
- Requirement: REQ-INT-SRV-004 (time synchronization)

---

#### [ASM-INF-003] Central Telemetry Server Availability

**Assumption:** Central telemetry server is deployed and operational before device deployment.

**Validation:** Server is separate project (not part of Z Monitor device).

**Impact:** Device cannot sync data without server.

**Risk if Invalid:** Device operates in offline mode indefinitely.

**Mitigation:** Local data storage, offline operation supported.

---

### 3.2 Integration Assumptions

#### [ASM-INT-001] HIS/EHR Integration

**Assumption:** Hospital Information System (HIS) or EHR provides REST API for patient lookup by MRN.

**Validation:** Common in modern hospital systems (Epic, Cerner).

**Impact:** Device can query patient demographics automatically.

**Risk if Invalid:** Manual patient data entry required (slower, error-prone).

**Mitigation:** Manual entry fallback, patient data caching.

**Traces To:**
- Requirement: REQ-INT-HIS-001 (patient lookup API)

---

#### [ASM-INT-002] ADT Push Notifications

**Assumption:** Hospital may or may not support ADT push notifications (HL7 messages). If not available, Central Station pushes assignments.

**Validation:** Not all hospitals have HL7 interface engines.

**Impact:** If unavailable, manual patient admission required (or Central Station push).

**Risk if Invalid:** Manual workflow slower but acceptable.

**Mitigation:** Support multiple admission methods (manual, barcode, central push).

**Traces To:**
- Requirement: REQ-INT-HIS-002 (ADT notifications - optional)

---

### 3.3 User Assumptions

#### [ASM-USER-001] Clinical Staff Training

**Assumption:** Clinical staff (nurses, physicians) receive 2-hour training on Z Monitor operation before use.

**Validation:** Standard practice for new medical equipment.

**Impact:** Users understand core functions (admission, alarm acknowledgment, settings).

**Risk if Invalid:** User errors increase, efficiency decreases.

**Mitigation:** Intuitive UI design, online help, quick reference guide.

**Traces To:**
- Requirement: REQ-NFR-USE-001 (learning curve 2 hours)

---

#### [ASM-USER-002] User Roles and Permissions

**Assumption:** Hospital assigns users to roles (Nurse, Physician, Technician, Administrator) based on job function.

**Validation:** Standard RBAC practice.

**Impact:** RBAC enforced correctly.

**Risk if Invalid:** Users may lack necessary permissions or have excessive permissions.

**Mitigation:** Clear role definitions, administrator can adjust permissions.

**Traces To:**
- Requirement: REQ-SEC-AUTHZ-001 (RBAC)

---

### 3.4 Data Assumptions

#### [ASM-DATA-001] Patient MRN Uniqueness

**Assumption:** Patient MRN (Medical Record Number) is unique per patient within hospital system.

**Validation:** Standard practice. MRNs are primary patient identifiers.

**Impact:** MRN used as primary key for patient lookup.

**Risk if Invalid:** Patient misidentification (critical safety issue).

**Mitigation:** Verify MRN uniqueness with HIS integration. Display patient name for nurse confirmation.

**Traces To:**
- Requirement: REQ-FUN-PAT-010 (patient lookup)

---

#### [ASM-DATA-002] Data Synchronization Frequency

**Assumption:** Telemetry data sync every 10 seconds is sufficient for centralized monitoring.

**Validation:** Industry practice. Real-time streaming not required for vital signs (1 Hz sampling).

**Impact:** 10-second delay in data appearing on central server.

**Risk if Invalid:** Central station data slightly stale (acceptable).

**Mitigation:** Alarms synced immediately (< 1 second).

**Traces To:**
- Requirement: REQ-FUN-DATA-001 (data sync)

---

### 3.5 Security Assumptions

#### [ASM-SEC-001] Physical Security

**Assumption:** Medical devices are deployed in physically secure locations (hospital units) with controlled access.

**Validation:** Typical hospital environment. Unauthorized visitors escorted.

**Impact:** Physical tampering risk is low (but not zero).

**Risk if Invalid:** Increased risk of device theft or tampering.

**Mitigation:** Tamper detection (REQ-SEC-PHYS-002). Encryption protects data if stolen.

---

#### [ASM-SEC-002] Certificate Authority (CA)

**Assumption:** Hospital operates Certificate Authority (CA) for issuing device certificates or uses third-party CA.

**Validation:** Common in enterprise environments.

**Impact:** Device provisioning uses hospital-issued certificates.

**Risk if Invalid:** Certificate provisioning workflow not possible.

**Mitigation:** Provide CA setup guide. Support self-signed certificates for development.

**Traces To:**
- Requirement: REQ-SEC-CERT-001 (X.509 certificates)
- Design: [15_CERTIFICATE_PROVISIONING.md](../architecture_and_design/15_CERTIFICATE_PROVISIONING.md)

---

#### [ASM-SEC-003] Network Segmentation

**Assumption:** Hospital network segments medical devices on separate VLAN from general network.

**Validation:** Best practice per IEC 62443 and NIST guidance.

**Impact:** Medical device VLAN provides additional security layer.

**Risk if Invalid:** Increased attack surface from general network.

**Mitigation:** Device implements own security measures (firewall, encryption).

**Traces To:**
- Requirement: REQ-SEC-NET-002 (firewall)

---

### 3.6 Performance Assumptions

#### [ASM-PERF-001] Sensor Data Generation Rate

**Assumption:** Vital signs sensors generate data at 1 Hz (1 sample per second) for numeric values, 250 Hz for waveforms.

**Validation:** Industry standard for patient monitoring.

**Impact:** Database write performance, network bandwidth requirements.

**Risk if Invalid:** Higher frequency requires more storage and bandwidth.

**Mitigation:** Configurable sampling rate. Downsampling for storage.

**Traces To:**
- Requirement: REQ-FUN-VITAL-001 (real-time display)
- Requirement: REQ-NFR-PERF-111 (database write throughput)

---

#### [ASM-PERF-002] Concurrent Device Count

**Assumption:** Central server supports 100+ concurrent devices (scalability).

**Validation:** Typical hospital ICU has 20-100 beds.

**Impact:** Server design must handle concurrent connections.

**Risk if Invalid:** Server bottleneck limits deployment scale.

**Mitigation:** Server scalability testing. Load balancing if needed.

**Traces To:**
- Requirement: REQ-NFR-SCALE-001 (concurrent monitoring)

---

## 4. Design Decisions and Trade-offs

### 4.1 Local Database vs Cloud-Only

**Decision:** Use local SQLite database with cloud sync (hybrid approach).

**Rationale:**
- **Reliability:** Offline operation critical for patient safety
- **Performance:** Local database faster than cloud queries
- **Compliance:** Local storage enables offline audit logs

**Trade-off:**
- **Complexity:** Data synchronization complexity
- **Storage:** Local storage required (SD card)

**Alternative Considered:** Cloud-only (rejected due to network dependency).

---

### 4.2 PIN vs Biometric Authentication

**Decision:** Primary authentication via PIN, biometric (fingerprint) optional enhancement.

**Rationale:**
- **Universality:** All users can use PIN (not all have fingerprint readers)
- **Speed:** PIN entry faster than card swipe
- **Cost:** No additional hardware required for PIN

**Trade-off:**
- **Security:** PIN weaker than biometric (but balanced with lockout)
- **Usability:** PIN memorable (4-6 digits)

**Alternative Considered:** RFID badge (common but requires badge reader hardware).

---

### 4.3 QML vs Qt Widgets for UI

**Decision:** Use QML (Qt Quick) for user interface.

**Rationale:**
- **Modern:** QML is modern declarative UI framework
- **Responsive:** QML better for responsive, fluid UIs
- **Animations:** Smooth transitions and animations
- **Touch:** Better touch support

**Trade-off:**
- **Performance:** QML slightly slower than Widgets (acceptable)
- **Learning Curve:** QML requires JavaScript knowledge

**Alternative Considered:** Qt Widgets (traditional, faster but less modern).

---

### 4.4 Repository Pattern vs Direct Database Access

**Decision:** Use Repository Pattern with domain aggregates (DDD).

**Rationale:**
- **Testability:** Repositories mockable for unit tests
- **Abstraction:** Business logic independent of database
- **Flexibility:** Can swap SQLite for other databases

**Trade-off:**
- **Complexity:** More code (repository interfaces and implementations)
- **Performance:** Slight overhead (negligible)

**Alternative Considered:** Direct database access (simpler but less testable).

**Traces To:**
- Design: [30_DATABASE_ACCESS_STRATEGY.md](../architecture_and_design/30_DATABASE_ACCESS_STRATEGY.md)
- Design: [28_DOMAIN_DRIVEN_DESIGN.md](../architecture_and_design/28_DOMAIN_DRIVEN_DESIGN.md)

---

## 5. Future Considerations

### 5.1 Multi-Patient Monitoring

**Current:** One patient per device.

**Future:** Support multiple patients per device (multi-bed view).

**Impact:** UI redesign, database schema changes, more complex alarm management.

---

### 5.2 Remote Monitoring

**Current:** Device-to-central server only.

**Future:** Remote physician access via web/mobile app.

**Impact:** Additional security requirements (web authentication), API development.

---

### 5.3 Predictive Analytics

**Current:** Reactive alarming (threshold-based).

**Future:** Predictive alarms using machine learning (detect trends, predict deterioration).

**Impact:** ML model integration, training data requirements, regulatory approval.

---

### 5.4 Interoperability Standards

**Current:** Custom APIs for HIS/server integration.

**Future:** HL7 FHIR standard for interoperability.

**Impact:** FHIR implementation, data model mapping, testing.

---

## 6. Limitations

### 6.1 Current Limitations

1. **Single Patient:** Device monitors one patient at a time (not multi-patient).
2. **Simulated Sensors:** Production version requires real sensor integration (hardware-dependent).
3. **Limited Vitals:** Heart rate, SpO2, respiration rate only (no blood pressure, temperature).
4. **English Only (Initial):** Multi-language support planned (English + Spanish).
5. **No Video:** No video streaming or recording capability.
6. **Local-Only Trends:** Trends limited to local data (7 days). Historical trends require server query.

### 6.2 Known Issues

*To be documented during development and testing.*

---

## 7. Summary

### Constraints Count: 12
- Hardware: 3
- Software: 3
- Regulatory: 2
- Deployment: 2

### Assumptions Count: 16
- Infrastructure: 3
- Integration: 2
- User: 2
- Data: 2
- Security: 3
- Performance: 2

### Design Decisions: 4
- Local database + cloud sync
- PIN authentication
- QML for UI
- Repository pattern

---

## 8. Related Documents

- **Architecture:** [../architecture_and_design/02_ARCHITECTURE.md](../architecture_and_design/02_ARCHITECTURE.md)
- **Setup Guide:** [../architecture_and_design/07_SETUP_GUIDE.md](../architecture_and_design/07_SETUP_GUIDE.md)
- **All Requirements:** Foundation for constraints

---

*Constraints and assumptions define the boundaries and context for system design. Clear documentation prevents misunderstandings and enables risk management.*

