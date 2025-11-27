# IProvisioningService Interface

**Interface ID:** IFACE-003  
**Version:** 1.0  
**Status:** Approved  
**Last Updated:** 2025-11-27

---

## 1. Overview

The `IProvisioningService` interface defines the contract for device provisioning, enabling secure configuration delivery via QR code pairing workflow.

**Purpose:**
- Abstract provisioning mechanism (Central Station vs manual vs automated)
- Enable secure configuration delivery
- Support certificate-based device authentication
- Track provisioning lifecycle

**Related Documents:**
- **Requirements:** [REQ-FUN-DEV-001](../../requirements/03_FUNCTIONAL_REQUIREMENTS.md), [REQ-INT-PROV-001](../../requirements/06_INTERFACE_REQUIREMENTS.md)
- **Use Cases:** [UC-DP-001](../../requirements/02_USE_CASES.md), [UC-DP-002](../../requirements/02_USE_CASES.md), [UC-DP-003](../../requirements/02_USE_CASES.md)
- **Workflow:** [17_DEVICE_PROVISIONING.md](../17_DEVICE_PROVISIONING.md)
- **Certificates:** [15_CERTIFICATE_PROVISIONING.md](../15_CERTIFICATE_PROVISIONING.md)

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
    /**
     * @brief Emitted when QR code generated.
     * 
     * @param qrCodeData QR code content and image
     */
    void qrCodeGenerated(const QRCodeData& qrCodeData);

    /**
     * @brief Emitted when QR code expires.
     * 
     * QR codes expire after 10 minutes for security.
     */
    void qrCodeExpired();

    /**
     * @brief Emitted when provisioning configuration received.
     * 
     * @param payload Configuration payload (for validation preview)
     */
    void configurationReceived(const ProvisioningPayload& payload);

    /**
     * @brief Emitted when provisioning completes successfully.
     * 
     * @param result Provisioning result with configuration details
     */
    void provisioningCompleted(const ProvisioningResult& result);

    /**
     * @brief Emitted when provisioning fails.
     * 
     * @param error Error details
     */
    void provisioningFailed(const ProvisioningError& error);

    /**
     * @brief Emitted when provisioning status changes.
     * 
     * @param status New provisioning status
     */
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
    
    /**
     * @brief Check if QR code is expired.
     * @return true if current time > expiresAt
     */
    bool isExpired() const {
        return QDateTime::currentDateTime() > expiresAt;
    }
    
    /**
     * @brief Get time remaining until expiry.
     * @return Seconds remaining (0 if expired)
     */
    int secondsUntilExpiry() const {
        int secs = QDateTime::currentDateTime().secsTo(expiresAt);
        return qMax(0, secs);
    }
    
    /**
     * @brief Generate JSON content for QR code.
     */
    QString generateQRContent() const {
        QJsonObject obj;
        obj["deviceId"] = deviceId;
        obj["serialNumber"] = serialNumber;
        obj["ipAddress"] = ipAddress;
        obj["pairingCode"] = pairingCode;
        obj["timestamp"] = timestamp.toMSecsSinceEpoch();
        return QString::fromUtf8(QJsonDocument(obj).toJson(QJsonDocument::Compact));
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
    
    /**
     * @brief Validate payload.
     * @return true if all required fields present
     */
    bool isValid() const {
        return !deviceId.isEmpty() && !serverUrl.isEmpty() &&
               !clientCertificate.isEmpty() && !clientPrivateKey.isEmpty() &&
               !caCertificate.isEmpty() && !signature.isEmpty();
    }
    
    /**
     * @brief Check if payload is expired.
     */
    bool isExpired() const {
        return QDateTime::currentDateTime() > expiresAt;
    }
    
    /**
     * @brief Verify payload signature.
     * @param publicKey Central Station public key
     * @return true if signature valid
     */
    bool verifySignature(const QByteArray& publicKey) const;
    
    /**
     * @brief Serialize to JSON.
     */
    QByteArray toJson() const;
    
    /**
     * @brief Parse from JSON.
     */
    static ProvisioningPayload fromJson(const QByteArray& json);
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

/**
 * @brief Convert status to string.
 */
QString provisioningStatusToString(ProvisioningStatus status) {
    switch (status) {
        case ProvisioningStatus::UNPROVISIONED: return "UNPROVISIONED";
        case ProvisioningStatus::READY_TO_PAIR: return "READY_TO_PAIR";
        case ProvisioningStatus::PAIRING_RECEIVED: return "PAIRING_RECEIVED";
        case ProvisioningStatus::PROVISIONED: return "PROVISIONED";
        case ProvisioningStatus::PROVISIONING_FAILED: return "PROVISIONING_FAILED";
        case ProvisioningStatus::DEPROVISIONED: return "DEPROVISIONED";
    }
    return "UNKNOWN";
}
```

### 3.4 ProvisioningResult

```cpp
/**
 * @struct ProvisioningResult
 * @brief Result of provisioning operation.
 */
struct ProvisioningResult {
    bool success;                       ///< true if provisioning succeeded
    ProvisioningStatus newStatus;       ///< New device status
    QString deviceId;                   ///< Device ID
    QString serverUrl;                  ///< Server URL configured
    QString deviceLabel;                ///< Device label configured
    QDateTime completedAt;              ///< Completion time
    ProvisioningError error;            ///< Error details (if failed)
    
    bool isSuccess() const { return success; }
};
```

### 3.5 ProvisioningError

```cpp
/**
 * @struct ProvisioningError
 * @brief Error details for provisioning failure.
 */
struct ProvisioningError {
    ErrorCode code;                     ///< Error code
    QString message;                    ///< Human-readable message
    QVariantMap details;                ///< Additional context
    
    bool hasError() const { return code != ErrorCode::None; }
};

/**
 * @enum ErrorCode
 * @brief Provisioning error codes.
 */
enum class ErrorCode {
    None,                               ///< No error
    InvalidPairingCode,                 ///< Pairing code mismatch
    PayloadExpired,                     ///< Payload timestamp expired
    SignatureInvalid,                   ///< Signature verification failed
    InvalidCertificate,                 ///< Certificate format invalid
    DeviceIdMismatch,                   ///< Device ID in payload doesn't match
    AlreadyProvisioned,                 ///< Device already provisioned
    NetworkError,                       ///< Network communication failed
    StorageError,                       ///< Failed to save configuration
    UnknownError                        ///< Unexpected error
};
```

---

## 4. Implementations

### 4.1 Production Implementation: CentralStationProvisioningService

**Technology:** HTTPS listener + certificate storage

```cpp
/**
 * @class CentralStationProvisioningService
 * @brief Production implementation for Central Station provisioning.
 * 
 * Receives provisioning payload via HTTPS POST from Central Station
 * and applies configuration to device.
 */
class CentralStationProvisioningService : public IProvisioningService {
    Q_OBJECT

public:
    /**
     * @brief Constructor.
     * @param certificateManager Certificate storage and validation
     * @param settingsManager Device settings storage
     */
    CentralStationProvisioningService(
        CertificateManager* certificateManager,
        SettingsManager* settingsManager,
        QObject* parent = nullptr
    );

    // IProvisioningService interface
    QRCodeData generateProvisioningQRCode(const QString& deviceId, const QString& deviceIp) override;
    QFuture<ProvisioningResult> applyConfiguration(const ProvisioningPayload& payload, const QString& pairingCode) override;
    ProvisioningStatus getStatus() const override;
    QFuture<ProvisioningResult> simulateProvisioning(const ProvisioningPayload& mockPayload) override;
    bool deprovisionDevice() override;

private slots:
    void onProvisioningRequestReceived(const QByteArray& payload);
    void onQRCodeExpired();

private:
    ProvisioningResult validateAndApply(const ProvisioningPayload& payload, const QString& pairingCode);
    void saveCertificates(const ProvisioningPayload& payload);
    void saveConfiguration(const ProvisioningPayload& payload);
    void logProvisioningEvent(const QString& event, bool success);

    CertificateManager* m_certificateManager;
    SettingsManager* m_settingsManager;
    ProvisioningStatus m_status = ProvisioningStatus::UNPROVISIONED;
    QString m_currentPairingCode;
    QDateTime m_qrCodeExpiry;
    QTimer* m_expiryTimer;
};
```

### 4.2 Mock Implementation: MockProvisioningService

**Technology:** In-memory (no network)

```cpp
/**
 * @class MockProvisioningService
 * @brief Mock implementation for testing and development.
 * 
 * Simulates provisioning without Central Station.
 */
class MockProvisioningService : public IProvisioningService {
    Q_OBJECT

public:
    MockProvisioningService(QObject* parent = nullptr);

    /**
     * @brief Set simulated delay for provisioning.
     * @param delayMs Delay in milliseconds (default: 1000ms)
     */
    void setSimulatedDelay(int delayMs) { m_delayMs = delayMs; }

    /**
     * @brief Simulate provisioning failure.
     */
    void setSimulateFailure(bool shouldFail) { m_simulateFailure = shouldFail; }

    /**
     * @brief Auto-provision after QR generation (testing convenience).
     */
    void setAutoProvision(bool enabled) { m_autoProvision = enabled; }

    // IProvisioningService interface
    QRCodeData generateProvisioningQRCode(const QString& deviceId, const QString& deviceIp) override;
    QFuture<ProvisioningResult> applyConfiguration(const ProvisioningPayload& payload, const QString& pairingCode) override;
    ProvisioningStatus getStatus() const override;
    QFuture<ProvisioningResult> simulateProvisioning(const ProvisioningPayload& mockPayload) override;
    bool deprovisionDevice() override;

private:
    ProvisioningStatus m_status = ProvisioningStatus::UNPROVISIONED;
    QString m_currentPairingCode;
    int m_delayMs = 1000;
    bool m_simulateFailure = false;
    bool m_autoProvision = false;
};
```

---

## 5. Usage Examples

### 5.1 Generate QR Code for Provisioning

```cpp
// Get device information
QString deviceId = "ZM-ICU-MON-04";
QString deviceIp = "10.1.50.104";

// Generate QR code
QRCodeData qrData = provisioningService->generateProvisioningQRCode(deviceId, deviceIp);

// Display QR code in UI
qDebug() << "Device ID:" << qrData.deviceId;
qDebug() << "Pairing Code:" << qrData.pairingCode;
qDebug() << "Expires in:" << qrData.secondsUntilExpiry() << "seconds";

// Show QR code image
qrCodeDisplay->setPixmap(QPixmap::fromImage(qrData.qrCodeImage));
pairingCodeLabel->setText(qrData.pairingCode);

// Setup expiry timer
QTimer::singleShot(qrData.secondsUntilExpiry() * 1000, [this]() {
    qWarning() << "QR code expired. Generate new code.";
    showExpiredMessage();
});
```

### 5.2 Apply Provisioning Configuration

```cpp
// Receive provisioning payload (from Central Station via HTTPS POST)
ProvisioningPayload payload = ProvisioningPayload::fromJson(requestBody);

// Get pairing code from current session
QString pairingCode = getCurrentPairingCode();

// Apply configuration
auto future = provisioningService->applyConfiguration(payload, pairingCode);

future.then([](const ProvisioningResult& result) {
    if (result.isSuccess()) {
        qDebug() << "Provisioning completed successfully!";
        qDebug() << "Server URL:" << result.serverUrl;
        qDebug() << "Device Label:" << result.deviceLabel;
        
        // Update UI
        showProvisionedState();
        
        // Connect to server
        connectToServer();
    } else {
        qCritical() << "Provisioning failed:" << result.error.message;
        
        // Handle specific errors
        switch (result.error.code) {
            case ErrorCode::InvalidPairingCode:
                showError("Pairing code mismatch. Generate new QR code.");
                break;
            case ErrorCode::PayloadExpired:
                showError("Configuration expired. Request new provisioning.");
                break;
            case ErrorCode::SignatureInvalid:
                showError("Invalid signature. Security violation!");
                break;
            default:
                showError("Provisioning failed: " + result.error.message);
                break;
        }
    }
});
```

### 5.3 Complete Provisioning Workflow

```cpp
/**
 * @brief Complete QR code provisioning workflow.
 */
class ProvisioningController : public QObject {
    Q_OBJECT

public:
    void startProvisioning() {
        // Step 1: Generate QR code
        QString deviceId = Settings::instance()->deviceId();
        QString deviceIp = NetworkUtils::getLocalIpAddress();
        
        QRCodeData qrData = m_provisioningService->generateProvisioningQRCode(deviceId, deviceIp);
        
        // Display QR code
        displayQRCode(qrData);
        
        // Store pairing code for later validation
        m_currentPairingCode = qrData.pairingCode;
        
        // Setup expiry timer
        startExpiryTimer(qrData.secondsUntilExpiry());
        
        // Update status
        m_status = ProvisioningStatus::READY_TO_PAIR;
        emit statusChanged(m_status);
    }
    
    void onProvisioningPayloadReceived(const QByteArray& payloadJson) {
        // Step 2: Parse payload
        ProvisioningPayload payload = ProvisioningPayload::fromJson(payloadJson);
        
        // Step 3: Apply configuration
        auto future = m_provisioningService->applyConfiguration(payload, m_currentPairingCode);
        
        future.then([this](const ProvisioningResult& result) {
            if (result.isSuccess()) {
                handleProvisioningSuccess(result);
            } else {
                handleProvisioningFailure(result.error);
            }
        });
    }

private:
    void handleProvisioningSuccess(const ProvisioningResult& result) {
        // Update status
        m_status = ProvisioningStatus::PROVISIONED;
        emit statusChanged(m_status);
        
        // Notify UI
        emit provisioningCompleted(result);
        
        // Connect to server
        connectToServer(result.serverUrl);
        
        // Log success
        qInfo() << "Device provisioned successfully";
    }
    
    void handleProvisioningFailure(const ProvisioningError& error) {
        m_status = ProvisioningStatus::PROVISIONING_FAILED;
        emit statusChanged(m_status);
        emit provisioningFailed(error);
        
        qCritical() << "Provisioning failed:" << error.message;
    }

    IProvisioningService* m_provisioningService;
    QString m_currentPairingCode;
    ProvisioningStatus m_status;
};
```

---

## 6. State Machine

### 6.1 Provisioning State Transitions

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
    ↓ [retry or manual intervention]
UNPROVISIONED
```

### 6.2 State Machine Implementation

```cpp
/**
 * @brief Provisioning state machine.
 */
void ProvisioningService::transitionToState(ProvisioningStatus newStatus) {
    ProvisioningStatus oldStatus = m_status;
    
    // Validate transition
    if (!isValidTransition(oldStatus, newStatus)) {
        qWarning() << "Invalid state transition:" 
                   << provisioningStatusToString(oldStatus) << "->" 
                   << provisioningStatusToString(newStatus);
        return;
    }
    
    // Perform transition
    m_status = newStatus;
    
    // Log transition
    qInfo() << "Provisioning state:" 
            << provisioningStatusToString(oldStatus) << "->" 
            << provisioningStatusToString(newStatus);
    
    // Emit signal
    emit statusChanged(newStatus);
    
    // Perform state-specific actions
    onStateEntered(newStatus);
}

bool ProvisioningService::isValidTransition(ProvisioningStatus from, ProvisioningStatus to) {
    // Valid transitions
    QMap<ProvisioningStatus, QList<ProvisioningStatus>> validTransitions = {
        {ProvisioningStatus::UNPROVISIONED, {ProvisioningStatus::READY_TO_PAIR}},
        {ProvisioningStatus::READY_TO_PAIR, {ProvisioningStatus::PAIRING_RECEIVED, ProvisioningStatus::UNPROVISIONED}},
        {ProvisioningStatus::PAIRING_RECEIVED, {ProvisioningStatus::PROVISIONED, ProvisioningStatus::PROVISIONING_FAILED}},
        {ProvisioningStatus::PROVISIONED, {ProvisioningStatus::DEPROVISIONED}},
        {ProvisioningStatus::PROVISIONING_FAILED, {ProvisioningStatus::UNPROVISIONED}},
        {ProvisioningStatus::DEPROVISIONED, {ProvisioningStatus::UNPROVISIONED}}
    };
    
    return validTransitions[from].contains(to);
}
```

---

## 7. Security Considerations

### 7.1 Signature Verification

```cpp
/**
 * @brief Verify provisioning payload signature.
 */
bool ProvisioningPayload::verifySignature(const QByteArray& publicKey) const {
    // Serialize payload (excluding signature field)
    QJsonObject obj = toJsonObject();
    obj.remove("signature");
    QByteArray data = QJsonDocument(obj).toJson(QJsonDocument::Compact);
    
    // Verify HMAC-SHA256 signature
    QMessageAuthenticationCode mac(QCryptographicHash::Sha256);
    mac.setKey(publicKey);
    mac.addData(data);
    QString expected = QString::fromLatin1(mac.result().toHex());
    
    return signature == expected;
}
```

### 7.2 Pairing Code Validation

```cpp
/**
 * @brief Validate pairing code.
 */
bool validatePairingCode(const QString& provided, const QString& expected) {
    // Case-insensitive comparison
    if (provided.toUpper() != expected.toUpper()) {
        qWarning() << "Pairing code mismatch";
        logSecurityEvent("PAIRING_CODE_MISMATCH", provided);
        return false;
    }
    
    // Check QR code not expired
    if (isQRCodeExpired()) {
        qWarning() << "Pairing code expired";
        return false;
    }
    
    return true;
}
```

### 7.3 Certificate Installation Security

```cpp
/**
 * @brief Install certificates securely.
 */
void ProvisioningService::installCertificates(const ProvisioningPayload& payload) {
    // Parse certificates
    QSslCertificate clientCert = QSslCertificate::fromData(
        payload.clientCertificate.toUtf8(), QSsl::Pem
    );
    
    QSslKey clientKey = QSslKey(
        payload.clientPrivateKey.toUtf8(), QSsl::Rsa, QSsl::Pem
    );
    
    // Validate certificate
    if (clientCert.isNull() || clientKey.isNull()) {
        throw ProvisioningException("Invalid certificate format");
    }
    
    // Verify certificate matches key
    if (!certificateMatchesKey(clientCert, clientKey)) {
        throw ProvisioningException("Certificate/key mismatch");
    }
    
    // Store in secure database (encrypted)
    m_certificateManager->storeCertificate(clientCert, "CLIENT");
    m_certificateManager->storePrivateKey(clientKey, "CLIENT");
    m_certificateManager->storeCertificate(caCert, "CA");
    
    // Log security event
    logSecurityEvent("CERTIFICATES_INSTALLED", clientCert.serialNumber());
}
```

---

## 8. Testing

### 8.1 Unit Test Examples

```cpp
TEST(IProvisioningService, GenerateQRCode) {
    // Arrange
    MockProvisioningService service;
    
    // Act
    QRCodeData qrData = service.generateProvisioningQRCode("ZM-TEST-01", "10.1.50.100");
    
    // Assert
    EXPECT_EQ(qrData.deviceId, "ZM-TEST-01");
    EXPECT_EQ(qrData.ipAddress, "10.1.50.100");
    EXPECT_EQ(qrData.pairingCode.length(), 6);
    EXPECT_FALSE(qrData.qrCodeImage.isNull());
    EXPECT_FALSE(qrData.isExpired());
}

TEST(IProvisioningService, ApplyConfigurationSuccess) {
    // Arrange
    MockProvisioningService service;
    auto qrData = service.generateProvisioningQRCode("ZM-TEST-01", "10.1.50.100");
    
    ProvisioningPayload payload;
    payload.deviceId = "ZM-TEST-01";
    payload.pairingCode = qrData.pairingCode;
    payload.serverUrl = "https://test-server.com";
    payload.clientCertificate = "--- MOCK CERT ---";
    payload.clientPrivateKey = "--- MOCK KEY ---";
    payload.caCertificate = "--- MOCK CA ---";
    payload.signature = "valid-signature";
    
    // Act
    auto future = service.applyConfiguration(payload, qrData.pairingCode);
    auto result = future.result();
    
    // Assert
    ASSERT_TRUE(result.isSuccess());
    EXPECT_EQ(result.newStatus, ProvisioningStatus::PROVISIONED);
    EXPECT_EQ(service.getStatus(), ProvisioningStatus::PROVISIONED);
}

TEST(IProvisioningService, InvalidPairingCode) {
    // Arrange
    MockProvisioningService service;
    auto qrData = service.generateProvisioningQRCode("ZM-TEST-01", "10.1.50.100");
    
    ProvisioningPayload payload;
    payload.deviceId = "ZM-TEST-01";
    payload.pairingCode = "WRONG1"; // Incorrect!
    
    // Act
    auto future = service.applyConfiguration(payload, qrData.pairingCode);
    auto result = future.result();
    
    // Assert
    ASSERT_FALSE(result.isSuccess());
    EXPECT_EQ(result.error.code, ErrorCode::InvalidPairingCode);
    EXPECT_EQ(service.getStatus(), ProvisioningStatus::PROVISIONING_FAILED);
}

TEST(IProvisioningService, QRCodeExpiry) {
    // Arrange
    MockProvisioningService service;
    auto qrData = service.generateProvisioningQRCode("ZM-TEST-01", "10.1.50.100");
    
    // Simulate time passage (11 minutes)
    QTest::qWait(11 * 60 * 1000); // In real test, use time mocking
    
    // Assert
    EXPECT_TRUE(qrData.isExpired());
    EXPECT_EQ(qrData.secondsUntilExpiry(), 0);
}
```

---

## 9. Integration with UI

### 9.1 QML Controller Example

```qml
// ProvisioningController.h
class ProvisioningController : public QObject {
    Q_OBJECT
    Q_PROPERTY(QString provisioningStatus READ provisioningStatus NOTIFY statusChanged)
    Q_PROPERTY(QImage qrCodeImage READ qrCodeImage NOTIFY qrCodeChanged)
    Q_PROPERTY(QString pairingCode READ pairingCode NOTIFY qrCodeChanged)
    Q_PROPERTY(int expirySeconds READ expirySeconds NOTIFY expiryChanged)

public:
    Q_INVOKABLE void generateQRCode();
    Q_INVOKABLE void simulateProvisioning();
    Q_INVOKABLE void deprovision();

signals:
    void statusChanged();
    void qrCodeChanged();
    void expiryChanged();
    void provisioningSucceeded();
    void provisioningFailed(const QString& error);

private slots:
    void onProvisioningCompleted(const ProvisioningResult& result);
    void onProvisioningFailed(const ProvisioningError& error);
    void onQRCodeExpired();

private:
    IProvisioningService* m_provisioningService;
    QRCodeData m_currentQRCode;
    QTimer* m_expiryTimer;
};
```

### 9.2 QML View Example

```qml
// ProvisioningView.qml
Rectangle {
    ColumnLayout {
        // QR Code Display
        Image {
            source: provisioningController.qrCodeImage
            width: 300
            height: 300
            visible: provisioningController.provisioningStatus === "READY_TO_PAIR"
        }
        
        // Pairing Code
        Text {
            text: "Pairing Code: " + provisioningController.pairingCode
            font.pixelSize: 36
            visible: provisioningController.provisioningStatus === "READY_TO_PAIR"
        }
        
        // Expiry Countdown
        Text {
            text: "Expires in: " + provisioningController.expirySeconds + " seconds"
            color: provisioningController.expirySeconds < 60 ? "red" : "black"
        }
        
        // Simulate Button (Development Only)
        Button {
            text: "Simulate Incoming Configuration"
            visible: isDevelopmentMode
            onClicked: provisioningController.simulateProvisioning()
        }
        
        // Status Display
        Text {
            text: "Status: " + provisioningController.provisioningStatus
            font.bold: true
        }
    }
    
    Connections {
        target: provisioningController
        
        function onProvisioningSucceeded() {
            showSuccessMessage("Device provisioned successfully!");
            navigateTo("dashboard");
        }
        
        function onProvisioningFailed(error) {
            showErrorDialog("Provisioning Failed", error);
        }
        
        function onQrCodeExpired() {
            showWarning("QR code expired. Generate new code.");
        }
    }
}
```

---

## 10. Performance Considerations

### 10.1 QR Code Generation Performance

- **Generation Time:** < 500ms
- **Image Size:** 300x300 pixels (optimal for tablet scanning)
- **Error Correction:** Level H (30% correction) enables scanning at angles
- **Caching:** QR code cached (don't regenerate on every UI refresh)

### 10.2 Configuration Application Performance

- **Certificate Parsing:** < 100ms
- **Database Storage:** < 500ms (encrypted storage)
- **Total Provisioning Time:** < 5 seconds (target per REQ-FUN-DEV-001)

---

## 11. Related Documents

- **Requirements:** [03_FUNCTIONAL_REQUIREMENTS.md](../../requirements/03_FUNCTIONAL_REQUIREMENTS.md) (REQ-FUN-DEV-001)
- **Requirements:** [06_INTERFACE_REQUIREMENTS.md](../../requirements/06_INTERFACE_REQUIREMENTS.md) (REQ-INT-PROV-001, 002)
- **Use Cases:** [02_USE_CASES.md](../../requirements/02_USE_CASES.md) (UC-DP-001, 002, 003)
- **Workflow:** [17_DEVICE_PROVISIONING.md](../17_DEVICE_PROVISIONING.md)
- **Certificates:** [15_CERTIFICATE_PROVISIONING.md](../15_CERTIFICATE_PROVISIONING.md)
- **Security:** [06_SECURITY.md](../06_SECURITY.md)

---

*This interface enables secure, user-friendly device provisioning through QR code workflow, eliminating manual configuration errors and improving deployment efficiency.*

