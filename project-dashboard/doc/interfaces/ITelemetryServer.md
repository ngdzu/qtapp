# ITelemetryServer Interface

## Purpose

Provides a standardized interface for sending telemetry data and sensor data to a central monitoring server. This interface abstracts the server communication layer, allowing for different implementations: production server (network-based), mock server (testing/development), and local file-based server (offline testing).

## Responsibilities

- Send telemetry data batches to the server
- Send sensor data to the server
- Handle server responses (acknowledgments, error codes)
- Manage connection state and health
- Support configurable server endpoints (URL, port, protocol)
- Handle retry logic and error recovery
- Support both synchronous and asynchronous transmission

## Threading & Ownership

- `ITelemetryServer` implementations should be owned by `NetworkManager` or `AppContainer`
- Network operations should run on a dedicated worker thread to avoid blocking the main thread
- The interface should be thread-safe for concurrent transmission requests
- For Qt-based implementations, prefer `QNetworkAccessManager` on a dedicated network thread

## Key API (Suggested Signatures)

### C++ Interface

```cpp
class ITelemetryServer {
public:
    virtual ~ITelemetryServer() = default;
    
    // Configure server endpoint
    virtual void SetServerUrl(const QString& url) = 0;
    virtual QString GetServerUrl() const = 0;
    
    // Security configuration
    virtual void SetSslConfiguration(const QSslConfiguration& config) = 0;
    virtual QSslConfiguration GetSslConfiguration() const = 0;
    virtual bool ValidateCertificates() = 0;  // Validates client cert, checks expiration, CRL
    
    // Connection management
    virtual bool Connect() = 0;
    virtual void Disconnect() = 0;
    virtual bool IsConnected() const = 0;
    
    // Send telemetry data (asynchronous, preferred)
    virtual void SendTelemetryAsync(
        const TelemetryData& data,
        std::function<void(const ServerResponse&)> callback
    ) = 0;
    
    // Send sensor data (asynchronous)
    virtual void SendSensorDataAsync(
        const SensorData& data,
        std::function<void(const ServerResponse&)> callback
    ) = 0;
    
    // Synchronous send (blocking, use with caution)
    virtual ServerResponse SendTelemetry(const TelemetryData& data) = 0;
    
    // Check server health/availability
    virtual bool IsServerAvailable() const = 0;
    
    // Get last error message
    virtual QString GetLastError() const = 0;
    
signals:
    // Qt-style signals for async operations (alternative to callbacks)
    void telemetrySent(const TelemetryData& data, const ServerResponse& response);
    void telemetrySendFailed(const TelemetryData& data, const QString& error);
    void connectionStatusChanged(bool connected);
};
```

### Data Structures

```cpp
struct TelemetryData {
    QString deviceId;
    QString bedId;
    QDateTime timestamp;
    QList<VitalSign> vitals;
    QList<Alarm> alarms;
    QList<InfusionEvent> infusionEvents;
    QList<PredictiveScore> predictiveScores;
    QString signature;  // Digital signature for data integrity
    QString nonce;  // Nonce for replay attack prevention
};

struct SensorData {
    QString deviceId;
    QDateTime timestamp;
    QByteArray waveformData;  // ECG, pleth, etc.
    QString sensorType;
    double sampleRate;
};

struct ServerResponse {
    bool success;
    int statusCode;  // HTTP status code
    QString message;
    QList<int> processedIds;  // IDs of records successfully processed
    QDateTime serverTimestamp;
};
```

## Error Semantics

- Return `ServerResponse` with `success = false` when:
  - Network/connection error
  - Server unavailable
  - Authentication/authorization failure (mTLS failure)
  - Invalid data format
  - Server timeout
  - HTTP error status codes (4xx, 5xx)

- Use `GetLastError()` to provide detailed error messages for logging and user feedback

## Implementation Variants

### 1. NetworkTelemetryServer (Production)
- Connects to real central monitoring server via HTTPS/mTLS
- Uses `QNetworkAccessManager` for HTTP requests
- Implements retry logic with exponential backoff
- Validates server certificates and provides client certificates for mTLS
- Handles network timeouts and connection failures

### 2. MockTelemetryServer (Testing/Development)
- Swallows all data without sending to real server
- Returns success responses immediately
- Can simulate various failure scenarios (network errors, timeouts, server errors)
- Useful for unit tests and development without requiring server infrastructure
- Logs all received data for verification in tests

### 3. FileTelemetryServer (Offline Testing)
- Writes telemetry data to local files (JSON, CSV, or binary format)
- Useful for offline testing and data collection
- Can replay data later or analyze locally
- Returns success responses to maintain normal flow

## Example Code Paths

### Asynchronous Send (Preferred)
```cpp
auto server = appContainer->GetTelemetryServer();
server->SetServerUrl("https://monitoring.example.com:8443");

server->SendTelemetryAsync(telemetryData, [this](const ServerResponse& response) {
    if (response.success) {
        databaseManager->markDataAsSynced(response.processedIds);
        logService->LogInfo("Telemetry sent successfully");
    } else {
        logService->LogError("Telemetry send failed: " + response.message);
        // Retry logic handled by NetworkManager
    }
});
```

