# Device Provisioning and Pairing

This document describes the secure device provisioning and pairing workflow for the Z Monitor, replacing manual network configuration with an industry-standard QR code-based pairing system.

## 1. Overview

The Z Monitor uses a secure provisioning workflow that eliminates manual certificate and server URL configuration. Instead, devices are provisioned through a QR code-based pairing process with the Central Station.

**Key Benefits:**
- **Security:** Eliminates manual certificate handling and reduces human error
- **Efficiency:** Faster device setup with QR code scanning
- **Auditability:** All provisioning events logged for compliance
- **Scalability:** Supports batch provisioning of multiple devices
- **User Experience:** Intuitive workflow for technicians

**Improvements Over Manual Configuration:**
- Time-limited pairing codes prevent unauthorized access
- Encrypted and signed configuration payloads ensure integrity
- One-time use pairing codes prevent replay attacks
- Explicit provisioning mode activation (security by design)
- Comprehensive error handling and recovery procedures

## 2. Provisioning Workflow

### 2.1. Workflow Steps

1. **Enable Provisioning Mode:** Device enters provisioning mode (explicit action required for security)
2. **Generate Pairing Information:** Device generates QR code and pairing code
3. **Scan/Enter Pairing Code:** Technician scans QR code or enters pairing code on authorized tablet
4. **Central Station Validation:** Central Station validates device and generates secure configuration
5. **Push Configuration:** Central Station securely pushes configuration to device
6. **Device Validation:** Device validates and applies configuration
7. **Connection Test:** Device tests connection with new configuration
8. **Provisioning Complete:** Device exits provisioning mode and connects to server

### 2.2. Security Features

- **Time-Limited Pairing:** QR code and pairing code expire after 10 minutes (configurable)
- **One-Time Use:** Pairing codes can only be used once
- **Encrypted Configuration:** Configuration payload encrypted with device's public key
- **Signed Configuration:** Configuration signed by Central Station's private key
- **Device Verification:** Device verifies Central Station's authority before accepting configuration
- **Audit Logging:** All provisioning events logged to `security_audit_log`

## 3. QR Code Format

The QR code contains the following information in JSON format:

```json
{
  "deviceId": "ZM-001",
  "deviceSerial": "SN-2024-001234",
  "ipAddress": "192.168.1.100",
  "pairingCode": "ABC-123-XYZ",
  "pairingToken": "base64-encoded-time-limited-token",
  "expiresAt": "2024-11-25T14:30:00Z",
  "firmwareVersion": "1.2.3"
}
```

**Security:**
- `pairingToken`: Cryptographically secure random token, HMAC-signed with device's private key
- `expiresAt`: Timestamp when pairing code expires (10 minutes from generation)
- QR code regenerated every 30 seconds to prevent replay attacks

## 4. Pairing Code Format

- **Format:** `XXX-XXX-XXX` (3 groups of 3 alphanumeric characters, case-insensitive)
- **Example:** `ABC-123-XYZ`
- **Generation:** Cryptographically secure random generation
- **Expiration:** 10 minutes from generation
- **One-Time Use:** Invalidated after successful pairing

## 5. Configuration Payload

The Central Station pushes the following configuration to the device:

```json
{
  "serverUrl": "https://monitoring.hospital.com:8443",
  "caCertificate": "base64-encoded-ca-cert",
  "clientCertificate": "base64-encoded-client-cert",
  "clientPrivateKey": "base64-encoded-encrypted-private-key",
  "deviceId": "ZM-001",
  "bedId": "ICU-3B",
  "timestamp": "2024-11-25T14:25:00Z",
  "signature": "base64-encoded-signature"
}
```

**Security:**
- **Encryption:** Payload encrypted with device's public key (RSA-2048 or ECDSA-P256)
- **Signature:** Payload signed with Central Station's private key
- **Validation:** Device verifies signature before accepting configuration
- **Private Key:** Client private key encrypted with device-specific key derivation

## 6. Provisioning States

The device can be in one of the following provisioning states:

