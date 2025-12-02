# IUserManagementService Interface

**Document ID:** INTERFACE-005  
**Version:** 1.0  
**Status:** Approved  
**Last Updated:** 2025-11-27

---

## 1. Overview

The `IUserManagementService` interface provides an abstraction for authenticating healthcare workers (nurses, physicians, technicians, administrators) against a hospital user management server. This enables centralized credential management, role-based access control (RBAC), and audit compliance.

**Purpose:**
- Authenticate users (nurses, physicians, technicians) via secret codes/PINs
- Retrieve user roles and permissions from hospital server
- Support mock server for development/testing when real hospital server unavailable
- Enable centralized user management (add/remove users without device reconfiguration)

**Key Use Cases:**
- **Nurse login:** Nurse enters secret code → device queries hospital server → server returns user profile + role → device grants access
- **Physician login:** Similar flow with elevated permissions
- **Session management:** Active sessions tracked, timeout enforced
- **Development/testing:** Mock server returns test users without real hospital infrastructure

---

## 2. Interface Definition (C++)

### 2.1 Core Interface

```cpp
/**
 * @interface IUserManagementService
 * @brief Interface for hospital user authentication and authorization.
 * 
 * Provides abstraction for querying hospital user management server
 * to authenticate healthcare workers and retrieve their roles/permissions.
 * 
 * @note All methods are asynchronous and return Result<T, Error>
 * @note Implementations must support network timeouts and retry logic
 * 
 * @see HospitalUserManagementAdapter (production implementation)
 * @see MockUserManagementService (development/testing)
 * 
 * @ingroup ExternalServiceInterfaces
 */
class IUserManagementService : public QObject {
    Q_OBJECT
public:
    virtual ~IUserManagementService() = default;
    
    /**
     * @brief Authenticate user with secret code/PIN.
     * 
     * Queries hospital user management server to validate credentials
     * and retrieve user profile, role, and permissions.
     * 
     * @param userId User identifier (e.g., "NURSE001", badge ID, employee ID)
     * @param secretCode User's secret code/PIN (4-8 digits)
     * @param deviceId Device identifier (for audit trail on server)
     * @return Result<UserProfile, AuthenticationError>
     * 
     * @note Result is returned asynchronously via authenticationCompleted() signal
     * @note This method returns immediately and is non-blocking
     * 
     * @performance Target latency: < 2 seconds (network-dependent)
     * @thread Network I/O Thread
     * 
     * @see UserProfile
     * @see AuthenticationError
     * @see authenticationCompleted()
     */
    virtual void authenticate(const QString& userId, 
                            const QString& secretCode,
                            const QString& deviceId) = 0;
    
    /**
     * @brief Validate active session token.
     * 
     * Checks if session token is still valid on hospital server.
     * Used for session refresh and timeout enforcement.
     * 
     * @param sessionToken Session token issued by previous authenticate() call
     * @return Result<bool, ValidationError>
     * 
     * @note Result returned via sessionValidationCompleted() signal
     * 
     * @performance Target latency: < 1 second
     */
    virtual void validateSession(const QString& sessionToken) = 0;
    
    /**
     * @brief Logout user (invalidate session on server).
     * 
     * Notifies hospital server that user has logged out.
     * Server invalidates session token.
     * 
     * @param sessionToken Session token to invalidate
     * @param userId User ID (for audit logging)
     * 
     * @note Result returned via logoutCompleted() signal
     * 
     * @performance Target latency: < 1 second (best effort)
     */
    virtual void logout(const QString& sessionToken, 
                       const QString& userId) = 0;
    
    /**
     * @brief Get user permissions for specific action.
     * 
     * Queries server for fine-grained permission check.
     * Some implementations may cache permissions locally.
     * 
     * @param sessionToken Active session token
     * @param permission Permission to check (e.g., "ACKNOWLEDGE_ALARM", "ADJUST_THRESHOLD")
     * @return Result<bool, PermissionError>
     * 
     * @note Result returned via permissionCheckCompleted() signal
     * 
     * @performance Target latency: < 500ms (may use cached data)
     */
    virtual void checkPermission(const QString& sessionToken,
                                const QString& permission) = 0;
    
    /**
     * @brief Get list of available permissions for role.
     * 
     * Retrieves all permissions granted to user's role.
     * Used for UI adaptation (show/hide features based on permissions).
     * 
     * @param sessionToken Active session token
     * @return Result<QStringList, Error>
     * 
     * @note Result returned via permissionsRetrieved() signal
     */
    virtual void getPermissions(const QString& sessionToken) = 0;
    
    /**
     * @brief Check if service is available (health check).
     * 
     * Pings hospital server to verify connectivity.
     * 
     * @return bool true if server reachable, false otherwise
     * 
     * @note Result returned via healthCheckCompleted() signal
     * 
     * @performance Target latency: < 500ms
     */
    virtual void healthCheck() = 0;
    
signals:
    /**
     * @brief Emitted when authentication completes.
     * 
     * @param userId User ID that was authenticated
     * @param result Success: UserProfile, Failure: AuthenticationError
     */
    void authenticationCompleted(const QString& userId, 
                                 const Result<UserProfile, AuthenticationError>& result);
    
    /**
     * @brief Emitted when session validation completes.
     * 
     * @param sessionToken Session token that was validated
     * @param isValid true if session is still valid, false otherwise
     * @param error Error details if validation failed
     */
    void sessionValidationCompleted(const QString& sessionToken,
                                   bool isValid,
                                   const std::optional<ValidationError>& error);
    
    /**
     * @brief Emitted when logout completes.
     * 
     * @param sessionToken Session token that was invalidated
     * @param success true if logout successful, false otherwise
     */
    void logoutCompleted(const QString& sessionToken, bool success);
    
    /**
     * @brief Emitted when permission check completes.
     * 
     * @param permission Permission that was checked
     * @param granted true if permission granted, false otherwise
     */
    void permissionCheckCompleted(const QString& permission, bool granted);
    
    /**
     * @brief Emitted when permissions retrieval completes.
     * 
     * @param permissions List of granted permissions
     */
    void permissionsRetrieved(const QStringList& permissions);
    
    /**
     * @brief Emitted when health check completes.
     * 
     * @param available true if server is reachable, false otherwise
     * @param latencyMs Latency in milliseconds (0 if unavailable)
     */
    void healthCheckCompleted(bool available, int latencyMs);
    
    /**
     * @brief Emitted when connection to server fails.
     * 
     * @param error Error details (network timeout, connection refused, etc.)
     */
    void connectionError(const NetworkError& error);
};
```

