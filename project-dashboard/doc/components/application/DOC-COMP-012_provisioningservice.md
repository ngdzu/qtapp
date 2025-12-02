---
doc_id: DOC-COMP-012
title: ProvisioningService
version: v1.0
category: Component
subcategory: Application Layer / Use Case Orchestration
status: Draft
owner: Application Layer Team
reviewers: 
  - Architecture Team
last_reviewed: 2025-01-26
next_review: 2026-01-26
related_docs:
  - DOC-ARCH-002  # System architecture
  - DOC-COMP-003  # DeviceAggregate
  - DOC-COMP-017  # IProvisioningRepository
related_tasks:
  - TASK-3B-001  # Phase 3B Migration
related_requirements:
  - REQ-FUN-DEV-001  # Device provisioning
  - REQ-INT-PROV-001  # Provisioning interface
tags:
  - application-service
  - provisioning
  - qr-code
  - device-configuration
  - certificate-management
diagram_files:
  - DOC-COMP-012_provisioning_workflow.mmd
  - DOC-COMP-012_provisioning_workflow.svg
---

# DOC-COMP-012: ProvisioningService

## 1. Overview

**Purpose:** Application service orchestrating device provisioning workflow via QR code pairing, enabling secure configuration delivery from Central Station including server URL, certificates, and device settings.

**Responsibilities:**
- Generate provisioning QR code containing device ID, IP address, and 6-character pairing code
- Validate incoming provisioning payload (signature verification, expiry check, pairing code match)
- Apply provisioning configuration (server URL, client certificate, CA certificate, device label)
- Manage provisioning state machine (UNPROVISIONED → READY_TO_PAIR → PROVISIONED)
- Store certificates in CertificateManager and settings in SettingsManager
- Emit Qt signals for QR code generation, provisioning completion, and status changes
- Support de-provisioning (admin-authorized reset to UNPROVISIONED state)

**Layer:** Application Layer

**Module:** Specification from `IProvisioningService.md` interface documentation

**Thread Affinity:** Application Services Thread (normal priority)

**Dependencies:**
- **Domain Aggregates:** DeviceAggregate (DOC-COMP-003) - Device provisioning state
- **Repository Interfaces:** IProvisioningRepository (DOC-COMP-017) - Provisioning session persistence
- **Infrastructure Services:** CertificateManager (certificate storage/validation), SettingsManager (device configuration)
- **Data Structures:** QRCodeData, ProvisioningPayload, ProvisioningStatus, ProvisioningResult, ProvisioningError

## 2. Architecture

![Provisioning Workflow](diagrams/DOC-COMP-012_provisioning_workflow.svg)

**Key Design Decisions:**
- **Decision 1: QR Code Expiry** - QR codes expire after 10 minutes for security, requiring regeneration if not scanned
- **Decision 2: Payload Signature Verification** - All provisioning payloads must be signed by Central Station; signature verified before applying configuration
- **Decision 3: One-Time Provisioning** - Device can only be provisioned once; de-provisioning requires administrator authentication
- **Decision 4: Certificate-Based Security** - mTLS certificates provisioned via encrypted payload, stored in CertificateManager

**Design Patterns Used:**
- **Application Service Pattern:** Coordinates provisioning workflow without business logic
- **State Machine Pattern:** ProvisioningStatus enum (UNPROVISIONED, READY_TO_PAIR, PROVISIONED, DEPROVISIONED)
- **Observer Pattern (Qt Signals/Slots):** Emits signals for QR code generation, provisioning completion, errors
- **Strategy Pattern:** Production (CentralStationProvisioningService) vs Mock (MockProvisioningService) implementations

## 3. Public API

### 3.1 Key Interface

```cpp
class IProvisioningService : public QObject {
    Q_OBJECT

public:
    /**
     * @brief Generate provisioning QR code.
     * @return QRCodeData containing device ID, IP, pairing code, QR image
     */
    virtual QRCodeData generateProvisioningQRCode(const QString& deviceId, const QString& deviceIp) = 0;

    /**
     * @brief Apply provisioning configuration from Central Station.
     * @param payload Encrypted and signed provisioning payload
     * @param pairingCode 6-character pairing code for validation
     * @return QFuture<ProvisioningResult> Future resolving to success/error
     */
    virtual QFuture<ProvisioningResult> applyConfiguration(
        const ProvisioningPayload& payload, const QString& pairingCode) = 0;

    /**
     * @brief Get current provisioning status.
     */
    virtual ProvisioningStatus getStatus() const = 0;

    /**
     * @brief De-provision device (admin-only).
     */
    virtual bool deprovisionDevice() = 0;

signals:
    void qrCodeGenerated(const QRCodeData& qrCodeData);
    void qrCodeExpired();
    void provisioningCompleted(const ProvisioningResult& result);
    void provisioningFailed(const ProvisioningError& error);
    void statusChanged(ProvisioningStatus status);
};
```

