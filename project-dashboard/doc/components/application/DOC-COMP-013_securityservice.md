---
doc_id: DOC-COMP-013
title: SecurityService
version: v1.0
category: Component
subcategory: Application Layer / Use Case Orchestration
status: Draft
owner: Application Layer Team
reviewers: 
  - Architecture Team
  - Security Team
last_reviewed: 2025-01-26
next_review: 2026-01-26
related_docs:
  - DOC-ARCH-002  # System architecture
related_tasks:
  - TASK-3B-001  # Phase 3B Migration
related_requirements:
  - REQ-FUN-USER-001  # User authentication
  - REQ-FUN-USER-003  # Inactivity timeout
tags:
  - application-service
  - authentication
  - authorization
  - session-management
  - rbac
  - security
diagram_files:
  - DOC-COMP-013_session_lifecycle.mmd
  - DOC-COMP-013_session_lifecycle.svg
---

# DOC-COMP-013: SecurityService

## 1. Overview

**Purpose:** Application service orchestrating user authentication, authorization (RBAC), session management, and inactivity timeout enforcement (15-minute auto-logout per REQ-FUN-USER-003).

**Responsibilities:**
- Authenticate users via IUserManagementService (secret code/PIN)
- Manage user session lifecycle (creation, validation, expiration)
- Enforce inactivity timeout (15 minutes, with 14-minute warning per requirement)
- Check user permissions via PermissionRegistry (RBAC enforcement)
- Log authentication events to IAuditRepository for security auditing
- Log user actions to IActionLogRepository for traceability
- Emit Qt signals for login success/failure, session expiration, inactivity warnings

**Layer:** Application Layer

**Module:** `z-monitor/src/application/services/SecurityService.h`

**Thread Affinity:** Application Services Thread (normal priority)

**Dependencies:**
- **Infrastructure Services:** IUserManagementService (authentication), SettingsManager (configuration)
- **Repository Interfaces:** IAuditRepository (security events), IActionLogRepository (user actions)
- **Security:** PermissionRegistry (RBAC), UserRole enum, Permission enum
- **Session:** UserSession struct (profile, timestamps, expiry)

## 2. Architecture

<!-- TODO: Add session lifecycle diagram -->

**Key Design Decisions:**
- **Decision 1: 15-Minute Inactivity Timeout** - Auto-logout after 15 minutes of inactivity (REQ-FUN-USER-003), with 14-minute warning signal for UI notification
- **Decision 2: Asynchronous Authentication** - `login()` is non-blocking, returns immediately; result delivered via `userLoggedIn()` or `loginFailed()` signals
- **Decision 3: Permission Check Local** - `hasPermission()` checks local PermissionRegistry (no server call), instant response for UI permission checks
- **Decision 4: Session Monitoring** - QTimer polls session validity every 5 minutes (300 seconds) to detect server-side session revocations

**Design Patterns Used:**
- **Application Service Pattern:** Coordinates authentication/authorization without business logic
- **Observer Pattern (Qt Signals/Slots):** Emits signals for login events, session expiration, inactivity warnings
- **Dependency Injection:** Constructor injection of IUserManagementService, repositories, SettingsManager
- **Strategy Pattern:** Production (server-based) vs Mock (in-memory) IUserManagementService implementations

## 3. Public API

### 3.1 Key Class

```cpp
class SecurityService : public QObject {
    Q_OBJECT

public:
    /**
     * @brief Constructor with dependency injection.
     */
    explicit SecurityService(
        IUserManagementService* userMgmtService,
        IAuditRepository* auditRepo,
        IActionLogRepository* actionLogRepo,
        SettingsManager* settingsManager,
        QObject* parent = nullptr);

    // === Authentication ===

    /**
     * @brief Authenticate user with secret code/PIN.
     * Result returned asynchronously via userLoggedIn() or loginFailed() signals.
     */
    void login(const QString& userId, const QString& secretCode);

    /**
     * @brief Logout current user.
     * Invalidates session on server and clears local session.
     */
    void logout();

    // === Authorization (RBAC) ===

    /**
     * @brief Check if user has specific permission.
     * @param permission Permission to check (e.g., Permission::VIEW_VITALS)
     * @return true if user has permission, false otherwise
     */
    bool hasPermission(Permission permission) const;

    /**
     * @brief Check if user has permission by string.
     * @param permissionStr Permission string (e.g., "VIEW_VITALS")
     * @return true if user has permission, false otherwise
     */
    bool hasPermission(const QString& permissionStr) const;

    // === Session Management ===

    /**
     * @brief Get current user ID.
     * @return Current user ID, or empty string if no active session
     */
    QString getCurrentUserId() const;

    /**
     * @brief Get current user role.
     * @return Current user role, or UserRole::Count if no active session
     */
    UserRole getCurrentRole() const;

    /**
     * @brief Check if user is logged in.
     * @return true if active session exists, false otherwise
     */
    bool isLoggedIn() const;

    /**
     * @brief Refresh session activity timestamp.
     * Updates last activity time to prevent session timeout.
     * Should be called on user interactions.
     */
    void refreshActivity();

    /**
     * @brief Record a configuration action.
     * Resets inactivity timer and logs action to IActionLogRepository.
     */
    void recordConfigurationAction(const QString& actionType,
                                   const QString& targetType = QString(),
                                   const QString& targetId = QString(),
                                   const QJsonObject& details = QJsonObject());

signals:
    /**
     * @brief Emitted when user successfully logs in.
     */
    void userLoggedIn(const QString& userId, UserRole role, const QString& displayName);

    /**
     * @brief Emitted when login fails.
     * @param remainingAttempts Remaining login attempts before lockout
     */
    void loginFailed(const QString& userId, const QString& errorMessage, int remainingAttempts);

    /**
     * @brief Emitted when user logs out.
     */
    void userLoggedOut();

    /**
     * @brief Emitted when session expires.
     */
    void sessionExpired(const QString& reason);

    /**
     * @brief Emitted when inactivity warning should be shown (14 minutes).
     */
    void inactivityWarning();

    /**
     * @brief Emitted when auto-logout is about to occur (15 minutes).
     */
    void autoLogoutImminent();

private slots:
    void onSessionMonitoringTimeout();      // Check session validity (5-minute intervals)
    void onInactivityTimeout();             // Auto-logout (15 minutes)
    void onInactivityWarning();             // Warning (14 minutes)

private:
    IUserManagementService* m_userMgmtService;        ///< User management service
    IAuditRepository* m_auditRepo;                    ///< Audit repository
    IActionLogRepository* m_actionLogRepo;            ///< Action log repository
    SettingsManager* m_settingsManager;                ///< Settings manager
    std::optional<UserSession> m_currentSession;       ///< Current active session
    QTimer* m_sessionMonitoringTimer;                  ///< Timer for session validation (5 min)
    QTimer* m_inactivityTimer;                        ///< Timer for inactivity auto-logout (15 min)
    QTimer* m_inactivityWarningTimer;                 ///< Timer for inactivity warning (14 min)
    int m_sessionTimeoutMinutes = 60;                  ///< Session timeout (default: 60 minutes)
    int m_inactivityTimeoutMinutes = 15;              ///< Inactivity timeout (REQ-FUN-USER-003)
};
```