### 2.2 Data Structures

```cpp
/**
 * @struct UserProfile
 * @brief User profile returned by successful authentication.
 */
struct UserProfile {
    QString userId;              ///< User identifier (e.g., "NURSE001")
    QString displayName;         ///< Full name for display (e.g., "Sarah Johnson, RN")
    UserRole role;               ///< Primary role (NURSE, PHYSICIAN, TECHNICIAN, ADMINISTRATOR)
    QStringList permissions;     ///< List of granted permissions
    QString sessionToken;        ///< Session token for subsequent requests
    QDateTime sessionExpiry;     ///< Session expiration time (UTC)
    QString departmentId;        ///< Department/unit ID (optional)
    QString badgeId;             ///< Physical badge ID (optional)
    QVariantMap metadata;        ///< Additional metadata (extensible)
    
    /**
     * @brief Check if user has specific permission.
     */
    bool hasPermission(const QString& permission) const {
        return permissions.contains(permission);
    }
    
    /**
     * @brief Check if session is still valid.
     */
    bool isSessionValid() const {
        return QDateTime::currentDateTimeUtc() < sessionExpiry;
    }
};

/**
 * @enum UserRole
 * @brief Predefined user roles with hierarchical permissions.
 */
enum class UserRole {
    NURSE,              ///< Registered Nurse (RN) - basic patient care
    PHYSICIAN,          ///< Medical Doctor (MD/DO) - elevated clinical permissions
    TECHNICIAN,         ///< Biomedical Technician - device configuration
    ADMINISTRATOR,      ///< System Administrator - full access
    OBSERVER,           ///< Observer/Student - read-only access
    UNKNOWN             ///< Unknown/invalid role
};

/**
 * @struct AuthenticationError
 * @brief Error details for failed authentication.
 */
struct AuthenticationError {
    enum class Reason {
        INVALID_CREDENTIALS,    ///< Wrong user ID or secret code
        ACCOUNT_LOCKED,         ///< Account locked due to too many failed attempts
        ACCOUNT_DISABLED,       ///< Account disabled by administrator
        NETWORK_ERROR,          ///< Cannot reach hospital server
        SERVER_ERROR,           ///< Server returned error
        TIMEOUT,                ///< Request timed out
        PERMISSION_DENIED,      ///< User not authorized to use this device
        LICENSE_EXPIRED,        ///< User's license/certification expired
        INVALID_DEPARTMENT      ///< User not assigned to this department/unit
    };
    
    Reason reason;               ///< Error reason
    QString message;             ///< Human-readable error message
    int remainingAttempts;       ///< Remaining login attempts before lockout (if applicable)
    QDateTime lockoutExpiry;     ///< When account lockout expires (if locked)
    QString serverMessage;       ///< Raw message from server (for debugging)
};

/**
 * @struct ValidationError
 * @brief Error details for session validation failure.
 */
struct ValidationError {
    enum class Reason {
        SESSION_EXPIRED,        ///< Session token expired
        SESSION_INVALID,        ///< Session token not found or invalid
        SESSION_REVOKED,        ///< Session revoked by administrator
        NETWORK_ERROR,          ///< Cannot reach hospital server
        SERVER_ERROR            ///< Server returned error
    };
    
    Reason reason;
    QString message;
};
```

