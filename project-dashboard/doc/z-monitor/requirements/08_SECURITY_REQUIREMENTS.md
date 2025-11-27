# Security Requirements

**Document ID:** REQ-DOC-08  
**Version:** 0.1  
**Status:** In Progress  
**Last Updated:** 2025-11-27

---

## 1. Overview

This document provides detailed security requirements for the Z Monitor system. Security is critical for protecting patient health information (PHI) and ensuring device integrity in a medical environment.

**Related Documents:**
- **Security Design:** [../architecture_and_design/06_SECURITY.md](../architecture_and_design/06_SECURITY.md)
- **Regulatory Requirements:** [07_REGULATORY_REQUIREMENTS.md](./07_REGULATORY_REQUIREMENTS.md)
- **Non-Functional Requirements:** [04_NON_FUNCTIONAL_REQUIREMENTS.md](./04_NON_FUNCTIONAL_REQUIREMENTS.md)

---

## 2. Security Requirements Categories

### 2.1 Authentication (REQ-SEC-AUTH-###)
User authentication, session management, credential security

### 2.2 Authorization (REQ-SEC-AUTHZ-###)
Access control, role-based permissions, privilege management

### 2.3 Encryption (REQ-SEC-ENC-###)
Data encryption (at rest, in transit), cryptographic algorithms

### 2.4 Certificate Management (REQ-SEC-CERT-###)
mTLS certificates, provisioning, renewal, revocation

### 2.5 Audit and Logging (REQ-SEC-AUDIT-###)
Security event logging, audit trails, compliance

### 2.6 Network Security (REQ-SEC-NET-###)
Network protocols, firewalls, intrusion detection

### 2.7 Physical Security (REQ-SEC-PHYS-###)
Device tampering, secure boot, hardware security

### 2.8 Incident Response (REQ-SEC-INC-###)
Breach detection, response procedures, forensics

---

## 3. Authentication Requirements

### [REQ-SEC-AUTH-001] PIN-Based Authentication

**Category:** Authentication  
**Priority:** Critical (Must Have)  
**Status:** Approved

**Description:**
The system shall authenticate users via PIN with cryptographic hash storage to prevent unauthorized access while balancing usability in clinical environment.

**Rationale:**
Medical staff need quick access for patient emergencies. PIN provides balance between security and usability. Cryptographic hashing prevents PIN compromise.

**Acceptance Criteria:**
- **PIN Format:**
  - Length: 4-6 digits
  - Character set: Numeric only (0-9)
  - Validation: No sequences (1234, 4321), no repeating (1111)
- **Hash Algorithm:**
  - bcrypt (cost factor 12+) OR Argon2id
  - Per-user salt (128-bit random)
  - Never store plaintext PIN
- **Brute Force Protection:**
  - 3 failed attempts → account locked 10 minutes
  - Lockout counter resets after successful login
  - Progressive delays: 0s, 1s, 3s (after each failure)
- **PIN Management:**
  - Initial PIN set during user creation
  - PIN change supported (administrators only)
  - PIN history: Cannot reuse last 3 PINs
  - PIN expiry: Optional (90 days recommended)
- **Emergency Override:**
  - Administrator can reset locked accounts
  - Override logged for audit

**Hash Example (bcrypt):**
```cpp
// PIN: "123456"
// bcrypt hash (cost 12):
// $2b$12$LQv3c1yqBWVHxkd0LHAkCOYz6TtxMQJqhN8/LJHVLhzCOYz6TtxMQ
```

**Related Requirements:**
- REQ-FUN-USER-001 (user authentication)
- REQ-FUN-USER-005 (brute force protection)
- REQ-NFR-SEC-001 (authentication security)
- REQ-REG-HIPAA-002 (access control)

**Traces To:**
- Use Case: UC-UA-001
- Design: [06_SECURITY.md](../architecture_and_design/06_SECURITY.md) (Section 3)
- Design: [09_CLASS_DESIGNS.md](../architecture_and_design/09_CLASS_DESIGNS.md) (AuthenticationService)
- Test: Test-SEC-AUTH-001

**Notes:**
- Consider biometric authentication (fingerprint) as enhancement
- Emergency "break glass" access requires separate mechanism

---

### [REQ-SEC-AUTH-002] Session Security

**Category:** Authentication  
**Priority:** Must Have  
**Status:** Approved

**Description:**
The system shall implement secure session management with automatic timeout, session token protection, and concurrent session control.

**Rationale:**
Active sessions are attack vectors. Secure session management prevents unauthorized access via stolen sessions.

**Acceptance Criteria:**
- **Session Token:**
  - UUID v4 (128-bit random)
  - Generated cryptographically (QUuid::createUuid())
  - Not predictable or sequential
- **Session Storage:**
  - In-memory (QHash<SessionId, SessionData>)
  - Not persisted to disk (security)
  - Cleared on application exit
- **Session Timeout:**
  - Inactivity timeout: 15 minutes (configurable)
  - Warning at 14 minutes: "Auto-logout in 1 minute"
  - User can dismiss warning to extend session
- **Absolute Timeout:**
  - Maximum session duration: 24 hours
  - Requires re-authentication after 24 hours
- **Concurrent Sessions:**
  - One session per user per device (no multi-session)
  - New login terminates previous session
  - Option: "Already logged in elsewhere. Force logout?"
- **Session Termination:**
  - Explicit logout clears session immediately
  - Timeout clears session automatically
  - Application exit clears all sessions

