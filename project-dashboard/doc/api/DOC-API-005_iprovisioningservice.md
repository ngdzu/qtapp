---
doc_id: DOC-API-005
title: IProvisioningService Interface
version: 1.0
category: API
subcategory: Device Management
status: Approved
created: 2025-11-27
updated: 2025-11-27
tags: [api, interface, provisioning, qr-code, configuration, certificates, device-pairing]
related_docs:
  - DOC-PROC-001 # Device Provisioning Workflow
  - DOC-PROC-002 # Certificate Provisioning
  - DOC-COMP-022 # SettingsManager
  - DOC-ARCH-002 # Architecture Patterns
authors:
  - Z Monitor Team
reviewers:
  - Architecture Team
---

# IProvisioningService Interface

## 1. Overview

The `IProvisioningService` interface defines the contract for device provisioning, enabling secure configuration delivery via QR code pairing workflow.

**Purpose:**
- Abstract provisioning mechanism (Central Station vs manual vs automated)
- Enable secure configuration delivery (server URL, certificates, settings)
- Support QR code-based pairing for ease of deployment
- Track provisioning lifecycle and status

**Key Characteristics:**
- **QR Code Workflow:** Generate QR code with pairing code for Central Station scanning
- **Secure:** Configuration payload encrypted and signed
- **Time-Limited:** QR codes expire after 10 minutes (security)
- **One-Time:** Provisioning typically performed once during device deployment
- **Reversible:** De-provisioning supported for device repurposing

---

## 2. Interface Definition

### 2.1 C++ Header

```cpp
/**
 * @interface IProvisioningService
 * @brief Interface for device provisioning and configuration delivery.
 * 
 * This interface manages the complete provisioning workflow from
 * QR code generation through configuration application.
 * 
 * @note Provisioning is one-time process during device deployment
 * @see ProvisioningPayload, ProvisioningSession
 * @ingroup Infrastructure
 */
class IProvisioningService : public QObject {
    Q_OBJECT

public:
    virtual ~IProvisioningService() = default;

    /**
     * @brief Generate provisioning QR code.
     * 
     * Creates QR code containing device identity and pairing code
     * for scanning by Central Station.
     * 
     * @param deviceId Unique device identifier
     * @param deviceIp Device IP address (for direct connection)
     * @return QRCodeData QR code content and image
     * 
     * @note QR code expires after 10 minutes (security)
     * @note Pairing code is 6-character alphanumeric
     */
    virtual QRCodeData generateProvisioningQRCode(
        const QString& deviceId,
        const QString& deviceIp
    ) = 0;

    /**
     * @brief Apply provisioning configuration.
     * 
     * Receives and applies configuration payload from Central Station,
     * including server URL, certificates, and device settings.
     * 
     * @param payload Encrypted and signed provisioning payload
     * @param pairingCode Pairing code for validation
     * @return QFuture<ProvisioningResult> Future resolving to success/error
     * 
     * @note Validates signature before applying configuration
     * @note Validates pairing code matches current session
     * @note Validates payload not expired
     */
    virtual QFuture<ProvisioningResult> applyConfiguration(
        const ProvisioningPayload& payload,
        const QString& pairingCode
    ) = 0;

    /**
     * @brief Get current provisioning status.
     * 
     * Returns device provisioning state.
     * 
     * @return ProvisioningStatus Current status
     */
    virtual ProvisioningStatus getStatus() const = 0;

    /**
     * @brief Test configuration (simulate incoming provisioning).
     * 
     * Development/testing function to simulate provisioning without
     * actual Central Station.
     * 
     * @param mockPayload Mock configuration payload
     * @return QFuture<ProvisioningResult> Future resolving to result
     * 
     * @note Development/testing only
     */
    virtual QFuture<ProvisioningResult> simulateProvisioning(
        const ProvisioningPayload& mockPayload
    ) = 0;

    /**
     * @brief Reset provisioning state (de-provision device).
     * 
     * Removes all provisioning configuration, returning device to
     * unprovisioned state. Requires administrator authorization.
     * 
     * @return bool true if de-provisioning succeeded
     * 
     * @note Requires administrator authentication
     * @note Logs de-provisioning event
     */
    virtual bool deprovisionDevice() = 0;

signals:
    void qrCodeGenerated(const QRCodeData& qrCodeData);
    void qrCodeExpired();
    void configurationReceived(const ProvisioningPayload& payload);
    void provisioningCompleted(const ProvisioningResult& result);
    void provisioningFailed(const ProvisioningError& error);
    void statusChanged(ProvisioningStatus status);
};
```