---

## 3. Implementations

### 3.1 Production Implementation: HospitalUserManagementAdapter

**Location:** `z-monitor/src/infrastructure/authentication/HospitalUserManagementAdapter.cpp/h`

**Responsibilities:**
- Connect to hospital user management server via HTTPS
- Send authentication requests (REST API or LDAP)
- Handle network errors, timeouts, retries
- Cache user permissions locally for performance
- Log all authentication events to `security_audit_log`

**Example Implementation:**

```cpp
/**
 * @class HospitalUserManagementAdapter
 * @brief Production implementation connecting to hospital LDAP/AD server.
 * 
 * Authenticates users against hospital Active Directory or LDAP server
 * using industry-standard protocols.
 * 
 * @note Runs on Network I/O Thread
 * @thread Network I/O Thread
 */
class HospitalUserManagementAdapter : public IUserManagementService {
    Q_OBJECT
public:
    explicit HospitalUserManagementAdapter(
        const QString& serverUrl,
        CertificateManager* certManager,
        QObject* parent = nullptr);
    
    void authenticate(const QString& userId, 
                     const QString& secretCode,
                     const QString& deviceId) override;
    
    void validateSession(const QString& sessionToken) override;
    void logout(const QString& sessionToken, const QString& userId) override;
    void checkPermission(const QString& sessionToken, const QString& permission) override;
    void getPermissions(const QString& sessionToken) override;
    void healthCheck() override;
    
private:
    QString m_serverUrl;           ///< Hospital server URL (e.g., "https://hospital.example.com/api/auth")
    QNetworkAccessManager* m_nam;  ///< Network manager
    CertificateManager* m_certManager;  ///< For mTLS
    QMap<QString, UserProfile> m_sessionCache;  ///< Cached user sessions
    QMap<QString, QStringList> m_permissionCache;  ///< Cached permissions
    
    void handleAuthenticationResponse(QNetworkReply* reply, const QString& userId);
    UserProfile parseUserProfile(const QJsonObject& json);
    void cacheUserSession(const UserProfile& profile);
};
```

**Protocol Options:**

1. **REST API (Recommended):**
   - **Endpoint:** `POST /api/v1/auth/login`
   - **Request:**
     ```json
     {
       "userId": "NURSE001",
       "secretCode": "1234",
       "deviceId": "ZM-ICU-04"
     }
     ```
   - **Response (Success):**
     ```json
     {
       "status": "SUCCESS",
       "user": {
         "userId": "NURSE001",
         "displayName": "Sarah Johnson, RN",
         "role": "NURSE",
         "permissions": ["VIEW_VITALS", "ACKNOWLEDGE_ALARM", "ADMIT_PATIENT"],
         "sessionToken": "eyJhbGciOiJIUzI1NiIsInR5cCI6IkpXVCJ9...",
         "sessionExpiry": "2025-11-27T20:00:00Z",
         "departmentId": "ICU",
         "badgeId": "12345"
       }
     }
     ```
   - **Response (Failure):**
     ```json
     {
       "status": "ERROR",
       "error": {
         "reason": "INVALID_CREDENTIALS",
         "message": "Invalid user ID or secret code",
         "remainingAttempts": 2
       }
     }
     ```

