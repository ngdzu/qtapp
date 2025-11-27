# Documentation Improvements and Clarifications

**Document ID:** DESIGN-016  
**Version:** 1.0  
**Status:** Approved  
**Last Updated:** 2025-11-27

---

This document captures comprehensive improvements made to the Z Monitor documentation to eliminate ambiguities, add missing features, and address security concerns.

## 1. Ambiguities Resolved

### 1.1. Authentication and Session Management
**Issue:** PIN storage, session timeout, and brute force protection details were vague.

**Resolved:**
- PIN storage: SHA-256 with per-user salt, stored in `users` table
- Session timeout: 30 minutes of inactivity (configurable)
- Brute force protection: 5 failed attempts → 15-minute lockout, exponential backoff
- Session refresh: Automatic refresh on activity, manual refresh available
- PIN complexity: Minimum 4 digits, maximum 8 digits (configurable)

### 1.2. Database Encryption Key Management
**Issue:** Key storage in development was unclear, key rotation not specified.

**Resolved:**
- Development: Key stored in `resources/config/db.key` with 600 permissions, never committed to repo
- Production: Hardware-backed keystore (HSM) or secure element preferred
- Key rotation: Documented process for rotating encryption keys without data loss
- Key derivation: PBKDF2 with 100,000 iterations, device-specific salt

### 1.3. Certificate Revocation Checking
**Issue:** CRL checking marked as "optional" but should be mandatory for production.

**Resolved:**
- Production: CRL checking is **mandatory** (not optional)
- Development: CRL checking can be disabled for testing
- CRL update frequency: Every 24 hours (configurable)
- CRL distribution: Via secure channel, validated with CA signature

### 1.4. Data Retention Policies
**Issue:** Inconsistent retention periods mentioned (7 days vs 90 days).

**Resolved:**
- Vitals data: 7 days (configurable, default)
- Alarm history: 90 days (configurable)
- Security audit log: 90 days (configurable, minimum for compliance)
- Device events: 30 days (configurable)
- Patient data: Until patient discharge + 30 days (configurable)

### 1.5. Error Recovery Mechanisms
**Issue:** Error handling and recovery procedures not fully specified.

**Resolved:**
- Network failures: Exponential backoff, circuit breaker pattern, retry queue
- Database errors: Transaction rollback, error logging, graceful degradation
- Certificate validation failures: Detailed error codes, recovery procedures
- Data sync failures: Persistent retry queue, conflict resolution strategy

### 1.6. Thread Safety Guarantees
**Issue:** Thread safety not explicitly documented for all components.

**Resolved:**
- Documented thread-safety guarantees for each component
- Specified which operations are thread-safe vs require single-thread access
- Added synchronization primitives documentation

## 2. Missing Features Added

### 2.1. Backup and Restore
**Added:**
- Automated daily backups of database and configuration
- Backup location: `resources/backups/` (encrypted)
- Restore procedure: Via Settings → System → Restore
- Backup retention: 30 days (configurable)
- Backup verification: Checksum validation after backup

### 2.2. Firmware Updates
**Added:**
- Over-the-air (OTA) firmware update mechanism
- Update verification: Digital signature validation
- Rollback capability: Automatic rollback on update failure
- Update scheduling: Can be scheduled during low-activity periods
- Update notifications: User notification before update installation

### 2.3. Device Registration
**Added:**
- Initial device registration with central server
- Registration process: Device ID + certificate serial → server registration
- Registration verification: Server confirms device registration
- Re-registration: Process for re-registering devices after factory reset

### 2.4. Clock Synchronization
**Added:**
- NTP (Network Time Protocol) synchronization
- Fallback: Manual time setting if NTP unavailable
- Clock drift detection: Alerts if clock drift exceeds threshold
- Timezone configuration: Configurable timezone settings

### 2.5. Log Rotation and Management
**Added:**
- Automatic log rotation: Daily rotation, 7-day retention
- Log compression: Compressed logs after 24 hours
- Log levels: Configurable per component
- Log export: Export logs for analysis (encrypted export)

### 2.6. Performance Monitoring
**Added:**
- System metrics collection: CPU, memory, disk usage
- Performance dashboard: Available in Diagnostics View
- Alert thresholds: Alerts on high resource usage
- Historical metrics: 24-hour rolling window

### 2.7. Device Health Monitoring
**Added:**
- Extended health checks: Beyond basic telemetry
- Component health: Individual component status
- Predictive maintenance: Alerts for potential failures
- Health reporting: Periodic health reports to central server

### 2.8. Offline Mode Handling
**Added:**
- Offline operation: Full functionality when server unavailable
- Data queuing: Queued data sent when connection restored
- Offline indicators: Clear UI indicators for offline state
- Offline time limits: Alerts if offline for extended period

### 2.9. Data Export
**Added:**
- Patient data export: Export patient data for analysis
- Export formats: CSV, JSON, HL7 FHIR (configurable)
- Export encryption: Encrypted exports for sensitive data
- Export audit: All exports logged for audit

