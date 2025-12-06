# Z Monitor Development Tasks - SEC

## Task ID Convention

**ALL tasks use format: `TASK-{CATEGORY}-{NUMBER}`**

- **See `.github/ztodo_task_guidelines.md` for complete task creation guidelines**

---

## Security Tasks

- [ ] TASK-SEC-001: Implement Certificate Management and Rotation
  - What: Implement certificate lifecycle management in `src/infrastructure/security/CertificateManager.cpp/h`. Supports certificate installation, validation, expiration checking, and automatic rotation. Integrates with `certificates` table in database. Monitors certificate expiration and sends notifications 30 days before expiry. Supports multiple certificate types (TLS client/server, code signing).
  - Why: TLS certificates are required for HIPAA compliance (REQ-REG-HIPAA-002). Certificate expiration can cause service outages. Automatic rotation reduces operational overhead. Certificate validation prevents man-in-the-middle attacks.
  - Files:
    - `src/infrastructure/security/CertificateManager.h/cpp`
    - `src/infrastructure/persistence/SQLiteCertificateRepository.h/cpp`
    - `tests/unit/infrastructure/security/CertificateManagerTest.cpp`
    - Update `schema/database.yaml` (verify `certificates` table schema)
  - Acceptance: Certificates installed/validated, expiration checking works, rotation triggered 30 days before expiry, notifications sent, multiple certificate types supported, unit tests verify lifecycle management.
  - Verification Steps:
    1. Functional: Certificates installed, validated, expiration detected, rotation works, notifications sent
    2. Code Quality: Doxygen comments, proper error handling, follows security best practices
    3. Documentation: Update `project-dashboard/doc/legacy/architecture_and_design/15_CERTIFICATE_PROVISIONING.md` with implementation details
    4. Integration: Works with DatabaseManager, notifications system
    5. Tests: Unit tests for lifecycle management, expiration detection, validation
  - Dependencies: SQLiteCertificateRepository, DatabaseManager, Schema Management
  - Documentation: See `project-dashboard/doc/legacy/architecture_and_design/15_CERTIFICATE_PROVISIONING.md` for certificate lifecycle design.
  - Prompt: `project-dashboard/prompt/TASK-SEC-001-certificate-management.md`

- [ ] TASK-SEC-002: Implement Audit Trail with Hash Chain Verification
  - What: Implement audit trail hash chain in `src/infrastructure/persistence/SQLiteAuditRepository.cpp/h` that prevents tampering with audit logs. Each audit entry includes hash of previous entry, forming immutable chain. Implements verification function that checks chain integrity. Integrates with `security_audit_log` table. All security-relevant events logged (login, logout, permission changes, certificate changes, configuration changes).
  - Why: HIPAA requires tamper-proof audit logs (REQ-REG-HIPAA-003, REQ-SEC-AUDIT-002). Hash chain ensures audit trail integrity. Verification detects any tampering attempts.
  - Files:
    - `src/infrastructure/persistence/SQLiteAuditRepository.h/cpp`
    - `src/infrastructure/security/AuditHashChain.h/cpp`
    - `tests/unit/infrastructure/persistence/SQLiteAuditRepositoryTest.cpp`
    - Update `schema/database.yaml` (verify `security_audit_log` table schema)
  - Acceptance: Audit entries include hash chain, chain integrity verified, tampering detected, all security events logged, unit tests verify hash chain integrity and tampering detection.
  - Verification Steps:
    1. Functional: Audit entries logged with hash, chain integrity verified, tampering detected
    2. Code Quality: Doxygen comments, cryptographically secure hash (SHA-256), follows security best practices
    3. Documentation: Update `project-dashboard/doc/guidelines/DOC-GUIDE-012_logging.md` with audit hash chain design
    4. Integration: Works with DatabaseManager, all services log to audit trail
    5. Tests: Unit tests for hash chain, integrity verification, tampering detection
  - Dependencies: SQLiteAuditRepository, DatabaseManager, Schema Management
  - Documentation: See `project-dashboard/doc/guidelines/DOC-GUIDE-012_logging.md` for audit logging design. See `project-dashboard/doc/processes/DOC-PROC-014_authentication_workflow.md` for security events.
  - Prompt: `project-dashboard/prompt/TASK-SEC-002-audit-hash-chain.md`