2. **LDAP/Active Directory (Alternative):**
   - **Protocol:** LDAP bind operation
   - **Server:** `ldap://hospital.example.com:389` or `ldaps://hospital.example.com:636`
   - **Bind DN:** `cn=NURSE001,ou=Nurses,dc=hospital,dc=example,dc=com`
   - **Search:** Query for user attributes (role, department, permissions)

### 3.2 Development/Testing Implementation: MockUserManagementService

**Location:** `z-monitor/src/infrastructure/authentication/MockUserManagementService.cpp/h`

**Responsibilities:**
- Provide hardcoded test users (nurses, physicians, technicians, admin)
- Simulate network latency (configurable delay)
- Simulate failures (invalid credentials, network timeout, server error)
- No external server required (works offline)

**Example Implementation:**

```cpp
/**
 * @class MockUserManagementService
 * @brief Mock implementation for development and testing.
 * 
 * Returns hardcoded test users without requiring real hospital server.
 * Useful for:
 * - Development without hospital infrastructure
 * - Automated testing (unit tests, integration tests)
 * - Demonstrations and training
 * 
 * @note Does NOT connect to any network
 * @thread Any thread (uses QTimer for async simulation)
 */
class MockUserManagementService : public IUserManagementService {
    Q_OBJECT
public:
    explicit MockUserManagementService(QObject* parent = nullptr);
    
    void authenticate(const QString& userId, 
                     const QString& secretCode,
                     const QString& deviceId) override;
    
    void validateSession(const QString& sessionToken) override;
    void logout(const QString& sessionToken, const QString& userId) override;
    void checkPermission(const QString& sessionToken, const QString& permission) override;
    void getPermissions(const QString& sessionToken) override;
    void healthCheck() override;
    
    /**
     * @brief Set simulated network latency (for testing).
     * @param latencyMs Latency in milliseconds (default: 500ms)
     */
    void setSimulatedLatency(int latencyMs) { m_simulatedLatency = latencyMs; }
    
    /**
     * @brief Enable/disable simulated failures (for testing error handling).
     * @param enabled If true, 20% of requests will fail randomly
     */
    void setSimulateFailures(bool enabled) { m_simulateFailures = enabled; }
    
private:
    QMap<QString, UserProfile> m_testUsers;  ///< Hardcoded test users
    QMap<QString, UserProfile> m_activeSessions;  ///< Active mock sessions
    int m_simulatedLatency = 500;  ///< Simulated network latency (ms)
    bool m_simulateFailures = false;  ///< Simulate random failures
    
    void initializeTestUsers();
    void simulateAsyncResponse(std::function<void()> callback);
};
```

**Hardcoded Test Users:**