### 3.2 Data Structures

**QRCodeData:**
- `deviceId`: Device unique identifier
- `pairingCode`: 6-character alphanumeric code (e.g., "X7Y9Z2")
- `timestamp`, `expiresAt`: Generation time and expiry (10 minutes)
- `qrCodeImage`: QR code image (300x300 pixels)

**ProvisioningPayload:**
- `serverUrl`: Central server URL
- `clientCertificate`, `clientPrivateKey`, `caCertificate`: mTLS certificates (PEM format)
- `deviceLabel`: Device label (e.g., "ICU-MON-04")
- `signature`: HMAC-SHA256 signature for validation

**ProvisioningStatus:**
- `UNPROVISIONED`: Initial state, device not configured
- `READY_TO_PAIR`: QR code displayed, awaiting scan
- `PROVISIONED`: Successfully provisioned
- `DEPROVISIONED`: Admin-authorized de-provisioning

## 4. Implementation Highlights

**QR Code Generation:**
- Generate 6-character random alphanumeric pairing code
- Encode device ID, IP, pairing code in JSON
- Generate 300x300px QR code image
- Set 10-minute expiry timer

**Payload Validation:**
- Verify payload not expired
- Verify pairing code matches current session
- Verify HMAC-SHA256 signature using Central Station public key
- Validate device ID matches current device

**Configuration Application:**
- Save certificates to CertificateManager
- Save server URL, device label to SettingsManager
- Update device provisioning state to PROVISIONED
- Log provisioning event
- Emit provisioningCompleted signal

## 5. Usage Examples

### 5.1 Generate QR Code

```cpp
QString deviceId = "ZM-ICU-MON-04";
QString deviceIp = "10.1.50.104";

QRCodeData qrData = provisioningService->generateProvisioningQRCode(deviceId, deviceIp);

// Display QR code in QML
qmlView->setQRCodeImage(qrData.qrCodeImage);
qmlView->setPairingCode(qrData.pairingCode);
```

### 5.2 Apply Provisioning (Received from Central Station)

```cpp
ProvisioningPayload payload = receivePayloadFromCentralStation();

auto future = provisioningService->applyConfiguration(payload, qrData.pairingCode);

future.then([](ProvisioningResult result) {
    if (result.isSuccess()) {
        qInfo() << "Provisioning completed successfully";
    } else {
        qWarning() << "Provisioning failed:" << result.error.message;
    }
});
```

## 6. Testing

**Unit Tests:**
- `test_GenerateQRCode_ValidData()` - Verify QR code generation with valid device ID/IP
- `test_QRCodeExpiry()` - Verify QR code expires after 10 minutes
- `test_ApplyConfiguration_ValidPayload()` - Verify successful provisioning with valid payload
- `test_ApplyConfiguration_InvalidSignature_Fails()` - Verify rejection of invalid signature
- `test_ApplyConfiguration_ExpiredPayload_Fails()` - Verify rejection of expired payload

**Integration Tests:**
- End-to-end provisioning workflow (QR generation → payload reception → configuration application)
- Certificate persistence verification
- Settings persistence verification

## 7. Performance & Security

**Performance:**
- QR code generation: <500ms
- Payload validation: <100ms (signature verification)
- Configuration application: <1s (certificate/settings writes)

**Security:**
- QR code expires after 10 minutes (security requirement)
- Payload signature verification (HMAC-SHA256)
- mTLS certificates stored encrypted in CertificateManager
- One-time provisioning (prevents re-provisioning attacks)

## 8. Related Documentation

- DOC-COMP-003: DeviceAggregate - Device provisioning state machine
- DOC-COMP-017: IProvisioningRepository - Provisioning session persistence
- `doc/z-monitor/architecture_and_design/17_DEVICE_PROVISIONING.md` - Provisioning workflow documentation
- `doc/z-monitor/architecture_and_design/15_CERTIFICATE_PROVISIONING.md` - Certificate management

## 9. Changelog

| Version | Date       | Author      | Changes                                            |
| ------- | ---------- | ----------- | -------------------------------------------------- |
| v1.0    | 2025-01-26 | Dustin Wind | Initial documentation from IProvisioningService.md |
