# Security Design

This document details the security architecture for the Z Monitor, covering data in transit, data at rest, and authentication.

## 1. Guiding Principles

-   **Defense in Depth:** Employ multiple layers of security controls.
-   **Principle of Least Privilege:** Components and users should only have the access required to perform their functions.
-   **Secure by Default:** The default configuration of the system must be secure.

## 2. Encryption in Transit

-   **Requirement:** All communication between the device and the central server must be encrypted.
-   **Implementation:**
    -   **Protocol:** TLS 1.2 or higher.
    -   **Endpoint:** All API endpoints must be accessed via `https://`.
    -   **C++ Client:** The `NetworkManager` class will use `QNetworkAccessManager` and a properly configured `QSslConfiguration` to enforce TLS.
    -   **Server:** The simulated central server (Python/Flask) will be configured to only serve content over HTTPS.

## 3. Authentication

### 3.1. Device-to-Server Authentication

-   **Requirement:** The device and server must authenticate each other to prevent unauthorized devices from connecting and to ensure the device is connecting to a legitimate server.
-   **Implementation: Mutual TLS (mTLS)**
    -   **Device Side:** Each device will be provisioned with a unique client certificate (`.pem`) and a corresponding private key (`.key`). The `NetworkManager` will load these into its `QSslConfiguration`. The device will also have a copy of the CA certificate to verify the server's identity.
    -   **Server Side:** The server will have its own server certificate and private key. It will also be configured with the CA certificate and will be set to require and verify client certificates from incoming connections. Any connection without a valid, trusted client certificate will be rejected at the TLS handshake level.

### 3.2. User-to-Device Authentication

-   **Requirement:** Only authorized clinicians and technicians can access the device's functions.
-   **Implementation: PIN-based Login & Roles**
    -   **Login Screen:** A PIN entry screen is presented on startup.
    -   **Roles:**
        -   **Clinician:** Can access monitoring, alarm management, and basic settings.
        -   **Technician:** Has all Clinician privileges, plus access to advanced settings like device calibration and diagnostics.
    -   **Backend:** The `AuthenticationService` in C++ is responsible for verifying the PIN and managing the active user's role and session.
    
    **PIN Storage and Security:**
    -   **Hashing:** PINs are hashed using SHA-256 with a per-user salt (stored in `users` table)
    -   **PIN Complexity:** Minimum 4 digits, maximum 8 digits (configurable via settings)
    -   **Storage:** Hashed PINs stored in `users.pin_hash` column, salts stored in `users.salt` column
    -   **Never store plaintext PINs** in database or logs
    
    **Brute Force Protection:**
    -   **Lockout Policy:** 5 failed login attempts → 15-minute account lockout
    -   **Exponential Backoff:** Lockout duration doubles after each violation (15 min → 30 min → 60 min)
    -   **Permanent Lockout:** After 10 failed attempts, account requires administrator unlock
    -   **Audit Logging:** All failed login attempts logged to `security_audit_log` with severity "warning"
    -   **Rate Limiting:** Maximum 1 login attempt per 2 seconds per IP/device
    
    **Session Management:**
    -   **Session Timeout:** 30 minutes of inactivity (configurable via settings, range: 15-60 minutes)
    -   **Session Refresh:** Automatic refresh on any user activity, manual refresh available via UI
    -   **Concurrent Sessions:** Single active session per user (new login invalidates previous session)
    -   **Session Tokens:** Cryptographically secure session tokens (UUID v4 + timestamp + HMAC)
    -   **Session Storage:** Active sessions stored in memory, session tokens validated on each request
    -   **Logout:** Explicit logout clears session, automatic logout on timeout
    -   **Session Audit:** All login/logout events logged to `security_audit_log`

## 4. Encryption at Rest