**Session Data Structure:**
```cpp
struct SessionData {
    QString sessionId;       // UUID
    QString userId;          // "NURSE001"
    UserRole role;           // NURSE, PHYSICIAN, etc.
    QStringList permissions; // [ADMIT_PATIENT, ACK_ALARM, ...]
    QDateTime loginTime;     // 2025-11-27 07:00:00
    QDateTime lastActivity;  // 2025-11-27 07:14:32
    QString deviceId;        // "ZM-ICU-MON-04"
};
```

**Related Requirements:**
- REQ-FUN-USER-002 (session management)
- REQ-FUN-USER-003 (auto-logout)
- REQ-NFR-SEC-001 (authentication security)

**Traces To:**
- Use Case: UC-UA-002, UC-UA-003
- Design: [09_CLASS_DESIGNS.md](../architecture_and_design/09_CLASS_DESIGNS.md) (AuthenticationService)
- Test: Test-SEC-AUTH-002

---

### [REQ-SEC-AUTH-003] Session Timeout Configuration

**Category:** Authentication  
**Priority:** Should Have  
**Status:** Approved

**Description:**
The system should allow administrators to configure session timeout duration to balance security and usability per hospital policy.

**Rationale:**
Different hospitals have different security policies. Configurable timeout enables policy compliance.

**Acceptance Criteria:**
- **Configuration Options:**
  - Timeout duration: 5, 10, 15, 30, 60 minutes
  - Default: 15 minutes
  - Stored in settings table
- **Administrator Only:**
  - Only administrators can change timeout
  - Change requires justification (logged)
- **Warning Timing:**
  - Warning displayed at timeout - 1 minute
  - Configurable: timeout - N minutes
- **Compliance:**
  - HIPAA recommended: 15 minutes or less
  - Rationale documented for longer timeouts

**Related Requirements:**
- REQ-SEC-AUTH-002 (session security)
- REQ-FUN-SYS-001 (settings)

**Traces To:**
- Design: [09_CLASS_DESIGNS.md](../architecture_and_design/09_CLASS_DESIGNS.md) (SettingsManager)
- Test: Test-SEC-AUTH-003

---

### [REQ-SEC-AUTH-004] Failed Login Attempt Logging

**Category:** Authentication  
**Priority:** Must Have  
**Status:** Approved

**Description:**
The system shall log all failed authentication attempts with sufficient detail for security incident investigation.

**Rationale:**
Failed login attempts may indicate brute force attacks or unauthorized access attempts. Logging enables detection and investigation.

**Acceptance Criteria:**
- **Logged Fields:**
  - Timestamp (precise to millisecond)
  - User ID attempted
  - Failure reason (INVALID_PIN, ACCOUNT_LOCKED, INVALID_USER)
  - Device ID
  - IP address (if networked)
  - Session attempt ID
- **Log Destination:**
  - Security audit log (security_audit_log table)
  - Local syslog (optional)
- **Analysis Support:**
  - Query: Failed attempts per user (detect attacks)
  - Query: Failed attempts per time period
  - Alert: > 10 failed attempts in 5 minutes (anomaly)
- **Privacy:**
  - Do NOT log PIN values (security risk)
  - Log hash values only if necessary for forensics

**Related Requirements:**
- REQ-SEC-AUDIT-001 (comprehensive audit)
- REQ-FUN-USER-005 (brute force protection)
- REQ-REG-HIPAA-003 (audit trail)

**Traces To:**
- Design: [21_LOGGING_STRATEGY.md](../architecture_and_design/21_LOGGING_STRATEGY.md)
- Design: [10_DATABASE_DESIGN.md](../architecture_and_design/10_DATABASE_DESIGN.md) (security_audit_log)
- Test: Test-SEC-AUTH-004

---

## 4. Authorization Requirements

### [REQ-SEC-AUTHZ-001] Role-Based Access Control (RBAC)

**Category:** Authorization  
**Priority:** Must Have  
**Status:** Approved

**Description:**
The system shall implement role-based access control (RBAC) with predefined roles and permissions to enforce least privilege principle.

**Rationale:**
HIPAA minimum necessary principle. Users should only perform actions required for their job. RBAC simplifies permission management.

**Acceptance Criteria:**
- **Roles Defined:**
  - **Nurse:** Admit/discharge patients, acknowledge alarms, view vitals, configure alarm thresholds (physician approval)
  - **Physician:** All nurse permissions + override alarm thresholds, access advanced settings
  - **Technician:** Device diagnostics, firmware updates, provisioning (NO patient data access)
  - **Administrator:** All permissions + user management, system configuration, audit logs
- **Permissions:**
  - ADMIT_PATIENT
  - DISCHARGE_PATIENT
  - TRANSFER_PATIENT
  - ACKNOWLEDGE_ALARM
  - SILENCE_ALARM
  - CONFIGURE_ALARM_THRESHOLDS
  - VIEW_PATIENT_VITALS
  - VIEW_PATIENT_TRENDS
  - EXPORT_DATA
  - ACCESS_SETTINGS
  - MANAGE_USERS
  - VIEW_AUDIT_LOGS
  - SYSTEM_DIAGNOSTICS
  - FIRMWARE_UPDATE
- **Permission Checks:**
  - Before every sensitive operation
  - Check: `if (session.hasPermission("ADMIT_PATIENT"))`
  - Unauthorized: Display error "Insufficient permissions"
- **Permission Enforcement:**
  - UI-level: Disable unauthorized buttons (grayed out)
  - Business logic level: Verify permissions before action
  - Database level: Not enforced (application-level only)