```cpp
void MockUserManagementService::initializeTestUsers() {
    // Test Nurse
    UserProfile nurse;
    nurse.userId = "NURSE001";
    nurse.displayName = "Sarah Johnson, RN";
    nurse.role = UserRole::NURSE;
    nurse.permissions = {
        "VIEW_VITALS", "VIEW_WAVEFORMS", "VIEW_ALARMS",
        "ACKNOWLEDGE_ALARM", "SILENCE_ALARM",
        "ADMIT_PATIENT", "DISCHARGE_PATIENT", "TRANSFER_PATIENT",
        "VIEW_TRENDS", "VIEW_PATIENT_DATA"
    };
    nurse.sessionToken = "mock-nurse-token-001";
    nurse.sessionExpiry = QDateTime::currentDateTimeUtc().addSecs(3600);  // 1 hour
    nurse.departmentId = "ICU";
    nurse.badgeId = "12345";
    m_testUsers["NURSE001:1234"] = nurse;  // userId:secretCode
    
    // Test Physician
    UserProfile physician;
    physician.userId = "PHYSICIAN001";
    physician.displayName = "Dr. Michael Chen, MD";
    physician.role = UserRole::PHYSICIAN;
    physician.permissions = {
        // All nurse permissions +
        "ADJUST_ALARM_THRESHOLDS", "OVERRIDE_ALARM",
        "ACCESS_ADVANCED_SETTINGS", "EXPORT_DATA",
        "VIEW_ALL_PATIENTS"
    };
    physician.sessionToken = "mock-physician-token-001";
    physician.sessionExpiry = QDateTime::currentDateTimeUtc().addSecs(7200);  // 2 hours
    physician.departmentId = "ICU";
    physician.badgeId = "67890";
    m_testUsers["PHYSICIAN001:5678"] = physician;
    
    // Test Technician
    UserProfile technician;
    technician.userId = "TECH001";
    technician.displayName = "James Smith, BMET";
    technician.role = UserRole::TECHNICIAN;
    technician.permissions = {
        "VIEW_DIAGNOSTICS", "ACCESS_SYSTEM_SETTINGS",
        "CONFIGURE_DEVICE", "ENTER_PROVISIONING_MODE",
        "VIEW_LOGS", "EXPORT_LOGS",
        "RESET_ALARMS", "CALIBRATE_DEVICE"
    };
    technician.sessionToken = "mock-tech-token-001";
    technician.sessionExpiry = QDateTime::currentDateTimeUtc().addSecs(3600);
    technician.departmentId = "BIOMED";
    technician.badgeId = "99999";
    m_testUsers["TECH001:9999"] = technician;
    
    // Test Administrator
    UserProfile admin;
    admin.userId = "ADMIN001";
    admin.displayName = "System Administrator";
    admin.role = UserRole::ADMINISTRATOR;
    admin.permissions = {
        // All permissions
        "MANAGE_USERS", "VIEW_AUDIT_LOGS", "MANAGE_SETTINGS",
        "FACTORY_RESET", "UPDATE_FIRMWARE"
    };
    admin.sessionToken = "mock-admin-token-001";
    admin.sessionExpiry = QDateTime::currentDateTimeUtc().addSecs(3600);
    admin.badgeId = "00000";
    m_testUsers["ADMIN001:0000"] = admin;
}
```

---

## 4. Usage Examples

### 4.1 Login Flow (Production)

```cpp
// In AuthenticationController or SecurityService
void SecurityService::login(const QString& userId, const QString& secretCode) {
    // Show loading indicator
    emit loginInProgress();
    
    // Query hospital server
    QString deviceId = SettingsManager::instance()->getDeviceLabel();
    m_userMgmtService->authenticate(userId, secretCode, deviceId);
    
    // Result will arrive via signal (asynchronous)
}

// Slot connected to IUserManagementService::authenticationCompleted()
void SecurityService::onAuthenticationCompleted(
    const QString& userId,
    const Result<UserProfile, AuthenticationError>& result)
{
    if (result.isSuccess()) {
        UserProfile profile = result.value();
        
        // Create local session
        m_currentSession = UserSession(profile);
        
        // Log successful login
        logAuditEvent("USER_LOGIN", profile.userId, profile.role);
        
        // Emit success signal
        emit userLoggedIn(profile.userId, profile.role);
        
        // Update UI
        emit loginSuccessful(profile.displayName);
        
    } else {
        AuthenticationError error = result.error();
        
        // Log failed login
        logAuditEvent("LOGIN_FAILED", userId, error.reason);
        
        // Emit failure signal
        emit loginFailed(userId, error.message, error.remainingAttempts);
        
        // Show error to user
        if (error.reason == AuthenticationError::Reason::ACCOUNT_LOCKED) {
            showError("Account locked until " + error.lockoutExpiry.toString());
        } else {
            showError(error.message);
        }
    }
}
```

### 4.2 Permission Check

```cpp
// Before performing sensitive action
void AlarmController::acknowledgeAlarm(const QString& alarmId) {
    // Check if user has permission
    QString sessionToken = SecurityService::instance()->getSessionToken();
    m_userMgmtService->checkPermission(sessionToken, "ACKNOWLEDGE_ALARM");
    
    // Result will arrive via signal
    // Actual acknowledgment performed in onPermissionCheckCompleted()
}

void AlarmController::onPermissionCheckCompleted(const QString& permission, bool granted) {
    if (permission == "ACKNOWLEDGE_ALARM") {
        if (granted) {
            // Perform action
            m_alarmService->acknowledgeAlarm(m_pendingAlarmId);
        } else {
            // Show permission denied error
            showError("You do not have permission to acknowledge alarms");
        }
    }
}
```