- **Not Provisioned:** Device has no server configuration (initial state)
- **Ready to Pair:** Device is in provisioning mode, displaying QR code
- **Pairing:** Central Station has initiated pairing process
- **Configuring:** Device is receiving and applying configuration
- **Validating:** Device is validating configuration and testing connection
- **Provisioned:** Device is successfully provisioned and connected
- **Error:** Provisioning failed (error message displayed)
- **Re-provisioning:** Device is being re-provisioned with new configuration

## 7. Error Handling

### 7.1. Common Errors

- **Pairing Code Expired:** QR code/pairing code expired, regenerate
- **Invalid Pairing Code:** Pairing code not recognized or already used
- **Configuration Validation Failed:** Configuration signature invalid or corrupted
- **Connection Test Failed:** Device cannot connect to server with new configuration
- **Certificate Validation Failed:** Certificates invalid or expired
- **Network Error:** Network connectivity issues during provisioning

### 7.2. Recovery Procedures

- **Automatic Retry:** Device automatically retries failed operations (3 attempts)
- **Manual Retry:** User can manually retry provisioning from error state
- **Reset Provisioning:** Clear provisioning state and start over
- **Factory Reset:** Complete device reset (requires administrator)

## 8. Re-provisioning

Devices can be re-provisioned to change server configuration:

1. **Enter Re-provisioning Mode:** Requires Technician role and explicit confirmation
2. **Follow Standard Workflow:** Same workflow as initial provisioning
3. **Preserve Data:** Patient data and settings preserved (unless factory reset)
4. **Certificate Rotation:** Old certificates revoked, new certificates installed
5. **Audit Logging:** Re-provisioning events logged for audit

## 9. UI Components

### 9.1. Provisioning View

**Location:** Settings → Network → Provisioning

**Components:**
- **Status Indicator:** Current provisioning state (Ready, Pairing, Configuring, Connected, Error)
- **QR Code Display:** Large QR code for scanning (regenerated every 30 seconds)
- **Pairing Code Display:** Human-readable pairing code (e.g., "ABC-123-XYZ")
- **Expiration Timer:** Countdown timer showing remaining time (e.g., "8:45 remaining")
- **Status Message:** Descriptive message for current state
- **Action Buttons:**
  - "Enter Provisioning Mode" (when not provisioned)
  - "Regenerate QR Code" (when ready to pair)
  - "Cancel Provisioning" (to exit provisioning mode)
  - "Re-provision Device" (when already provisioned, requires Technician role)

### 9.2. Connected Status View

**Location:** Settings → Network → Status (read-only)

**Components:**
- **Connection Status:** Connected/Disconnected indicator
- **Server URL:** Current server URL (read-only)
- **Certificate Status:** Certificate expiration, validation status
- **Last Connected:** Timestamp of last successful connection
- **Connection Statistics:** Uptime, data transmitted, errors

### 9.3. Simulated Provisioning (Development)

**Location:** Settings → Network → Provisioning → "Simulate Configuration"

**Purpose:** For development/testing without real Central Station

**Behavior:**
- Generates mock configuration payload
- Simulates Central Station push
- Applies configuration to device
- Tests connection with mock server

## 10. Security Considerations

### 10.1. Provisioning Mode Security

- **Explicit Activation:** Provisioning mode must be explicitly enabled (not automatic)
- **Role-Based Access:** Only Technician role can enter provisioning mode
- **Time Limits:** Pairing codes expire after 10 minutes
- **One-Time Use:** Pairing codes invalidated after use
- **Audit Logging:** All provisioning mode activations logged

### 10.2. Configuration Security

- **Encryption:** Configuration encrypted with device's public key
- **Signing:** Configuration signed by Central Station
- **Validation:** Device validates signature before accepting
- **Secure Storage:** Certificates stored with restricted permissions (600)
- **Key Derivation:** Private key encryption uses device-specific key derivation

### 10.3. Network Security

- **Initial Connection:** First connection may be unencrypted for provisioning
- **Configuration Push:** Configuration payload encrypted and signed
- **Post-Provisioning:** All subsequent communication uses mTLS
- **Certificate Validation:** Full certificate chain validation after provisioning