**Role-Permission Matrix:**
| Permission | Nurse | Physician | Technician | Admin |
|------------|-------|-----------|------------|-------|
| ADMIT_PATIENT | ✓ | ✓ | ✗ | ✓ |
| DISCHARGE_PATIENT | ✓ | ✓ | ✗ | ✓ |
| ACKNOWLEDGE_ALARM | ✓ | ✓ | ✗ | ✓ |
| CONFIGURE_ALARM_THRESHOLDS | 🔶 | ✓ | ✗ | ✓ |
| VIEW_PATIENT_VITALS | ✓ | ✓ | ✗ | ✓ |
| EXPORT_DATA | 🔶 | ✓ | ✗ | ✓ |
| SYSTEM_DIAGNOSTICS | ✗ | ✗ | ✓ | ✓ |
| MANAGE_USERS | ✗ | ✗ | ✗ | ✓ |

*🔶 = Requires approval or justification*

**Related Requirements:**
- REQ-FUN-USER-004 (RBAC)
- REQ-DATA-SEC-002 (access control)
- REQ-REG-HIPAA-004 (minimum necessary)

**Traces To:**
- Design: [06_SECURITY.md](../architecture_and_design/06_SECURITY.md) (Section 4)
- Design: [09_CLASS_DESIGNS.md](../architecture_and_design/09_CLASS_DESIGNS.md) (AuthenticationService)
- Test: Test-SEC-AUTHZ-001

---

### [REQ-SEC-AUTHZ-002] Permission Audit Logging

**Category:** Authorization  
**Priority:** Must Have  
**Status:** Approved

**Description:**
The system shall log all permission checks (success and denial) for audit trail and security analysis.

**Rationale:**
Authorization failures may indicate unauthorized access attempts. Successful checks demonstrate compliance with access control.

**Acceptance Criteria:**
- **Logged Events:**
  - Permission granted (success)
  - Permission denied (failure)
- **Logged Fields:**
  - Timestamp
  - User ID
  - Role
  - Permission requested
  - Resource accessed (patient MRN, setting name, etc.)
  - Outcome (GRANTED, DENIED)
  - Reason for denial (if applicable)
- **Log Volume:**
  - Log permission denials: Always
  - Log permission grants: For sensitive operations only (to reduce volume)
  - Sensitive: EXPORT_DATA, MANAGE_USERS, VIEW_AUDIT_LOGS
- **Analysis:**
  - Query: All permission denials per user (detect unauthorized attempts)
  - Query: Permission grants for sensitive operations

**Related Requirements:**
- REQ-SEC-AUTHZ-001 (RBAC)
- REQ-SEC-AUDIT-001 (comprehensive audit)

**Traces To:**
- Design: [21_LOGGING_STRATEGY.md](../architecture_and_design/21_LOGGING_STRATEGY.md)
- Test: Test-SEC-AUTHZ-002

---

## 5. Encryption Requirements

### [REQ-SEC-ENC-001] Encryption in Transit - TLS 1.2+

**Category:** Encryption  
**Priority:** Critical (Must Have)  
**Status:** Approved

**Description:**
The system shall encrypt all network communication using TLS 1.2 or higher with strong cipher suites to protect PHI in transit.

**Rationale:**
HIPAA requires encryption of PHI in transit. TLS is industry standard for secure communication. Prevents man-in-the-middle attacks.

**Acceptance Criteria:**
- **TLS Version:**
  - TLS 1.2 (minimum)
  - TLS 1.3 (preferred)
  - TLS 1.1 and below: Disabled (vulnerable)
- **Cipher Suites (Preferred Order):**
  1. TLS_ECDHE_RSA_WITH_AES_256_GCM_SHA384
  2. TLS_ECDHE_RSA_WITH_AES_128_GCM_SHA256
  3. TLS_ECDHE_RSA_WITH_CHACHA20_POLY1305_SHA256
  4. TLS_DHE_RSA_WITH_AES_256_GCM_SHA384
- **Disabled Cipher Suites:**
  - RC4 (broken)
  - DES/3DES (weak)
  - Export ciphers
  - NULL ciphers
  - Anonymous DH (no authentication)
- **Key Exchange:**
  - ECDHE (Elliptic Curve Diffie-Hellman Ephemeral) - preferred (PFS)
  - DHE (Diffie-Hellman Ephemeral) - acceptable (PFS)
  - RSA key exchange - deprecated (no PFS)
- **Perfect Forward Secrecy (PFS):**
  - Required (ECDHE or DHE)
  - Compromise of long-term keys doesn't compromise past sessions

**Qt QSslConfiguration Example:**
```cpp
QSslConfiguration sslConfig = QSslConfiguration::defaultConfiguration();
sslConfig.setProtocol(QSsl::TlsV1_2OrLater);
sslConfig.setCiphers(QSslCipher::supportedCiphers()); // Filter for strong ciphers
request.setSslConfiguration(sslConfig);
```

**Related Requirements:**
- REQ-NFR-SEC-002 (encryption in transit)
- REQ-REG-HIPAA-001 (encryption mandate)
- REQ-SEC-ENC-002 (mTLS)

**Traces To:**
- Design: [06_SECURITY.md](../architecture_and_design/06_SECURITY.md) (Section 6)
- Design: [09_CLASS_DESIGNS.md](../architecture_and_design/09_CLASS_DESIGNS.md) (NetworkManager)
- Test: Test-SEC-ENC-001 (TLS verification, cipher suite testing)

**Notes:**
- TLS 1.3 removes weak ciphers by default
- PCI DSS also requires TLS 1.2+

---

### [REQ-SEC-ENC-002] Mutual TLS (mTLS) Authentication

**Category:** Encryption  
**Priority:** Critical (Must Have)  
**Status:** Approved

**Description:**
The system shall use mutual TLS (mTLS) for device-to-server authentication, providing bidirectional certificate-based authentication.