---

## 3. Data Structures

### 3.1 QRCodeData

```cpp
/**
 * @struct QRCodeData
 * @brief QR code content and image for provisioning.
 */
struct QRCodeData {
    QString deviceId;           ///< Device unique identifier
    QString serialNumber;       ///< Device serial number
    QString ipAddress;          ///< Device IP address
    QString pairingCode;        ///< 6-character pairing code (e.g., "X7Y9Z2")
    QDateTime timestamp;        ///< Generation time
    QDateTime expiresAt;        ///< Expiry time (timestamp + 10 minutes)
    QImage qrCodeImage;         ///< QR code image (300x300 pixels)
    QString qrCodeContent;      ///< JSON content encoded in QR
    
    bool isExpired() const {
        return QDateTime::currentDateTime() > expiresAt;
    }
    
    int secondsUntilExpiry() const {
        int secs = QDateTime::currentDateTime().secsTo(expiresAt);
        return qMax(0, secs);
    }
};
```

### 3.2 ProvisioningPayload

```cpp
/**
 * @struct ProvisioningPayload
 * @brief Configuration payload from Central Station.
 * 
 * Contains all configuration needed for device operation.
 * Encrypted and signed for security.
 */
struct ProvisioningPayload {
    QString deviceId;                   ///< Device identifier (must match)
    QString pairingCode;                ///< Pairing code for validation
    
    // Server Configuration
    QString serverUrl;                  ///< Central server URL
    int syncInterval;                   ///< Sync interval in seconds (default: 10)
    
    // Certificates (PEM format)
    QString clientCertificate;          ///< Client certificate for mTLS
    QString clientPrivateKey;           ///< Client private key
    QString caCertificate;              ///< CA certificate
    QString serverCertificate;          ///< Server certificate (for pinning)
    
    // Device Settings
    QString deviceLabel;                ///< Device label (e.g., "ICU-MON-04")
    QString measurementUnit;            ///< METRIC or IMPERIAL
    QString language;                   ///< EN, ES, etc.
    
    // Security
    QString signature;                  ///< HMAC-SHA256 signature of payload
    QDateTime timestamp;                ///< Payload creation time
    QDateTime expiresAt;                ///< Expiry time (timestamp + 1 hour)
    
    bool isValid() const {
        return !deviceId.isEmpty() && !serverUrl.isEmpty() &&
               !clientCertificate.isEmpty() && !signature.isEmpty();
    }
    
    bool isExpired() const {
        return QDateTime::currentDateTime() > expiresAt;
    }
    
    bool verifySignature(const QByteArray& publicKey) const;
};
```

### 3.3 ProvisioningStatus

```cpp
/**
 * @enum ProvisioningStatus
 * @brief Device provisioning status.
 */
enum class ProvisioningStatus {
    UNPROVISIONED,      ///< Device not yet provisioned (initial state)
    READY_TO_PAIR,      ///< QR code displayed, waiting for scan
    PAIRING_RECEIVED,   ///< Configuration received, validating
    PROVISIONED,        ///< Successfully provisioned
    PROVISIONING_FAILED,///< Provisioning failed (error state)
    DEPROVISIONED       ///< Device de-provisioned (admin action)
};
```

---

## 4. Implementations

