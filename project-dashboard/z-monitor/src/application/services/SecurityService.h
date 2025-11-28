/**
 * @file SecurityService.h
 * @brief Service for user authentication, authorization, and session management.
 *
 * This file contains the SecurityService class which orchestrates user
 * authentication, session management, permission checking, and RBAC enforcement.
 *
 * @author Z Monitor Team
 * @date 2025-01-15
 */

#pragma once

#include <QObject>
#include <QString>
#include <QDateTime>
#include <QTimer>
#include <optional>
#include <memory>

#include "domain/interfaces/IUserManagementService.h"
#include "domain/security/UserRole.h"
#include "domain/security/Permission.h"
#include "domain/repositories/IAuditRepository.h"

namespace zmon {

// Forward declarations
class SettingsManager;

/**
 * @struct UserSession
 * @brief Represents an active user session.
 */
struct UserSession {
    UserProfile userProfile;          ///< User profile from authentication
    QDateTime createdAt;             ///< Session creation time
    QDateTime lastActivityTime;       ///< Last activity timestamp
    QDateTime expiresAt;              ///< Session expiration time

    /**
     * @brief Check if session is still valid.
     */
    bool isValid() const {
        return QDateTime::currentDateTimeUtc() < expiresAt;
    }

    /**
     * @brief Check if session is expired.
     */
    bool isExpired() const {
        return !isValid();
    }
};

/**
 * @class SecurityService
 * @brief Service for user authentication, authorization, and session management.
 *
 * Orchestrates:
 * - User authentication via IUserManagementService
 * - Session lifecycle management
 * - Permission checking and RBAC enforcement
 * - Session timeout handling
 * - Audit logging of authentication events
 *
 * @note Thread-safe: Can be called from any thread (uses queued connections)
 * @note Runs on Application Services Thread
 * @thread Application Services Thread
 * @ingroup Application
 */
class SecurityService : public QObject {
    Q_OBJECT

public:
    /**
     * @brief Constructor.
     *
     * @param userMgmtService User management service (mock or production)
     * @param auditRepo Audit repository for logging authentication events
     * @param settingsManager Settings manager for configuration
     * @param parent Parent QObject (for Qt parent-child ownership)
     */
    explicit SecurityService(
        IUserManagementService* userMgmtService,
        IAuditRepository* auditRepo,
        SettingsManager* settingsManager,
        QObject* parent = nullptr);

    /**
     * @brief Destructor.
     */
    ~SecurityService() override = default;

    /**
     * @brief Authenticate user with secret code/PIN.
     *
     * Initiates authentication via IUserManagementService.
     * Result is returned asynchronously via userLoggedIn() or loginFailed() signals.
     *
     * @param userId User identifier
     * @param secretCode User's secret code/PIN
     *
     * @note This method returns immediately and is non-blocking
     */
    void login(const QString& userId, const QString& secretCode);

    /**
     * @brief Logout current user.
     *
     * Invalidates session on server and clears local session.
     */
    void logout();

    /**
     * @brief Check if user has specific permission.
     *
     * Checks if current user's role has the requested permission.
     * Uses PermissionRegistry for role-to-permission mapping.
     *
     * @param permission Permission to check
     * @return true if user has permission, false otherwise
     *
     * @note Returns false if no active session
     */
    bool hasPermission(Permission permission) const;

    /**
     * @brief Check if user has permission by string.
     *
     * Convenience method that converts permission string to enum.
     *
     * @param permissionStr Permission string (e.g., "VIEW_VITALS")
     * @return true if user has permission, false otherwise
     */
    bool hasPermission(const QString& permissionStr) const;

    /**
     * @brief Get current user ID.
     *
     * @return Current user ID, or empty string if no active session
     */
    QString getCurrentUserId() const;

    /**
     * @brief Get current user role.
     *
     * @return Current user role, or UserRole::Count if no active session
     */
    UserRole getCurrentRole() const;

    /**
     * @brief Get current user display name.
     *
     * @return Current user display name, or empty string if no active session
     */
    QString getCurrentUserDisplayName() const;