**Rationale:**
mTLS provides strong device authentication (not just passwords). Server verifies device certificate, device verifies server certificate. Prevents unauthorized devices.

**Acceptance Criteria:**
- **Client Certificate:**
  - Device presents client certificate to server
  - Certificate contains device ID (Common Name)
  - Certificate signed by hospital CA
- **Server Certificate:**
  - Server presents certificate to device
  - Device verifies server certificate (CA validation)
  - Certificate pinning optional (enhanced security)
- **Certificate Validation:**
  - Both sides validate certificate chain to trusted CA
  - Verify certificate not expired
  - Verify certificate not revoked (OCSP or CRL)
  - Verify certificate hostname matches (server)
- **Handshake Failure:**
  - Invalid certificate → Connection refused
  - Log security event
  - Display error to user
- **Fallback:**
  - No fallback to non-mTLS (security)
  - If mTLS fails, device cannot connect

**mTLS Handshake:**
```
1. Client → Server: ClientHello
2. Server → Client: ServerHello, Certificate (server cert)
3. Client verifies server cert
4. Server → Client: CertificateRequest
5. Client → Server: Certificate (client cert)
6. Server verifies client cert
7. Client → Server: ClientKeyExchange, CertificateVerify
8. Both: Change Cipher Spec, Finished
9. Encrypted application data
```

**Related Requirements:**
- REQ-SEC-ENC-001 (TLS)
- REQ-SEC-CERT-001 (certificate management)
- REQ-FUN-DEV-002 (certificate management)

**Traces To:**
- Design: [06_SECURITY.md](../architecture_and_design/06_SECURITY.md) (Section 6)
- Design: [15_CERTIFICATE_PROVISIONING.md](../architecture_and_design/15_CERTIFICATE_PROVISIONING.md)
- Test: Test-SEC-ENC-002 (mTLS handshake testing)

---

### [REQ-SEC-ENC-003] Encryption at Rest - SQLCipher AES-256

**Category:** Encryption  
**Priority:** Critical (Must Have)  
**Status:** Approved

**Description:**
The system shall encrypt local database using SQLCipher with AES-256-CBC to protect PHI at rest from unauthorized access.

**Rationale:**
HIPAA requires encryption of PHI at rest. Device theft or unauthorized file access must not expose patient data.

**Acceptance Criteria:**
- **Encryption Library:**
  - SQLCipher 4.x
  - AES-256-CBC encryption
  - HMAC-SHA512 for authentication
- **PRAGMA Settings:**
  ```sql
  PRAGMA key = '<encryption_key>';
  PRAGMA cipher_page_size = 4096;
  PRAGMA kdf_iter = 256000;
  PRAGMA cipher_hmac_algorithm = HMAC_SHA512;
  PRAGMA cipher_kdf_algorithm = PBKDF2_HMAC_SHA512;
  ```
- **Encryption Key:**
  - 256-bit key (32 bytes)
  - Generated cryptographically (QRandomGenerator::securelySeeded())
  - Stored securely (see REQ-SEC-ENC-004)
  - Not hardcoded in source code
- **Key Derivation:**
  - PBKDF2-HMAC-SHA512
  - 256,000 iterations (balance security vs performance)
  - Per-database salt
- **Performance Impact:**
  - < 10% overhead (acceptable for security)
  - Measured on target hardware
- **Verification:**
  - Database file unreadable without key
  - `hexdump` shows encrypted bytes (no plaintext)

**Related Requirements:**
- REQ-DATA-SEC-001 (database encryption)
- REQ-NFR-SEC-003 (encryption at rest)
- REQ-REG-HIPAA-001 (encryption mandate)
- REQ-SEC-ENC-004 (key management)

**Traces To:**
- Design: [06_SECURITY.md](../architecture_and_design/06_SECURITY.md) (Section 2)
- Design: [10_DATABASE_DESIGN.md](../architecture_and_design/10_DATABASE_DESIGN.md)
- Test: Test-SEC-ENC-003 (encryption verification)

---

### [REQ-SEC-ENC-004] Encryption Key Management

**Category:** Encryption  
**Priority:** Critical (Must Have)  
**Status:** Approved

**Description:**
The system shall securely generate, store, and manage encryption keys with protection against unauthorized access and key compromise.

**Rationale:**
Encryption is useless if keys are compromised. Secure key management is critical for maintaining confidentiality.

**Acceptance Criteria:**
- **Key Generation:**
  - Cryptographically secure random number generator (CSRNG)
  - Qt: QRandomGenerator::securelySeeded()
  - Linux: /dev/urandom (entropy source)
- **Key Storage:**
  - **Option 1 (Preferred):** Hardware Security Module (HSM) or TPM
  - **Option 2:** Encrypted file protected by hardware-derived key
  - **Option 3:** Derived from device-specific secret + user PIN
  - NOT stored in plaintext
  - NOT hardcoded in source
- **Key Access Control:**
  - Key accessible only to database manager process
  - Memory protection (mlock to prevent swapping)
  - Clear from memory after use
- **Key Rotation:**
  - Supported (annual or on compromise)
  - Re-encrypt database with new key
  - Old key retained for grace period
- **Key Backup:**
  - Escrow key backup (encrypted)
  - Recovery mechanism for lost keys
  - Administrator access (emergency)
- **Key Compromise Response:**
  - Immediate key rotation
  - Database re-encryption
  - Incident investigation

**Key Storage Options Comparison:**
| Option | Security | Availability | Complexity | Cost |
|--------|----------|--------------|------------|------|
| HSM/TPM | Highest | High | High | High |
| Encrypted File | Medium | High | Medium | Low |
| Derived Key | Medium-Low | High | Low | Low |