### 4.1 CentralStationProvisioningService (Production)

**Technology:** HTTPS listener + certificate storage

**Features:**
- Receives provisioning payload via HTTPS POST from Central Station
- Validates signature (HMAC-SHA256)
- Validates pairing code and expiry
- Installs certificates to secure database
- Applies configuration to SettingsManager

### 4.2 MockProvisioningService (Testing)

**Technology:** In-memory (no network)

**Features:**
- Simulates provisioning without Central Station
- Configurable delay (default: 1000ms)
- Auto-provision option for testing convenience
- No external dependencies

---

## 5. Usage Example

```cpp
// Step 1: Generate QR code
QString deviceId = "ZM-ICU-MON-04";
QString deviceIp = "10.1.50.104";

QRCodeData qrData = provisioningService->generateProvisioningQRCode(deviceId, deviceIp);

// Display QR code in UI
qrCodeDisplay->setPixmap(QPixmap::fromImage(qrData.qrCodeImage));
pairingCodeLabel->setText(qrData.pairingCode);

// Step 2: Wait for Central Station to send configuration
// (Configuration received via HTTPS POST from Central Station)

// Step 3: Apply configuration
auto future = provisioningService->applyConfiguration(payload, qrData.pairingCode);

future.then([](const ProvisioningResult& result) {
    if (result.isSuccess()) {
        m_logService->info("Provisioning completed successfully", {
            {"serverUrl", result.serverUrl},
            {"deviceLabel", result.deviceLabel}
        });
        
        // Update UI
        showProvisionedState();
        
        // Connect to server
        connectToServer();
    } else {
        m_logService->critical("Provisioning failed", {
            {"error", result.error.message}
        });
        showError(result.error.message);
    }
});
```

---

## 6. State Machine

**Provisioning State Transitions:**
```
UNPROVISIONED
    ↓ [generateProvisioningQRCode()]
READY_TO_PAIR
    ↓ [QR scanned by Central Station]
    ↓ [applyConfiguration() called]
PAIRING_RECEIVED (validating)
    ↓ [validation succeeds]
PROVISIONED ✅
    
    OR
    
    ↓ [validation fails]
PROVISIONING_FAILED ❌
```

---

## 7. Security Considerations

### 7.1 Signature Verification

- **HMAC-SHA256:** All payloads signed with Central Station private key
- **Verification:** Device verifies signature before applying configuration
- **Prevents Tampering:** Ensures configuration integrity

### 7.2 Pairing Code Validation

- **Case-Insensitive:** "X7Y9Z2" = "x7y9z2"
- **Time-Limited:** Pairing code expires after 10 minutes
- **One-Time Use:** Pairing code invalidated after successful provisioning

### 7.3 Certificate Installation

- **Encrypted Storage:** Certificates stored in encrypted database
- **Validation:** Verify certificate format and validity before installation
- **Audit Logging:** Log all certificate installation events

---

## 8. Performance Characteristics

- **QR Code Generation:** < 500ms
- **Configuration Application:** < 5 seconds (including certificate installation)
- **QR Code Image Size:** 300x300 pixels
- **Error Correction:** Level H (30% correction) for reliable scanning

---

## 9. Related Documents

- **DOC-PROC-001:** Device Provisioning Workflow - Complete provisioning workflow
- **DOC-PROC-002:** Certificate Provisioning - Certificate management
- **DOC-COMP-022:** SettingsManager - Configuration storage
- **DOC-ARCH-002:** Architecture Patterns - Infrastructure layer interfaces

---

## 10. Changelog

| Version | Date       | Author         | Changes                                        |
| ------- | ---------- | -------------- | ---------------------------------------------- |
| 1.0     | 2025-11-27 | Z Monitor Team | Migrated from INTERFACE-003, added frontmatter |

---

*This interface enables secure, user-friendly device provisioning through QR code workflow, eliminating manual configuration errors and improving deployment efficiency.*
