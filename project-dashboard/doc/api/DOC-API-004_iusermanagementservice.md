---
doc_id: DOC-API-004
title: IUserManagementService Interface
version: 1.0
category: API
subcategory: Authentication
status: Approved
created: 2025-11-27
updated: 2025-11-27
tags: [api, interface, authentication, authorization, rbac, hospital-server, user-management]
related_docs:
  - DOC-COMP-011 # SecurityService
  - DOC-PROC-003 # Authentication Workflow
  - DOC-PROC-004 # Login Workflow
  - DOC-ARCH-002 # Architecture Patterns
authors:
  - Z Monitor Team
reviewers:
  - Architecture Team
---

# IUserManagementService Interface

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
- **Session management:** Active sessions tracked, timeout enforced (1-2 hours)
- **Development/testing:** Mock server returns test users without real hospital infrastructure

---

## 2. Interface Definition

### 2.1 Core Interface

```cpp
/**
 * @interface IUserManagementService
 * @brief Interface for hospital user authentication and authorization.
 * 
 * Provides abstraction for querying hospital user management server
 * to authenticate healthcare workers and retrieve their roles/permissions.
 * 
 * @note All methods are asynchronous and return results via signals
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
     * @param userId User identifier (e.g., "NURSE001", badge ID)
     * @param secretCode User's secret code/PIN (4-8 digits)
     * @param deviceId Device identifier (for audit trail on server)
     * 
     * @note Result is returned asynchronously via authenticationCompleted() signal
     * @note Target latency: < 2 seconds (network-dependent)
     * @thread Network I/O Thread
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
     * 
     * @note Result returned via sessionValidationCompleted() signal
     * @note Target latency: < 1 second
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
     * @note Target latency: < 1 second (best effort)
     */
    virtual void logout(const QString& sessionToken, 
                       const QString& userId) = 0;
    
    /**
     * @brief Check user permission for specific action.
     * 
     * Queries server for fine-grained permission check.
     * Some implementations may cache permissions locally.
     * 
     * @param sessionToken Active session token
     * @param permission Permission to check (e.g., "ACKNOWLEDGE_ALARM")
     * 
     * @note Result returned via permissionCheckCompleted() signal
     * @note Target latency: < 500ms (may use cached data)
     */
    virtual void checkPermission(const QString& sessionToken,
                                const QString& permission) = 0;
    
    /**
     * @brief Get list of available permissions for user.
     * 
     * Retrieves all permissions granted to user's role.
     * Used for UI adaptation (show/hide features based on permissions).
     * 
     * @param sessionToken Active session token
     * 
     * @note Result returned via permissionsRetrieved() signal
     */
    virtual void getPermissions(const QString& sessionToken) = 0;
    
    /**
     * @brief Check if service is available (health check).
     * 
     * Pings hospital server to verify connectivity.
     * 
     * @note Result returned via healthCheckCompleted() signal
     * @note Target latency: < 500ms
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
     * @param isValid true if session is still valid
     * @param error Error details if validation failed
     */
    void sessionValidationCompleted(const QString& sessionToken,
                                   bool isValid,
                                   const std::optional<ValidationError>& error);
    
    /**
     * @brief Emitted when logout completes.
     * 
     * @param sessionToken Session token that was invalidated
     * @param success true if logout successful
     */
    void logoutCompleted(const QString& sessionToken, bool success);
    
    /**
     * @brief Emitted when permission check completes.
     * 
     * @param permission Permission that was checked
     * @param granted true if permission granted
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
     * @param available true if server is reachable
     * @param latencyMs Latency in milliseconds (0 if unavailable)
     */
    void healthCheckCompleted(bool available, int latencyMs);
    
    /**
     * @brief Emitted when connection to server fails.
     * 
     * @param error Error details
     */
    void connectionError(const NetworkError& error);
};
```

---

## 3. Data Structures

### 3.1 UserProfile

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
    
    bool hasPermission(const QString& permission) const {
        return permissions.contains(permission);
    }
    
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
```

### 3.2 AuthenticationError

```cpp
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
    int remainingAttempts;       ///< Remaining login attempts before lockout
    QDateTime lockoutExpiry;     ///< When account lockout expires (if locked)
};
```

---

## 4. Implementations

### 4.1 HospitalUserManagementAdapter (Production)

**Technology:** HTTPS REST API or LDAP/Active Directory

**Features:**
- Connects to hospital user management server
- Supports REST API or LDAP/AD authentication
- Caches user permissions locally (5-minute TTL)
- Logs all authentication events to security_audit_log

### 4.2 MockUserManagementService (Development/Testing)

**Technology:** In-memory map

**Features:**
- Hardcoded test users (nurse, physician, technician, admin)
- Simulated network latency (configurable 0-500ms)
- Simulated failure modes for testing error handling
- No external server required (works offline)

**Hardcoded Test Users:**
- **NURSE001:1234** → Sarah Johnson, RN (Nurse permissions)
- **PHYSICIAN001:5678** → Dr. Michael Chen, MD (Physician permissions)
- **TECH001:9999** → James Smith, BMET (Technician permissions)
- **ADMIN001:0000** → System Administrator (Full permissions)

---

## 5. Usage Example

```cpp
// In SecurityService - handle login
void SecurityService::login(const QString& userId, const QString& secretCode) {
    QString deviceId = Settings::instance()->deviceLabel();
    m_userMgmtService->authenticate(userId, secretCode, deviceId);
}

// Slot connected to authenticationCompleted() signal
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
        
    } else {
        AuthenticationError error = result.error();
        
        // Log failed login
        logAuditEvent("LOGIN_FAILED", userId, error.reason);
        
        // Show error to user
        if (error.reason == AuthenticationError::Reason::ACCOUNT_LOCKED) {
            showError("Account locked until " + error.lockoutExpiry.toString());
        } else {
            showError(error.message);
        }
    }
}
```

---

## 6. Security Considerations

### 6.1 Secret Code Transmission

- **HTTPS with TLS 1.3:** Encrypt secret code in transit
- **Certificate Pinning:** Prevent man-in-the-middle attacks
- **No Logging:** Never log secret codes in plaintext
- **Memory Protection:** Clear secret code from memory after use

### 6.2 Session Token Security

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

| Setting                    | Type    | Default                 | Description                              |
| -------------------------- | ------- | ----------------------- | ---------------------------------------- |
| `user_mgmt_server_url`     | String  | `""` (empty = use mock) | Hospital user management server URL      |
| `user_mgmt_use_mock`       | Boolean | `true`                  | Use mock service (for development)       |
| `session_timeout_minutes`  | Integer | `60`                    | Session timeout in minutes               |
| `max_failed_attempts`      | Integer | `3`                     | Max failed login attempts before lockout |
| `lockout_duration_minutes` | Integer | `15`                    | Account lockout duration                 |

---

## 8. Related Documents

- **DOC-COMP-011:** SecurityService - Uses this interface for authentication
- **DOC-PROC-003:** Authentication Workflow - Complete authentication flow
- **DOC-PROC-004:** Login Workflow - Login UI and action logging
- **DOC-ARCH-002:** Architecture Patterns - Infrastructure layer interfaces

---

## 9. Changelog

| Version | Date       | Author         | Changes                                        |
| ------- | ---------- | -------------- | ---------------------------------------------- |
| 1.0     | 2025-11-27 | Z Monitor Team | Migrated from INTERFACE-005, added frontmatter |

---

*This interface enables centralized hospital user management with support for development/testing via mock service, RBAC enforcement, and comprehensive audit logging.*