**Related Requirements:**
- REQ-SEC-ENC-003 (database encryption)
- REQ-DATA-SEC-001 (database encryption)

**Traces To:**
- Design: [06_SECURITY.md](../architecture_and_design/06_SECURITY.md) (Section 2.2)
- Design: [09_CLASS_DESIGNS.md](../architecture_and_design/09_CLASS_DESIGNS.md) (KeyManager - to be designed)
- Test: Test-SEC-ENC-004

**Notes:**
- HSM/TPM ideal but may not be available on all hardware
- Key derivation from device serial + PIN is fallback

---

## 6. Certificate Management Requirements

### [REQ-SEC-CERT-001] X.509 Certificate Standards

**Category:** Certificate Management  
**Priority:** Must Have  
**Status:** Approved

**Description:**
The system shall use X.509 certificates compliant with RFC 5280 for mTLS authentication with proper fields and extensions.

**Rationale:**
X.509 is industry standard for certificates. RFC 5280 compliance ensures interoperability and proper validation.

**Acceptance Criteria:**
- **Certificate Format:**
  - X.509 v3
  - PEM or DER encoding (PEM preferred for readability)
- **Key Algorithm:**
  - RSA 2048-bit or 4096-bit (preferred)
  - ECDSA P-256 or P-384 (alternative, smaller keys)
- **Signature Algorithm:**
  - SHA-256 with RSA (sha256WithRSAEncryption)
  - SHA-384 with ECDSA (ecdsa-with-SHA384)
  - NOT SHA-1 (deprecated, weak)
- **Certificate Fields:**
  - **Subject:** CN=ZM-ICU-MON-04, O=Hospital Name, C=US
  - **Issuer:** CN=Hospital CA, O=Hospital Name, C=US
  - **Serial Number:** Unique per certificate
  - **Validity Period:** Not Before, Not After (1 year typical)
- **Extensions:**
  - Key Usage: Digital Signature, Key Encipherment
  - Extended Key Usage: TLS Client Authentication, TLS Server Authentication
  - Subject Alternative Name (SAN): DNS:device.hospital.com (if applicable)
  - Authority Key Identifier
  - Subject Key Identifier
- **Certificate Chain:**
  - Device Certificate → Intermediate CA (optional) → Root CA
  - Root CA self-signed (trusted anchor)

**Example Certificate (PEM):**
```
-----BEGIN CERTIFICATE-----
MIIDXTCCAkWgAwIBAgIJAKZ...
[Base64-encoded certificate data]
...vXz9c8lW3Q==
-----END CERTIFICATE-----
```

**Related Requirements:**
- REQ-SEC-ENC-002 (mTLS)
- REQ-FUN-DEV-002 (certificate management)
- REQ-SEC-CERT-002 (certificate lifecycle)

**Traces To:**
- Design: [15_CERTIFICATE_PROVISIONING.md](../architecture_and_design/15_CERTIFICATE_PROVISIONING.md)
- Design: [06_SECURITY.md](../architecture_and_design/06_SECURITY.md) (Section 5)
- Test: Test-SEC-CERT-001

---

### [REQ-SEC-CERT-002] Certificate Lifecycle Management

**Category:** Certificate Management  
**Priority:** Must Have  
**Status:** Approved

**Description:**
The system shall manage complete certificate lifecycle including provisioning, validation, renewal, and revocation.

**Rationale:**
Certificates expire and may be compromised. Lifecycle management ensures continuous secure operation.

**Acceptance Criteria:**
- **Provisioning:**
  - Certificates installed during device provisioning
  - QR code workflow (REQ-FUN-DEV-001)
  - Automated provisioning service
- **Validation:**
  - Verify certificate chain to trusted CA
  - Check expiry (not before, not after)
  - Check revocation status (OCSP or CRL)
  - Verify hostname/device ID match
- **Renewal:**
  - Automatic renewal initiated 30 days before expiry
  - CSR (Certificate Signing Request) generated
  - Submitted to provisioning service
  - New certificate installed automatically
- **Expiry Monitoring:**
  - Check expiry on startup and daily
  - Warning: 30 days before expiry
  - Error: 7 days before expiry
  - Block connection: After expiry
- **Revocation:**
  - Support CRL (Certificate Revocation List) or OCSP
  - Check revocation before connection
  - Revoked certificate → Connection refused
- **Certificate Storage:**
  - Database: certificates table
  - Fields: serial, subject, issuer, not_before, not_after, status
  - Encrypted (SQLCipher)

**Certificate States:**
- ACTIVE: Valid and not expiring soon
- EXPIRING_SOON: < 30 days to expiry
- EXPIRED: Past not_after date
- REVOKED: On CRL or OCSP responder
- RENEWED: New certificate installed

**Related Requirements:**
- REQ-FUN-DEV-002 (certificate management)
- REQ-INT-PROV-003 (certificate renewal)
- REQ-SEC-CERT-001 (X.509 standards)

**Traces To:**
- Use Case: UC-DP-004, UC-DP-005
- Design: [15_CERTIFICATE_PROVISIONING.md](../architecture_and_design/15_CERTIFICATE_PROVISIONING.md)
- Design: [10_DATABASE_DESIGN.md](../architecture_and_design/10_DATABASE_DESIGN.md) (certificates table)
- Test: Test-SEC-CERT-002

---

### [REQ-SEC-CERT-003] Certificate Pinning

**Category:** Certificate Management  
**Priority:** Should Have  
**Status:** Approved

**Description:**
The system should support certificate pinning to protect against CA compromise and man-in-the-middle attacks using rogue certificates.