### Mock Server (Testing)
```cpp
auto mockServer = std::make_unique<MockTelemetryServer>();
mockServer->SetServerUrl("mock://localhost");  // Special URL for mock

// In tests, verify data was "sent"
mockServer->SendTelemetryAsync(testData, [](const auto& response) {
    EXPECT_TRUE(response.success);
});

// Verify mock received the data
auto receivedData = mockServer->GetLastTelemetryData();
ASSERT_EQ(receivedData.deviceId, "ZM-001");
```

### Connection Management
```cpp
auto server = networkManager->GetTelemetryServer();
server->SetServerUrl(settingsManager->GetValue("serverUrl").toString());

if (server->Connect()) {
    systemController->UpdateConnectionStatus("Connected");
} else {
    systemController->UpdateConnectionStatus("Failed: " + server->GetLastError());
}
```

## Integration with NetworkManager

`NetworkManager` should use `ITelemetryServer` for all server communication:

```cpp
class NetworkManager {
    std::unique_ptr<ITelemetryServer> telemetryServer;
    std::unique_ptr<ISettingsManager> settingsManager;
    
public:
    void Initialize() {
        // Create appropriate server implementation based on config
        if (settingsManager->GetValue("useMockServer").toBool()) {
            telemetryServer = std::make_unique<MockTelemetryServer>();
        } else {
            telemetryServer = std::make_unique<NetworkTelemetryServer>();
        }
        
        // Configure server URL from settings
        QString serverUrl = settingsManager->GetValue("serverUrl").toString();
        if (serverUrl.isEmpty()) {
            serverUrl = "https://localhost:8443";  // Default
        }
        telemetryServer->SetServerUrl(serverUrl);
    }
    
    void SendTelemetry(const TelemetryData& data) {
        telemetryServer->SendTelemetryAsync(data, [this](const auto& response) {
            HandleServerResponse(response);
        });
    }
};
```

## Server Configuration

Server URL should be configurable through `SettingsManager`:

- **Setting Key:** `serverUrl` (QString)
- **Default Value:** `"https://localhost:8443"` (for local development)
- **Format:** Full URL including protocol, host, port, and optional path
  - Examples:
    - `"https://monitoring.hospital.com:8443"`
    - `"https://localhost:8443"`
    - `"mock://localhost"` (for mock server)
- **Validation:** Must be valid URL format, must use HTTPS protocol (except mock)
- **Access Control:** Server URL configuration requires Technician role

## Tests to Write

1. **Connection Tests:**
   - Successful connection to server
   - Connection failure handling
   - Server URL validation
   - mTLS certificate validation

2. **Transmission Tests:**
   - Successful telemetry send
   - Successful sensor data send
   - Error handling (network errors, server errors)
   - Retry logic verification
   - Response processing (processed IDs)

3. **Mock Server Tests:**
   - Mock swallows data correctly
   - Mock returns success responses
   - Mock can simulate failures
   - Mock logs received data

4. **Configuration Tests:**
   - Server URL persistence
   - URL format validation
   - Default URL handling
   - Configuration change triggers reconnection

5. **Integration Tests:**
   - NetworkManager integration with ITelemetryServer
   - SettingsManager integration for server URL
   - Database sync marking after successful send
   - Error recovery and retry behavior

## Security Considerations

### Transport Security
- Server URL must use HTTPS (except for mock/local testing)
- mTLS must be enforced for production servers
- TLS 1.2 minimum, TLS 1.3 preferred
- Strong cipher suites only (ECDHE-RSA-AES256-GCM-SHA384 or better)
- Server certificates must be validated
- Client certificates must be properly configured
- Certificate expiration and revocation checking

### Authentication
- Device identity verified via client certificate
- Certificate must match device ID in settings
- Optional JWT token-based authentication after mTLS handshake
- Token refresh before expiration

### Data Integrity
- Each payload digitally signed (ECDSA or RSA)
- Signature computed over deviceId + timestamp + payload hash
- Timestamp validation to prevent replay attacks
- Clock skew tolerance: ±5 minutes
- Replay window: 1 minute

### Additional Security Measures
- Rate limiting: 60 requests per minute (configurable)
- Exponential backoff on failures
- Circuit breaker pattern for repeated failures
- Certificate pinning (optional, for additional MITM protection)
- Field-level encryption for sensitive PHI (optional)
- Security audit logging for all security events
- Server URL changes logged for audit
- Private key encryption at rest
- Secure memory handling for keys

## Development/Testing Mode

For development and testing, a `MockTelemetryServer` should be used:

- **Configuration:** Set `useMockServer=true` in settings or use `mock://` URL scheme
- **Behavior:** Accepts all data, returns immediate success, logs data for verification
- **Benefits:**
  - No server infrastructure required for development
  - Faster test execution
  - Deterministic test behavior
  - Can verify data format without network overhead