-   **Requirement:** All sensitive patient data stored locally on the device must be encrypted.
-   **Implementation: Encrypted SQLite Database**
    -   **Technology:** The project will use **SQLCipher**, a widely-used open-source extension to SQLite that provides transparent 256-bit AES encryption of database files.
    -   **Backend:** The `DatabaseManager` class will be responsible for managing the connection to the encrypted database, including handling the encryption key.
    -   **Key Management:** The encryption key for the database must be stored securely, for instance, in a hardware-backed keystore if available on the target device. For simulation, it can be a hardcoded value or stored in a configuration file with restricted permissions.
    
    **Key Storage:**
    -   **Development:** Encryption key stored in `resources/config/db.key` with file permissions 600 (read/write owner only)
    -   **Production:** Hardware-backed keystore (HSM) or secure element preferred, fallback to OS keystore
    -   **Key File:** Never commit key files to version control (added to `.gitignore`)
    -   **Key Derivation:** For development, key can be derived using PBKDF2 with 100,000 iterations and device-specific salt
    
    **Key Rotation:**
    -   **Process:** Key rotation requires re-encrypting entire database
    -   **Procedure:**
        1. Generate new encryption key
        2. Create new encrypted database with new key
        3. Copy all data from old database to new database (decrypt with old key, encrypt with new key)
        4. Verify data integrity
        5. Replace old database with new database
        6. Securely erase old database and old key
    -   **Frequency:** Key rotation recommended annually or after security incidents
    -   **Backup:** Backup database before key rotation

## 5. Certificate & Key Provisioning

-   **Requirement:** A secure process for installing and managing cryptographic materials on the device.
-   **Detailed Guide:** See [Certificate Provisioning Guide](./15_CERTIFICATE_PROVISIONING.md) for comprehensive step-by-step instructions, workflow diagrams, and troubleshooting.
-   **Process Overview:**
    1.  A Certificate Authority (CA) is created (self-signed for this project).
    2.  The CA is used to sign a unique certificate for the server and a unique certificate for each device.
    3.  **Device:** The device's client certificate, its private key, and the CA certificate are securely installed in a protected area of the device's filesystem during "manufacturing".
    4.  **Server:** The server's certificate, its private key, and the CA certificate are installed on the server.
-   **Workflow Diagrams:** See [Certificate Provisioning Guide](./15_CERTIFICATE_PROVISIONING.md) for detailed sequence diagrams (SVG rendered):
    - [Initial Certificate Provisioning](./15_CERTIFICATE_PROVISIONING_INITIAL.svg): CA setup → device installation
    - [Certificate Renewal](./15_CERTIFICATE_PROVISIONING_RENEWAL.svg): Zero-downtime renewal process
    - [Certificate Revocation](./15_CERTIFICATE_PROVISIONING_REVOCATION.svg): CRL distribution and enforcement
    - [Certificate Validation](./15_CERTIFICATE_PROVISIONING_VALIDATION.svg): Device startup validation process
    - [Secure Data Transmission](./15_CERTIFICATE_PROVISIONING_TRANSMISSION.svg): End-to-end secure transmission flow

## 6. Secure Data Transmission Architecture

### 6.1. Multi-Layer Security for Telemetry and Sensor Data

The Z Monitor implements a comprehensive security architecture for transmitting telemetry and sensor data to the central server:

#### Layer 1: Transport Security (mTLS)
- **Mutual TLS (mTLS)** provides bidirectional authentication and encryption
- **TLS Version:** TLS 1.2 minimum, TLS 1.3 preferred
- **Cipher Suites:** Strong, modern cipher suites only (e.g., ECDHE-RSA-AES256-GCM-SHA384)
- **Certificate Validation:**
  - Server certificate must be signed by trusted CA
  - Client certificate must be signed by trusted CA
  - Certificate chain validation
  - Certificate expiration checking
  - Certificate revocation list (CRL) checking (**mandatory for production**, optional for development/testing)

#### Layer 2: Application-Level Authentication
- **Device Identity:** Each device has a unique client certificate with device ID embedded in Subject Alternative Name (SAN)
- **Certificate Binding:** Device certificate is cryptographically bound to device ID stored in settings
- **Token-Based Auth (Optional):** For additional security, devices can obtain short-lived JWT tokens after mTLS handshake
  - Token obtained via `/api/auth/device-token` endpoint after successful mTLS
  - Token included in `Authorization: Bearer <token>` header for subsequent requests
  - Token expiration: 1 hour (configurable)
  - Token refresh: Automatic before expiration