**Rationale:**
CA compromise (DigiNotar, Comodo breaches) can enable attackers to issue fraudulent certificates. Pinning prevents acceptance of unauthorized certificates.

**Acceptance Criteria:**
- **Pinning Types:**
  - **Certificate Pinning:** Pin specific certificate (more rigid)
  - **Public Key Pinning:** Pin public key (allows cert renewal with same key)
- **Implementation:**
  - Pin server certificate public key hash (SHA-256)
  - Store pin in configuration or database
  - Verify pin during TLS handshake
- **Pin Update:**
  - Pin included in provisioning payload
  - Pin update via secure channel
  - Support multiple pins (backup)
- **Pin Failure:**
  - Pin mismatch → Connection refused
  - Log security event (possible attack)
  - Alert administrator
- **Backup Pins:**
  - Primary pin + backup pin (in case of rotation)
  - Accept connection if either matches

**Example (Qt):**
```cpp
QSslConfiguration config;
QList<QSslCertificate> expectedCerts = loadPinnedCerts();
config.setCaCertificates(expectedCerts);
```

**Related Requirements:**
- REQ-SEC-ENC-002 (mTLS)
- REQ-SEC-CERT-002 (lifecycle)
- REQ-NFR-SEC-002 (encryption in transit)

**Traces To:**
- Design: [06_SECURITY.md](../architecture_and_design/06_SECURITY.md) (Section 6.5)
- Design: [09_CLASS_DESIGNS.md](../architecture_and_design/09_CLASS_DESIGNS.md) (NetworkManager)
- Test: Test-SEC-CERT-003

**Notes:**
- RFC 7469 - Public Key Pinning Extension for HTTP (HPKP)
- Deprecated for web (dynamic pins), but useful for IoT/medical devices (static pins)

---

## 7. Audit and Logging Requirements

### [REQ-SEC-AUDIT-001] Comprehensive Security Audit Logging

**Category:** Audit and Logging  
**Priority:** Must Have  
**Status:** Approved

**Description:**
The system shall log all security-relevant events to a tamper-evident audit log for compliance, investigation, and incident response.

**Rationale:**
HIPAA requires audit trails. Security incidents require forensic investigation. Comprehensive logging enables detection and accountability.

**Acceptance Criteria:**
- **Events Logged:**
  - Authentication (login, logout, failed attempts)
  - Authorization (permission grants, denials)
  - Data access (patient admission, vitals view, trends)
  - Data modification (patient update, settings change)
  - Data export/disclosure
  - Configuration changes
  - Certificate operations (provisioning, renewal, revocation)
  - Network connections (server, provisioning)
  - Security violations (brute force, unauthorized access)
  - Alarm events (triggered, acknowledged, escalated)
- **Log Fields (Minimum):**
  - Timestamp (Unix milliseconds, UTC)
  - Event type (LOGIN, LOGOUT, PATIENT_ADMIT, etc.)
  - Severity (INFO, WARNING, ERROR, CRITICAL)
  - User ID (or SYSTEM)
  - Patient MRN (if applicable)
  - Action description
  - Outcome (SUCCESS, FAILURE)
  - Device ID
  - IP address (if networked)
  - Additional context (JSON object)
- **Log Storage:**
  - Database: security_audit_log table
  - Append-only (no updates or deletes)
  - Encrypted (SQLCipher)
- **Tamper Evidence:**
  - Hash chain: Each log entry includes hash of previous entry
  - Integrity verification: Detect tampering
  - Signature: Optional digital signature for log entries
- **Log Rotation:**
  - Local retention: 90 days
  - Central archival: Sync to central server immediately
  - Central retention: 6 years (HIPAA)

**Log Entry Example:**
```json
{
  "timestamp": 1732718400000,
  "eventType": "PATIENT_ADMIT",
  "severity": "INFO",
  "userId": "NURSE001",
  "patientMrn": "MRN-12345",
  "action": "Patient John Doe admitted to device ZM-ICU-MON-04",
  "outcome": "SUCCESS",
  "deviceId": "ZM-ICU-MON-04",
  "ipAddress": "10.1.50.104",
  "context": {
    "admissionMethod": "MANUAL_MRN_ENTRY",
    "lookupSource": "HIS"
  }
}
```

**Related Requirements:**
- REQ-NFR-SEC-010 (audit logging)
- REQ-REG-HIPAA-003 (audit trail)
- REQ-DATA-RET-003 (audit log retention)

**Traces To:**
- Design: [21_LOGGING_STRATEGY.md](../architecture_and_design/21_LOGGING_STRATEGY.md)
- Design: [10_DATABASE_DESIGN.md](../architecture_and_design/10_DATABASE_DESIGN.md) (security_audit_log)
- Test: Test-SEC-AUDIT-001

---

### [REQ-SEC-AUDIT-002] Audit Log Protection

**Category:** Audit and Logging  
**Priority:** Must Have  
**Status:** Approved

**Description:**
The system shall protect audit logs from unauthorized access, modification, and deletion to maintain log integrity and evidentiary value.

**Rationale:**
Audit logs are legal evidence. Tampered logs are inadmissible. Protection ensures integrity and accountability.

**Acceptance Criteria:**
- **Access Control:**
  - Only administrators can view audit logs
  - No user can modify or delete log entries
  - Database constraints: Append-only table
- **Encryption:**
  - Logs encrypted at rest (SQLCipher)
  - Logs encrypted in transit (TLS)
- **Integrity Verification:**
  - Hash chain: Each entry contains hash of previous
  - Verify chain on startup and on-demand
  - Detect: Missing entries, modified entries, reordered entries