- [ ] TASK-SEC-003: Implement SQLCipher Integration for Database Encryption
  - What: Implement SQLCipher integration following the plan in `project-dashboard/doc/architecture/DOC-ARCH-018_sqlcipher_integration.md`. Add key derivation (PBKDF2 with 256,000 iterations), key storage in Qt Keychain, encryption settings configuration. Update `DatabaseManager` to support encrypted databases. Add migration path from unencrypted to encrypted databases.
  - Why: HIPAA requires encryption at rest for Protected Health Information (REQ-REG-HIPAA-001). SQLCipher provides transparent database encryption. Qt Keychain ensures secure key storage.
  - Files:
    - `src/infrastructure/persistence/DatabaseManager.cpp/h` (add SQLCipher support)
    - `src/infrastructure/security/KeyManager.h/cpp` (key derivation and storage)
    - `src/infrastructure/security/DatabaseEncryption.h/cpp` (encryption setup)
    - `tests/unit/infrastructure/security/DatabaseEncryptionTest.cpp`
    - Update `CMakeLists.txt` (add SQLCipher dependency when ENABLE_SQLCIPHER=ON)
  - Acceptance: SQLCipher encrypts database, key derivation works (PBKDF2), key stored in Qt Keychain, encryption settings configurable, migration from unencrypted to encrypted works, unit tests verify encryption and key management.
  - Verification Steps:
    1. Functional: Database encrypted, key derivation works, key storage works, migration works
    2. Code Quality: Doxygen comments, secure key handling (keys never logged), follows security best practices
    3. Documentation: `project-dashboard/doc/architecture/DOC-ARCH-018_sqlcipher_integration.md` implementation status updated
    4. Integration: Works with DatabaseManager, all repositories work with encrypted database
    5. Tests: Unit tests for encryption, key management, migration
  - Dependencies: SQLCipher library, Qt Keychain library, DatabaseManager
  - Documentation: See `project-dashboard/doc/architecture/DOC-ARCH-018_sqlcipher_integration.md` for SQLCipher integration plan. See `project-dashboard/doc/guidelines/DOC-GUIDE-014_database_access_strategy.md` for database architecture.
  - Prompt: `project-dashboard/prompt/TASK-SEC-003-sqlcipher-integration.md`

- [ ] TASK-SEC-005: Implement Secure Provisioning
  - What: Implement the device provisioning workflow (initial setup, server pairing, key exchange).
  - Why: Ensures devices are securely onboarded to the hospital network.
  - Files:
    - `z-monitor/src/application/ProvisioningManager.h/cpp`
    - `z-monitor/src/ui/views/ProvisioningView.qml`
  - Acceptance: Device generates keys, exchanges with server, receives config, enters active state.
  - Verification Steps:
    1. Functional: Provisioning flow works end-to-end
    2. Code Quality: Doxygen comments, secure coding
    3. Documentation: Provisioning flow documented
    4. Integration: Works with UI and Network
    5. Tests: Integration tests for flow
  - Prompt: `project-dashboard/prompt/52-implement-secure-provisioning.md`

- [ ] TASK-SEC-006: Implement mTLS for Telemetry
  - What: Configure `TelemetryService` to use mutual TLS (mTLS) for all connections to the central server.
  - Why: Mandatory security requirement for transmitting PHI (Protected Health Information).
  - Files:
    - `z-monitor/src/infrastructure/network/TelemetryService.cpp` (update)
    - `z-monitor/src/infrastructure/network/TlsConfiguration.h/cpp`
  - Acceptance: Connection rejected without valid cert, accepted with valid cert, encrypted traffic.
  - Verification Steps:
    1. Functional: mTLS connection works, invalid cert rejected
    2. Code Quality: Doxygen comments
    3. Documentation: mTLS setup documented
    4. Integration: Works with Central Server Sim
    5. Tests: Integration tests with/without certs
  - Prompt: `project-dashboard/prompt/53-implement-mtls-telemetry.md`

- [ ] TASK-SEC-008: Implement Key Management
  - What: Implement secure storage for the database encryption key (e.g., using OS keychain or hardware security module if available, or obfuscated file).
  - Why: Prevents the encryption key from being easily compromised.
  - Files:
    - `z-monitor/src/infrastructure/security/KeyStore.h/cpp`
  - Acceptance: Key stored securely, retrieved on startup, rotated if needed.
  - Verification Steps:
    1. Functional: Key storage/retrieval works
    2. Code Quality: Secure coding
    3. Documentation: Key management documented
    4. Integration: Used by DatabaseManager
    5. Tests: Unit tests for KeyStore
  - Prompt: `project-dashboard/prompt/55-implement-key-management.md`