    /**
     * @brief Check if user is logged in.
     *
     * @return true if active session exists, false otherwise
     */
    bool isLoggedIn() const;

    /**
     * @brief Get session token.
     *
     * @return Session token, or empty string if no active session
     */
    QString getSessionToken() const;

    /**
     * @brief Refresh session activity timestamp.
     *
     * Updates last activity time to prevent session timeout.
     * Should be called on user interactions.
     */
    void refreshActivity();

    /**
     * @brief Check session validity.
     *
     * Validates session with server and handles expiration.
     * Called periodically by session monitoring timer.
     */
    void checkSessionValidity();

    /**
     * @brief Set session timeout.
     *
     * @param timeoutMinutes Session timeout in minutes (default: 60)
     */
    void setSessionTimeout(int timeoutMinutes);

    /**
     * @brief Get session timeout.
     *
     * @return Session timeout in minutes
     */
    int getSessionTimeout() const { return m_sessionTimeoutMinutes; }

public slots:
    /**
     * @brief Handle authentication completed signal from IUserManagementService.
     */
    void onAuthenticationCompleted(const QString& userId,
                                   const std::optional<UserProfile>& profile,
                                   const std::optional<AuthenticationError>& error);

    /**
     * @brief Handle session validation completed signal from IUserManagementService.
     */
    void onSessionValidationCompleted(const QString& sessionToken,
                                      bool isValid,
                                      const std::optional<ValidationError>& error);

    /**
     * @brief Handle logout completed signal from IUserManagementService.
     */
    void onLogoutCompleted(const QString& sessionToken, bool success);

signals:
    /**
     * @brief Emitted when user successfully logs in.
     *
     * @param userId User ID
     * @param role User role
     * @param displayName User display name
     */
    void userLoggedIn(const QString& userId, UserRole role, const QString& displayName);

    /**
     * @brief Emitted when login fails.
     *
     * @param userId User ID that failed to log in
     * @param errorMessage Error message
     * @param remainingAttempts Remaining login attempts before lockout
     */
    void loginFailed(const QString& userId, const QString& errorMessage, int remainingAttempts);

    /**
     * @brief Emitted when user logs out.
     */
    void userLoggedOut();

    /**
     * @brief Emitted when session expires.
     *
     * @param reason Reason for expiration (e.g., "timeout", "revoked")
     */
    void sessionExpired(const QString& reason);

    /**
     * @brief Emitted when permission check is needed (for async permission checks).
     *
     * @param permission Permission that was checked
     * @param granted true if permission granted, false otherwise
     */
    void permissionChecked(const QString& permission, bool granted);

private slots:
    /**
     * @brief Handle session monitoring timer timeout.
     */
    void onSessionMonitoringTimeout();

private:
    /**
     * @brief Initialize session monitoring.
     *
     * Sets up timer for periodic session validation.
     */
    void initializeSessionMonitoring();

    /**
     * @brief Handle session expiration.
     *
     * Clears session and emits appropriate signals.
     *
     * @param reason Reason for expiration
     */
    void handleSessionExpired(const QString& reason);

    /**
     * @brief Log authentication event to audit repository.
     *
     * @param eventType Event type (e.g., "LOGIN_SUCCESS", "LOGIN_FAILED")
     * @param userId User ID
     * @param details Additional details (JSON string)
     */
    void logAuditEvent(const QString& eventType,
                       const QString& userId,
                       const QString& details = QString());

    /**
     * @brief Convert UserRole to string for audit logging.
     *
     * @param role User role
     * @return String representation
     */
    QString roleToString(UserRole role) const;

    IUserManagementService* m_userMgmtService;        ///< User management service
    IAuditRepository* m_auditRepo;                    ///< Audit repository
    SettingsManager* m_settingsManager;                ///< Settings manager
    std::optional<UserSession> m_currentSession;       ///< Current active session
    QTimer* m_sessionMonitoringTimer;                  ///< Timer for session validation
    int m_sessionTimeoutMinutes = 60;                  ///< Session timeout in minutes
    int m_sessionValidationInterval = 300;            ///< Session validation interval (seconds)
};

} // namespace zmon