#### Layer 3: Data Integrity
- **Message Signing:** Each telemetry payload includes a digital signature
  - Signature algorithm: ECDSA with P-256 curve or RSA-2048
  - Signature computed over: deviceId + timestamp + payload hash
  - Signature included in request header: `X-Device-Signature`
- **Timestamp Validation:** Server validates timestamps to prevent replay attacks
  - Clock skew tolerance: ±1 minute for production, ±5 minutes for development/testing
  - Replay window: 1 minute (reject duplicate timestamps within window)
  - **Clock Synchronization:** NTP synchronization required in production to maintain accurate timestamps

#### Layer 4: Payload Encryption (Optional, for Extra Sensitive Data)
- **Field-Level Encryption:** Critical PHI fields can be encrypted within the payload
  - Encryption algorithm: AES-256-GCM
  - Key derivation: From device certificate and server public key
  - Only specific fields encrypted (e.g., patient identifiers in certain scenarios)

### 6.2. Certificate Management

#### Certificate Storage
- **Location:** `resources/certs/` directory with restricted file permissions (600)
- **Files:**
  - `client.crt`: Device client certificate (PEM format)
  - `client.key`: Device private key (PEM format, encrypted with passphrase in production)
  - `ca.crt`: Certificate Authority certificate for server verification
  - `crl.pem`: Certificate Revocation List (optional, updated periodically)

#### Certificate Lifecycle
- **Provisioning:** Certificates installed during device manufacturing/provisioning
- **Validation:** On startup, device validates certificate:
  - Not expired
  - Not revoked (CRL check)
  - Valid signature chain
  - Matches device ID
- **Renewal:** Certificates can be renewed before expiration
  - Renewal process: Request new certificate from CA via secure channel
  - Old certificate remains valid until new one is installed
  - Seamless transition without service interruption
- **Revocation:** If device is compromised, certificate can be revoked
  - Revocation propagated via CRL
  - Device will fail to connect after revocation

#### Certificate Database Tracking
- Certificates are tracked in `certificates` table:
  - Certificate serial number
  - Device ID
  - Issuance date
  - Expiration date
  - Status (active, expired, revoked)
  - Last validation timestamp

### 6.3. Network Security Configuration

#### Connection Security
- **HTTPS Only:** All server communication must use HTTPS (no HTTP)
- **Certificate Pinning (Optional):** For additional security, device can pin server certificate
  - Server certificate fingerprint stored in device
  - Connection fails if server certificate doesn't match pinned fingerprint
  - Useful for preventing MITM attacks even if CA is compromised

#### Rate Limiting
- **Client-Side:** Device implements rate limiting to prevent abuse
  - Maximum requests per minute: 60 (configurable)
  - Exponential backoff on failures
- **Server-Side:** Server enforces rate limiting per device
  - Prevents DoS attacks
  - Returns 429 (Too Many Requests) when limit exceeded

#### Connection Retry Logic
- **Exponential Backoff:** Retry delays increase exponentially on failures
  - Initial delay: 1 second
  - Maximum delay: 60 seconds
  - Maximum retries: 5
- **Circuit Breaker:** After repeated failures, device enters "circuit open" state
  - Stops attempting connections for a period
  - Prevents resource exhaustion
  - Automatically attempts reconnection after cooldown period

### 6.4. Security Audit Logging

All security-relevant events are logged for audit and forensics:

- **Connection Events:**
  - Successful mTLS handshake
  - Failed authentication attempts
  - Certificate validation failures
  - Connection timeouts
- **Data Transmission Events:**
  - Successful telemetry transmission
  - Failed transmission attempts
  - Payload size violations
  - Rate limit violations
- **Certificate Events:**
  - Certificate validation
  - Certificate expiration warnings
  - Certificate renewal attempts
  - Certificate revocation detection

Audit logs stored in `audit_log` table with:
- Timestamp
- Event type
- Device ID
- User ID (if applicable)
- Success/failure status
- Error details (if failure)