### 2.10. Factory Reset
**Added:**
- Factory reset procedure: Complete device reset
- Reset confirmation: Multi-step confirmation required
- Data erasure: Secure data erasure (overwrite with random data)
- Certificate cleanup: Certificates removed during reset
- Reset logging: Factory reset logged for audit

### 2.11. Device Pairing/Binding
**Added:**
- Device-to-bed pairing: Associate device with bed location
- Pairing verification: Visual confirmation of pairing
- Unpairing: Process for unpairing devices
- Pairing history: Log of pairing/unpairing events

## 3. Security Issues Addressed

### 3.1. PIN Brute Force Protection
**Issue:** Rate limiting mentioned but specifics missing.

**Resolved:**
- Lockout policy: 5 failed attempts → 15-minute lockout
- Exponential backoff: Lockout duration doubles after each violation
- Account lockout: Permanent lockout after 10 failed attempts (requires admin unlock)
- Audit logging: All failed login attempts logged

### 3.2. Session Management
**Issue:** Session timeout and refresh not detailed.

**Resolved:**
- Session timeout: 30 minutes of inactivity (configurable)
- Session refresh: Automatic on activity, manual refresh available
- Concurrent sessions: Single session per user (new login invalidates old)
- Session tokens: Cryptographically secure session tokens

### 3.3. Secure Boot and Firmware Integrity
**Issue:** Not mentioned in documentation.

**Resolved:**
- Secure boot: Bootloader verifies firmware signature
- Firmware integrity: SHA-256 checksums verified on boot
- Tamper detection: Hardware tamper detection (if available)
- Boot failure handling: Device enters safe mode on verification failure

### 3.4. Tamper Detection
**Issue:** Not mentioned in documentation.

**Resolved:**
- Physical tamper: Hardware tamper detection (if available)
- Software tamper: Certificate validation, file integrity checks
- Tamper response: Immediate security event, device lockout, alert to server
- Tamper logging: All tamper events logged to security audit log

### 3.5. Secure Erase for Decommissioning
**Issue:** Not mentioned in documentation.

**Resolved:**
- Secure erase procedure: Multi-pass overwrite with random data
- Certificate revocation: Certificates revoked during decommissioning
- Data erasure verification: Verification of successful erasure
- Decommissioning audit: Complete audit trail of decommissioning

### 3.6. Incident Response Procedures
**Issue:** Not mentioned in documentation.

**Resolved:**
- Security incident classification: Critical, High, Medium, Low
- Incident response steps: Documented procedures for each severity
- Incident notification: Automatic notification to security team
- Incident logging: All incidents logged with full details

### 3.7. Clock Skew Tolerance
**Issue:** ±5 minutes might be too lenient.

**Resolved:**
- Production: ±1 minute tolerance (stricter)
- Development: ±5 minutes tolerance (for testing)
- Clock sync enforcement: NTP synchronization required in production
- Clock drift alerts: Alerts if clock drift exceeds threshold

### 3.8. Database Encryption Key Storage
**Issue:** Development key storage guidance insufficient.

**Resolved:**
- Development: `resources/config/db.key` with 600 permissions
- Key file location: Never in version control, added to `.gitignore`
- Key derivation: PBKDF2 with device-specific salt
- Key rotation: Documented rotation procedure

## 4. Documentation Updates Made

The following documents were updated to reflect these improvements:

1. **06_SECURITY.md**: Enhanced with detailed authentication, session management, secure boot, tamper detection, and incident response
2. **09_CLASS_DESIGNS.md**: Updated AuthenticationService, DatabaseManager, NetworkManager with new methods and properties
3. **10_DATABASE_DESIGN.md**: Clarified retention policies, added backup/restore tables
4. **08_DEFINITIONS.md**: Added definitions for new features and security concepts
5. **02_ARCHITECTURE.md**: Added new components (BackupManager, FirmwareManager, HealthMonitor)
6. **03_UI_UX_GUIDE.md**: Added UI specifications for new features
7. **07_SETUP_GUIDE.md**: Added setup instructions for new features

## 5. Implementation Priority

### High Priority (Security-Critical)
1. PIN brute force protection
2. Session management
3. Certificate revocation checking (mandatory)
4. Secure boot and firmware integrity
5. Database encryption key management

### Medium Priority (Feature-Complete)
1. Backup and restore
2. Device registration
3. Clock synchronization
4. Log rotation
5. Offline mode handling

### Low Priority (Nice-to-Have)
1. Performance monitoring
2. Device health monitoring
3. Data export
4. Factory reset
5. Device pairing

## 6. Testing Requirements

All new features and security improvements require:
- Unit tests for core functionality
- Integration tests for end-to-end flows
- Security tests for authentication and authorization
- Performance tests for critical paths
- Compliance tests for audit logging

## 7. Compliance Considerations

- **HIPAA**: All patient data encryption, audit logging, access controls
- **FDA**: Device registration, firmware updates, secure boot
- **IEC 62304**: Software lifecycle, risk management, validation
- **ISO 27001**: Security management, incident response, access control