- **Immutability:**
  - SQL: No UPDATE or DELETE permissions on audit log table
  - Application: No APIs for log modification
  - Exceptions: Purge after retention period (automatic, logged)
- **Backup:**
  - Audit logs included in database backup
  - Sync to central server immediately (redundancy)
- **Alerting:**
  - Integrity violation → Alert administrator immediately
  - Failed sync → Alert (logs at risk)

**Related Requirements:**
- REQ-SEC-AUDIT-001 (comprehensive logging)
- REQ-DATA-SEC-001 (encryption)

**Traces To:**
- Design: [21_LOGGING_STRATEGY.md](../architecture_and_design/21_LOGGING_STRATEGY.md)
- Design: [10_DATABASE_DESIGN.md](../architecture_and_design/10_DATABASE_DESIGN.md)
- Test: Test-SEC-AUDIT-002 (tampering detection)

---

## 8. Network Security Requirements

### [REQ-SEC-NET-001] Network Protocol Restrictions

**Category:** Network Security  
**Priority:** Must Have  
**Status:** Approved

**Description:**
The system shall only use secure protocols (HTTPS, TLS) and block insecure protocols (HTTP, FTP, Telnet) to prevent cleartext PHI transmission.

**Rationale:**
Insecure protocols transmit data in cleartext. Prevents eavesdropping and man-in-the-middle attacks.

**Acceptance Criteria:**
- **Allowed Protocols:**
  - HTTPS (HTTP over TLS)
  - mTLS (mutual TLS)
  - NTP (time sync - authenticated if possible)
- **Blocked Protocols:**
  - HTTP (plaintext)
  - FTP (plaintext)
  - Telnet (plaintext)
  - SMTP (unless STARTTLS)
- **Protocol Enforcement:**
  - No HTTP fallback (HTTPS only)
  - URL validation: Reject non-HTTPS URLs
  - Port restrictions: Only HTTPS (443), NTP (123)
- **Development Exception:**
  - Development/testing: HTTP allowed on localhost only
  - Production: HTTP strictly prohibited

**Related Requirements:**
- REQ-SEC-ENC-001 (TLS)
- REQ-SEC-ENC-002 (mTLS)

**Traces To:**
- Design: [06_SECURITY.md](../architecture_and_design/06_SECURITY.md) (Section 6)
- Design: [09_CLASS_DESIGNS.md](../architecture_and_design/09_CLASS_DESIGNS.md) (NetworkManager)
- Test: Test-SEC-NET-001

---

### [REQ-SEC-NET-002] Network Firewall Configuration

**Category:** Network Security  
**Priority:** Should Have  
**Status:** Approved

**Description:**
The system should support firewall configuration to restrict network access to only required services and block unauthorized connections.

**Rationale:**
Defense in depth. Firewall reduces attack surface by blocking unused ports and services.

**Acceptance Criteria:**
- **Inbound Rules:**
  - Allow: HTTPS (443) from provisioning service (during provisioning only)
  - Allow: NTP (123) from NTP server
  - Block: All other inbound connections
- **Outbound Rules:**
  - Allow: HTTPS (443) to telemetry server
  - Allow: HTTPS (443) to HIS API
  - Allow: NTP (123) to NTP server
  - Block: All other outbound connections
- **Firewall Tool:**
  - Linux: iptables or nftables
  - Configuration file provided
- **Default Deny:**
  - Firewall default policy: DENY
  - Explicit allow rules for required traffic
- **Logging:**
  - Log blocked connections (anomaly detection)
  - Alert on repeated connection attempts

**Firewall Rules Example (iptables):**
```bash
# Default deny
iptables -P INPUT DROP
iptables -P OUTPUT DROP

# Allow established connections
iptables -A INPUT -m state --state ESTABLISHED,RELATED -j ACCEPT
iptables -A OUTPUT -m state --state ESTABLISHED,RELATED -j ACCEPT

# Allow HTTPS outbound to telemetry server
iptables -A OUTPUT -p tcp --dport 443 -d 10.1.1.100 -j ACCEPT

# Allow NTP outbound
iptables -A OUTPUT -p udp --dport 123 -j ACCEPT
```

**Related Requirements:**
- REQ-REG-62443-001 (network security)
- REQ-SEC-NET-001 (protocol restrictions)

**Traces To:**
- Design: [06_SECURITY.md](../architecture_and_design/06_SECURITY.md) (Section 6.6)
- Process: Firewall configuration guide
- Test: Test-SEC-NET-002

**Notes:**
- Hospital network may have centralized firewall
- Device-level firewall adds defense in depth

---

## 9. Physical Security Requirements

### [REQ-SEC-PHYS-001] Secure Boot (Optional)

**Category:** Physical Security  
**Priority:** Nice to Have  
**Status:** Approved

**Description:**
The system should optionally support secure boot to prevent execution of unauthorized firmware or operating system modifications.

**Rationale:**
Physical access enables firmware tampering. Secure boot verifies firmware integrity before execution.

**Acceptance Criteria:**
- **Boot Process:**
  - UEFI Secure Boot or U-Boot verified boot
  - Bootloader verifies kernel signature
  - Kernel verifies init system signature
- **Key Management:**
  - Public keys stored in hardware (TPM, efuse)
  - Private keys held by device manufacturer
- **Tampering Detection:**
  - Verification failure → Boot halted
  - Alert displayed: "Firmware integrity check failed"
  - Log security event
- **Recovery:**
  - Secure recovery mode
  - Re-flash verified firmware

**Related Requirements:**
- REQ-SEC-PHYS-002 (tamper detection)
- REQ-REG-62443-001 (system integrity)