### 6.5. Secure Key Storage

#### Private Key Protection
- **Encryption at Rest:** Private keys stored encrypted on device filesystem
  - Encryption: AES-256
  - Key derivation: From device hardware ID or secure element
- **Hardware Security Module (HSM):** For production devices with HSM support
  - Private keys never leave HSM
  - Cryptographic operations performed in HSM
  - Prevents key extraction even if device is compromised
- **Memory Protection:** Private keys loaded into memory only when needed
  - Memory cleared after use
  - Secure memory allocation (mlock/mprotect on Linux)

#### Key Rotation
- **Automatic Rotation:** Keys can be rotated without service interruption
- **Process:**
  1. Generate new key pair
  2. Request new certificate from CA
  3. Install new certificate alongside old one
  4. Use new certificate for new connections
  5. Old certificate remains valid until expiration
  6. Remove old certificate after expiration

### 6.6. Implementation in NetworkManager

The `NetworkManager` class implements the security architecture:

```cpp
class NetworkManager {
    // Certificate management
    QSslCertificate clientCertificate;
    QSslKey clientPrivateKey;
    QSslCertificate caCertificate;
    
    // Security configuration
    void ConfigureSecurity() {
        QSslConfiguration sslConfig = QSslConfiguration::defaultConfiguration();
        
        // Set TLS version
        sslConfig.setProtocol(QSsl::TlsV1_2OrLater);
        
        // Load certificates
        sslConfig.setLocalCertificate(clientCertificate);
        sslConfig.setPrivateKey(clientPrivateKey);
        sslConfig.setCaCertificates({caCertificate});
        
        // Require client certificate (mTLS)
        sslConfig.setPeerVerifyMode(QSslSocket::VerifyPeer);
        sslConfig.setPeerVerifyDepth(2);
        
        // Set strong cipher suites
        sslConfig.setCiphers(strongCipherSuites);
        
        // Apply configuration
        QNetworkAccessManager::setSslConfiguration(sslConfig);
    }
    
    // Validate certificate before connection
    bool ValidateCertificate() {
        // Check expiration
        if (clientCertificate.expiryDate() < QDateTime::currentDateTime()) {
            return false;
        }
        
        // Check device ID match
        QString deviceId = GetDeviceIdFromCertificate();
        if (deviceId != settingsManager->GetValue("deviceId").toString()) {
            return false;
        }
        
        // Check CRL (if available)
        if (!CheckCertificateRevocation()) {
            return false;
        }
        
        return true;
    }
};
```

### 6.7. Server-Side Security

The central server implements complementary security measures:

- **mTLS Enforcement:** Server requires and validates client certificates
- **Device Registration:** Only registered devices (certificate serial numbers) are allowed
- **Request Validation:**
  - Verify device signature
  - Validate timestamp (prevent replay)
  - Check rate limits
  - Validate payload structure
- **Response Security:**
  - Responses signed by server
  - Sensitive data in responses encrypted
  - Include server timestamp for clock sync

### 6.8. Security Testing

Comprehensive security testing includes:

- **Certificate Validation Tests:**
  - Expired certificate rejection
  - Revoked certificate rejection
  - Invalid signature rejection
  - Wrong device ID rejection
- **Connection Security Tests:**
  - MITM attack prevention
  - Replay attack prevention
  - Rate limiting enforcement
  - Certificate pinning validation
- **Data Integrity Tests:**
  - Signature validation
  - Payload tampering detection
  - Timestamp validation
- **Penetration Testing:**
  - Network traffic analysis
  - Certificate extraction attempts
  - Key extraction attempts
  - DoS attack resistance

## 7. Secure Boot and Firmware Integrity

### 7.1. Secure Boot Process

-   **Requirement:** Device must verify firmware integrity before execution to prevent tampering and ensure only authorized firmware runs.
-   **Implementation:**
    -   **Bootloader Verification:** Bootloader verifies firmware signature using device's public key
    -   **Firmware Signature:** Firmware signed with manufacturer's private key during build process
    -   **Signature Validation:** SHA-256 hash of firmware verified against signature
    -   **Boot Failure:** If verification fails, device enters safe mode and alerts administrator