## 11. Implementation Components

### 11.1. ProvisioningService (C++)

**Responsibilities:**
- Generate pairing codes and QR codes
- Manage provisioning state machine
- Validate configuration payloads
- Apply configuration to device
- Test connections after provisioning

**Key Methods:**
- `enterProvisioningMode()`: Enters provisioning mode
- `exitProvisioningMode()`: Exits provisioning mode
- `generatePairingCode()`: Generates new pairing code
- `generateQRCode()`: Generates QR code with pairing information
- `validatePairingCode(const QString& code)`: Validates pairing code
- `receiveConfiguration(const QByteArray& payload)`: Receives and validates configuration
- `applyConfiguration(const ProvisioningConfig& config)`: Applies configuration
- `testConnection()`: Tests connection with new configuration

### 11.2. ProvisioningController (C++)

**Responsibilities:**
- Expose provisioning state to QML
- Handle provisioning UI interactions
- Manage QR code generation and display

**Properties (Q_PROPERTY):**
- `provisioningState`: Current provisioning state (enum)
- `pairingCode`: Current pairing code (QString)
- `qrCodeData`: QR code image data (QByteArray)
- `expirationTime`: Time remaining until pairing code expires (int, seconds)
- `errorMessage`: Error message if provisioning failed (QString)
- `isProvisioned`: Whether device is provisioned (bool)
- `serverUrl`: Current server URL (read-only, QString)
- `certificateStatus`: Certificate status information (QString)

**Q_INVOKABLE Methods:**
- `enterProvisioningMode()`: Enters provisioning mode
- `exitProvisioningMode()`: Exits provisioning mode
- `regenerateQRCode()`: Generates new QR code
- `simulateConfiguration()`: Simulates configuration push (development only)
- `reprovisionDevice()`: Starts re-provisioning process

### 11.3. Central Station API

**Endpoint:** `POST /api/provisioning/pair`

**Request:**
```json
{
  "deviceId": "ZM-001",
  "pairingCode": "ABC-123-XYZ",
  "pairingToken": "base64-token"
}
```

**Response:**
```json
{
  "success": true,
  "configuration": {
    "serverUrl": "https://monitoring.hospital.com:8443",
    "caCertificate": "...",
    "clientCertificate": "...",
    "clientPrivateKey": "...",
    "signature": "..."
  }
}
```

## 12. Database Schema

### 12.1. Provisioning Events Table

```sql
CREATE TABLE IF NOT EXISTS provisioning_events (
  id INTEGER PRIMARY KEY AUTOINCREMENT,
  timestamp INTEGER NOT NULL,
  event_type TEXT NOT NULL,
  device_id TEXT NOT NULL,
  pairing_code TEXT NULL,
  success BOOLEAN NOT NULL,
  error_message TEXT NULL,
  configuration_hash TEXT NULL
);
```

**Event Types:**
- `provisioning_mode_entered`
- `pairing_code_generated`
- `pairing_code_validated`
- `configuration_received`
- `configuration_applied`
- `connection_tested`
- `provisioning_completed`
- `provisioning_failed`
- `reprovisioning_started`

## 13. Testing

### 13.1. Unit Tests

- QR code generation and validation
- Pairing code generation and expiration
- Configuration payload encryption/decryption
- Configuration signature validation
- State machine transitions

### 13.2. Integration Tests

- End-to-end provisioning workflow
- Error handling and recovery
- Re-provisioning process
- Connection testing after provisioning

### 13.3. Security Tests

- Pairing code expiration enforcement
- One-time use enforcement
- Configuration signature validation
- Certificate validation after provisioning
- Audit logging verification

## 14. Migration from Manual Configuration

For devices already configured manually:

1. **Detect Manual Configuration:** Check if device has manual server URL/certificates
2. **Migration Prompt:** Prompt user to migrate to provisioning-based configuration
3. **Preserve Settings:** Preserve existing configuration during migration
4. **Re-provision:** Follow standard provisioning workflow
5. **Remove Manual Settings:** Clear manual configuration after successful provisioning