**Traces To:**
- Design: [06_SECURITY.md](../architecture_and_design/06_SECURITY.md) (Section 7)
- Test: Test-SEC-PHYS-001

**Notes:**
- Hardware-dependent (Raspberry Pi, x86)
- Raspberry Pi: Enable USB boot lock, GPIO boot lock

---

### [REQ-SEC-PHYS-002] Tamper Detection

**Category:** Physical Security  
**Priority:** Should Have  
**Status:** Approved

**Description:**
The system should detect physical tampering (enclosure opened, storage removed) and log security events.

**Rationale:**
Physical access enables attacks. Tamper detection enables incident response and forensics.

**Acceptance Criteria:**
- **Detection Methods:**
  - Enclosure switch (detects case opening)
  - Storage presence detection (SD card removed)
  - Boot integrity check (secure boot)
- **Response:**
  - Log security event: "Tamper detected"
  - Alert administrator (if networked)
  - Optionally: Zeroize encryption keys (extreme)
- **Logging:**
  - Timestamp of tamper event
  - Type of tamper (enclosure, storage, boot)
  - Persistent log (survives reboot)

**Related Requirements:**
- REQ-SEC-PHYS-001 (secure boot)
- REQ-SEC-AUDIT-001 (audit logging)

**Traces To:**
- Design: [06_SECURITY.md](../architecture_and_design/06_SECURITY.md) (Section 7)
- Test: Test-SEC-PHYS-002

**Notes:**
- Requires hardware support (tamper switches)
- Not all platforms support

---

## 10. Incident Response Requirements

### [REQ-SEC-INC-001] Security Incident Classification

**Category:** Incident Response  
**Priority:** Must Have  
**Status:** Approved

**Description:**
The system shall classify security incidents by severity to enable appropriate response procedures and HIPAA breach notification if required.

**Rationale:**
Different incidents require different responses. Classification enables prioritization and regulatory compliance (HIPAA breach notification).

**Acceptance Criteria:**
- **Severity Levels:**
  - **CRITICAL:** Confirmed PHI breach, active attack, system compromise
  - **HIGH:** Potential PHI breach, repeated unauthorized access attempts
  - **MEDIUM:** Security policy violation, misconfiguration
  - **LOW:** Informational, suspicious activity
- **Incident Types:**
  - **Breach:** Unauthorized PHI access, theft, disclosure
  - **Attack:** Brute force, network intrusion, malware
  - **Compromise:** System takeover, data tampering
  - **Misconfiguration:** Weak passwords, open ports
  - **Insider Threat:** Unauthorized data export, snooping
- **Response Procedures:**
  - CRITICAL: Immediate response, isolation, forensics, breach notification
  - HIGH: Investigate within 24 hours, mitigate, report to security team
  - MEDIUM: Investigate, remediate, document
  - LOW: Log, monitor, periodic review
- **Escalation:**
  - Administrator notified for HIGH/CRITICAL
  - Hospital security/CISO notified for CRITICAL
  - Law enforcement notified if criminal activity

**Related Requirements:**
- REQ-REG-HIPAA-005 (breach notification)
- REQ-SEC-AUDIT-001 (logging)

**Traces To:**
- Design: [06_SECURITY.md](../architecture_and_design/06_SECURITY.md) (Section 8)
- Process: Incident response plan (to be documented)
- Test: Test-SEC-INC-001 (incident simulation)

---

## 11. Security Requirements Summary

### Total Requirements: 21 (of ~30-40 planned)

| Category | Requirements | Critical | Must Have | Should Have | Nice to Have |
|----------|--------------|----------|-----------|-------------|--------------|
| Authentication | 4 | 1 | 2 | 1 | 0 |
| Authorization | 2 | 0 | 2 | 0 | 0 |
| Encryption | 4 | 3 | 0 | 1 | 0 |
| Certificate Mgmt | 3 | 0 | 2 | 1 | 0 |
| Audit & Logging | 2 | 0 | 2 | 0 | 0 |
| Network Security | 2 | 0 | 1 | 1 | 0 |
| Physical Security | 2 | 0 | 0 | 1 | 1 |
| Incident Response | 1 | 0 | 1 | 0 | 0 |
| **Total** | **20** | **4** | **10** | **5** | **1** |

### Remaining Requirements (to be added):
- ~5 additional encryption requirements (algorithms, key sizes, protocols)
- ~5 certificate management requirements (CRL, OCSP, trust store)
- ~5 network security requirements (IDS, rate limiting, DDoS protection)
- ~5 incident response requirements (forensics, recovery, lessons learned)

---

## 12. Security Testing Requirements

All security requirements must be verified through:
- **Static Analysis:** Code review for security vulnerabilities
- **Dynamic Analysis:** Penetration testing, fuzzing
- **Compliance Testing:** Verify cryptographic implementations
- **Regression Testing:** Security tests in CI/CD

---

## 13. Related Documents

- **Security Design:** [../architecture_and_design/06_SECURITY.md](../architecture_and_design/06_SECURITY.md)
- **Regulatory Requirements:** [07_REGULATORY_REQUIREMENTS.md](./07_REGULATORY_REQUIREMENTS.md)
- **Certificate Provisioning:** [../architecture_and_design/15_CERTIFICATE_PROVISIONING.md](../architecture_and_design/15_CERTIFICATE_PROVISIONING.md)
- **Logging Strategy:** [../architecture_and_design/21_LOGGING_STRATEGY.md](../architecture_and_design/21_LOGGING_STRATEGY.md)

---

*Security is not optional for medical devices handling PHI. These requirements ensure patient privacy, data integrity, and regulatory compliance.*