### 4.3 Session Timeout Handling

```cpp
// In SecurityService - check session periodically
void SecurityService::checkSessionValidity() {
    if (!m_currentSession.has_value()) {
        return;  // No active session
    }
    
    // Check if session expired locally
    if (!m_currentSession->isSessionValid()) {
        handleSessionExpired("local");
        return;
    }
    
    // Validate with server (every 5 minutes)
    m_userMgmtService->validateSession(m_currentSession->sessionToken);
}

void SecurityService::onSessionValidationCompleted(
    const QString& sessionToken,
    bool isValid,
    const std::optional<ValidationError>& error)
{
    if (!isValid) {
        if (error.has_value() && error->reason == ValidationError::Reason::SESSION_REVOKED) {
            handleSessionExpired("revoked");
        } else {
            handleSessionExpired("expired");
        }
    }
}

void SecurityService::handleSessionExpired(const QString& reason) {
    // Clear current session
    m_currentSession = std::nullopt;
    
    // Log event
    logAuditEvent("SESSION_EXPIRED", reason);
    
    // Emit signal
    emit sessionExpired();
    
    // Return to login screen
    emit requestLoginScreen();
}
```

---

## 5. Error Handling

### 5.1 Network Errors

**Scenario:** Hospital server is unreachable

**Handling:**
- Show error message: "Cannot connect to hospital server"
- Provide retry option
- Allow administrator to switch to "Offline Mode" (local authentication fallback)
- Log network error to `security_audit_log`

### 5.2 Authentication Failures

**Scenario:** Invalid credentials

**Handling:**
- Show error: "Invalid user ID or secret code. 2 attempts remaining."
- Increment failed attempt counter
- After 3 failed attempts: lock account for 15 minutes
- Log all failed attempts to `security_audit_log`

### 5.3 Session Expiry

**Scenario:** Session token expired

**Handling:**
- Show warning: "Your session has expired. Please log in again."
- Save current work (if possible)
- Return to login screen
- Preserve unsent telemetry data

---

## 6. Security Considerations

### 6.1 Secret Code Transmission

**Protection:**
- **HTTPS with TLS 1.3:** Encrypt secret code in transit
- **Certificate Pinning:** Prevent man-in-the-middle attacks
- **No Logging:** Never log secret codes in plaintext
- **Memory Protection:** Clear secret code from memory after use

### 6.2 Session Token Security

**Protection:**
- **Short Expiry:** 1-2 hours maximum
- **Secure Storage:** Store token in memory only (not disk)
- **Token Revocation:** Support server-side revocation
- **Single Device:** Token tied to specific device ID

### 6.3 Audit Logging

**Required Events:**
- All login attempts (success and failure)
- All logout events
- Session expiry events
- Permission checks (success and denial)
- Account lockouts
- Server connection failures

---

## 7. Configuration

### 7.1 Settings

**Location:** `settings` table or configuration file

| Setting | Type | Default | Description |
|---------|------|---------|-------------|
| `user_mgmt_server_url` | String | `""` (empty = use mock) | Hospital user management server URL |
| `user_mgmt_use_mock` | Boolean | `true` | Use mock service (for development) |
| `session_timeout_minutes` | Integer | `60` | Session timeout in minutes |
| `max_failed_attempts` | Integer | `3` | Max failed login attempts before lockout |
| `lockout_duration_minutes` | Integer | `15` | Account lockout duration |
| `session_validation_interval` | Integer | `300` | Session validation interval (seconds) |
| `network_timeout_seconds` | Integer | `10` | Network request timeout |

### 7.2 Switching Between Mock and Production

```cpp
// In main.cpp or application initialization
void Application::initializeUserManagementService() {
    bool useMock = SettingsManager::instance()->getValue("user_mgmt_use_mock", true).toBool();
    
    if (useMock) {
        m_logService->info("Using MOCK user management service (development mode)", {});
        m_userMgmtService = new MockUserManagementService(this);
    } else {
        QString serverUrl = SettingsManager::instance()->getValue("user_mgmt_server_url").toString();
        m_logService->info("Using PRODUCTION user management service", {
            {"serverUrl", serverUrl}
        });
        m_userMgmtService = new HospitalUserManagementAdapter(serverUrl, m_certManager, this);
    }
    
    // Inject into SecurityService
    m_securityService = new SecurityService(m_userMgmtService, this);
}
```