### 3.2 UserSession Structure

```cpp
struct UserSession {
    UserProfile userProfile;          ///< User profile (userId, role, displayName)
    QDateTime createdAt;             ///< Session creation time
    QDateTime lastActivityTime;       ///< Last activity timestamp
    QDateTime expiresAt;              ///< Session expiration time

    bool isValid() const { return QDateTime::currentDateTimeUtc() < expiresAt; }
    bool isExpired() const { return !isValid(); }
};
```

## 4. Implementation Highlights

**Login Flow:**
1. Call `IUserManagementService->authenticateUser(userId, secretCode)`
2. On success: Create UserSession, start inactivity timer, emit `userLoggedIn()`
3. On failure: Emit `loginFailed()` with error message and remaining attempts

**Inactivity Timeout Enforcement:**
1. Start 15-minute inactivity timer on login
2. Reset timer on `refreshActivity()` or `recordConfigurationAction()`
3. Emit `inactivityWarning()` at 14 minutes
4. Emit `autoLogoutImminent()` at 15 minutes, then logout

**Permission Check:**
```cpp
bool SecurityService::hasPermission(Permission permission) const {
    if (!m_currentSession.has_value()) return false;
    UserRole role = m_currentSession->userProfile.role;
    return PermissionRegistry::hasPermission(role, permission);
}
```

## 5. Usage Examples

### 5.1 Login

```cpp
// Connect signals
connect(securityService, &SecurityService::userLoggedIn, this, &LoginController::onLoginSuccess);
connect(securityService, &SecurityService::loginFailed, this, &LoginController::onLoginFailed);

// Initiate login (non-blocking)
securityService->login("nurse-smith", "123456");

// Result delivered via signals
```

### 5.2 Permission Check

```cpp
if (securityService->hasPermission(Permission::ADMIT_PATIENT)) {
    // Show admit patient UI
} else {
    // Hide admit patient button
}
```

### 5.3 Refresh Activity on User Interaction

```cpp
// Call on button click, screen touch, etc.
securityService->refreshActivity();
```

## 6. Testing

**Unit Tests:**
- `test_Login_Success()` - Verify successful login creates session and emits signal
- `test_Login_InvalidCredentials_EmitsFailedSignal()` - Verify login failure emits loginFailed signal
- `test_HasPermission_ValidRole_ReturnsTrue()` - Verify permission check for valid role
- `test_InactivityTimeout_LogsOutAfter15Minutes()` - Verify auto-logout after 15 minutes
- `test_RefreshActivity_ResetsInactivityTimer()` - Verify refreshActivity() resets timer

**Integration Tests:**
- End-to-end login → permission check → inactivity warning → auto-logout flow
- Audit logging verification (login success/failure events)

## 7. Performance & Security

**Performance:**
- `hasPermission()`: O(1) lookup in PermissionRegistry (instant)
- `login()`: Non-blocking (async IUserManagementService call)
- Session validation: 5-minute interval (minimal overhead)

**Security:**
- Inactivity timeout: 15 minutes (REQ-FUN-USER-003)
- Session expiry: 60 minutes (configurable via SettingsManager)
- Audit logging: All authentication events logged (login success/failure, logout, session expiration)
- Action logging: All configuration actions logged (ADMIT_PATIENT, CHANGE_SETTING, etc.)

## 8. Related Documentation

- User Role and Permission Documentation - RBAC specification
- IUserManagementService Interface - Authentication service contract
- REQ-FUN-USER-003 - Inactivity timeout requirement

## 9. Changelog

| Version | Date       | Author      | Changes                                      |
| ------- | ---------- | ----------- | -------------------------------------------- |
| v1.0    | 2025-01-26 | Dustin Wind | Initial documentation from SecurityService.h |