### 7.2. Firmware Integrity Checks

-   **On Boot:** Complete firmware integrity check using SHA-256 checksums
-   **Runtime Checks:** Periodic integrity checks of critical firmware components
-   **Checksum Storage:** Firmware checksums stored in secure, read-only storage
-   **Tamper Detection:** Hardware tamper detection (if available) triggers immediate security event

### 7.3. Firmware Update Security

-   **Update Verification:** All firmware updates must be digitally signed
-   **Update Process:**
    1. Download firmware update package
    2. Verify digital signature of update package
    3. Verify checksum of update package
    4. Install update in isolated partition
    5. Verify installed firmware integrity
    6. Switch to new firmware partition
    7. Rollback on verification failure
-   **Rollback Capability:** Automatic rollback to previous firmware version on update failure
-   **Update Audit:** All firmware updates logged to `security_audit_log`

## 8. Tamper Detection and Response

### 8.1. Physical Tamper Detection

-   **Hardware Tamper Switches:** If available, hardware tamper switches detect physical access
-   **Enclosure Monitoring:** Sensors detect unauthorized enclosure opening
-   **Response:** Immediate security event, device lockout, alert to central server

### 8.2. Software Tamper Detection

-   **Certificate Validation:** Continuous validation of device certificates
-   **File Integrity:** Periodic checksum verification of critical system files
-   **Configuration Validation:** Validation of configuration files against known good state
-   **Anomaly Detection:** Detection of unexpected system behavior or configuration changes

### 8.3. Tamper Response

-   **Immediate Actions:**
    - Log security event to `security_audit_log` with severity "critical"
    - Lock device (require administrator unlock)
    - Alert central server immediately
    - Disable network connectivity (optional, configurable)
-   **Recovery:** Tamper lockout requires administrator intervention and security review

## 9. Secure Erase and Decommissioning

### 9.1. Secure Erase Procedure

-   **Requirement:** All patient data and cryptographic materials must be securely erased before device decommissioning.
-   **Process:**
    1. Revoke device certificate (add to CRL)
    2. Securely erase database (multi-pass overwrite with random data)
    3. Securely erase certificates and private keys
    4. Securely erase configuration files
    5. Verify erasure (read back to confirm overwrite)
    6. Log decommissioning event to `security_audit_log`
-   **Erasure Standard:** NIST 800-88 compliant (3-pass overwrite minimum)
-   **Verification:** Cryptographic verification of successful erasure

### 9.2. Decommissioning Audit

-   **Complete Audit Trail:** All decommissioning steps logged with timestamps
-   **Certificate Revocation:** Device certificate revoked and added to CRL
-   **Server Notification:** Central server notified of device decommissioning
-   **Documentation:** Decommissioning certificate generated for compliance

## 10. Incident Response Procedures

### 10.1. Security Incident Classification

-   **Critical:** Immediate threat to patient safety or data breach (e.g., tamper detection, certificate compromise)
-   **High:** Significant security violation (e.g., unauthorized access, failed authentication after lockout)
-   **Medium:** Suspicious activity (e.g., multiple failed login attempts, unusual network activity)
-   **Low:** Informational security events (e.g., certificate expiration warnings, rate limit warnings)

### 10.2. Incident Response Steps

1. **Detection:** Automatic detection via security audit logging and monitoring
2. **Classification:** Automatic classification based on event type and severity
3. **Notification:** Automatic notification to security team for Critical/High incidents
4. **Containment:** Automatic containment actions (device lockout, network disconnect) for Critical incidents
5. **Investigation:** Security team investigates incident using audit logs
6. **Remediation:** Remediation steps based on incident type
7. **Documentation:** Complete incident report logged to `security_audit_log`
8. **Recovery:** Recovery procedures to restore normal operation

### 10.3. Incident Logging

-   **All Incidents:** Logged to `security_audit_log` with full details
-   **Incident Reports:** Detailed reports available for compliance and forensics
-   **Retention:** Incident logs retained for 90 days minimum (configurable)