---

## 8. Testing Strategy

### 8.1 Unit Tests

```cpp
TEST_F(UserManagementServiceTest, Authenticate_ValidCredentials_ReturnsUserProfile) {
    // Arrange
    MockUserManagementService service;
    QSignalSpy spy(&service, &IUserManagementService::authenticationCompleted);
    
    // Act
    service.authenticate("NURSE001", "1234", "ZM-TEST-01");
    
    // Assert
    ASSERT_EQ(spy.count(), 1);
    auto result = spy.at(0).at(1).value<Result<UserProfile, AuthenticationError>>();
    ASSERT_TRUE(result.isSuccess());
    EXPECT_EQ(result.value().userId, "NURSE001");
    EXPECT_EQ(result.value().role, UserRole::NURSE);
}

TEST_F(UserManagementServiceTest, Authenticate_InvalidCredentials_ReturnsError) {
    // Arrange
    MockUserManagementService service;
    QSignalSpy spy(&service, &IUserManagementService::authenticationCompleted);
    
    // Act
    service.authenticate("NURSE001", "wrong", "ZM-TEST-01");
    
    // Assert
    ASSERT_EQ(spy.count(), 1);
    auto result = spy.at(0).at(1).value<Result<UserProfile, AuthenticationError>>();
    ASSERT_FALSE(result.isSuccess());
    EXPECT_EQ(result.error().reason, AuthenticationError::Reason::INVALID_CREDENTIALS);
}
```

### 8.2 Integration Tests

```cpp
TEST_F(SecurityServiceIntegrationTest, LoginWorkflow_ValidCredentials_CreatesSession) {
    // Arrange
    MockUserManagementService mockService;
    SecurityService security(&mockService);
    
    // Act
    security.login("NURSE001", "1234");
    QTest::qWait(600);  // Wait for async response
    
    // Assert
    ASSERT_TRUE(security.hasActiveSession());
    EXPECT_EQ(security.getCurrentUserId(), "NURSE001");
    EXPECT_EQ(security.getCurrentRole(), UserRole::NURSE);
}
```

### 8.3 Mock Server Simulation

For integration testing without real hospital server:

```bash
# Run mock HTTP server (Python)
cd project-dashboard/mock-servers
python3 user_management_mock_server.py --port 8080

# Configure z-monitor to use mock server
z-monitor --user-mgmt-server http://localhost:8080/api/auth
```

---

## 9. Performance Requirements

| Operation | Target Latency | Priority |
|-----------|---------------|----------|
| `authenticate()` | < 2 seconds | High |
| `validateSession()` | < 1 second | Medium |
| `logout()` | < 1 second | Low (best effort) |
| `checkPermission()` | < 500 ms | High (may use cache) |
| `healthCheck()` | < 500 ms | Low |

**Cache Strategy:**
- Cache user permissions locally for 5 minutes
- Cache session validation for 5 minutes
- Invalidate cache on logout or session expiry

---

## 10. Related Documents

- **Requirements:** [08_SECURITY_REQUIREMENTS.md](../../requirements/08_SECURITY_REQUIREMENTS.md) - REQ-SEC-AUTH-001, REQ-SEC-AUTHZ-001
- **Design:** [09_CLASS_DESIGNS.md](../09_CLASS_DESIGNS.md) - SecurityService, AuthenticationService
- **Thread Model:** [12_THREAD_MODEL.md](../12_THREAD_MODEL.md) - Network I/O Thread
- **System Components:** [29_SYSTEM_COMPONENTS.md](../29_SYSTEM_COMPONENTS.md) - Authentication components
- **Other Interfaces:**
  - [IPatientLookupService.md](./IPatientLookupService.md) - Similar external service pattern
  - [ITelemetryServer.md](./ITelemetryServer.md) - Similar network service pattern

---

**Document Version:** 1.0  
**Status:** Draft - Ready for Review  
**Last Updated:** 2025-11-27

*This interface enables centralized hospital user management with support for development/testing via mock service.*

