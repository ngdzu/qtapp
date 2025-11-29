/**
 * @file IUserManagementService.h
 * @brief Interface for hospital user authentication and authorization.
 *
 * This file defines the IUserManagementService interface which provides an
 * abstraction for authenticating healthcare workers (nurses, physicians,
 * technicians, administrators) against a hospital user management server.
 *
 * @note This interface uses Qt types (QObject, QString, etc.) for asynchronous
 * communication via signals/slots. This is an exception to the domain layer's
 * no-Qt rule, as external service interfaces require Qt for async operations.
 *
 * @author Z Monitor Team
 * @date 2025-01-15
 */

#pragma once

#include <QObject>
#include <QString>
#include <QStringList>
#include <QDateTime>
#include <QVariantMap>
#include <optional>
#include "../security/UserRole.h"

namespace zmon {

/**
 * @struct UserProfile
 * @brief User profile returned by successful authentication.
 */
struct UserProfile {
    QString userId;              ///< User identifier (e.g., "NURSE001")
    QString displayName;         ///< Full name for display (e.g., "Sarah Johnson, RN")
    UserRole role;               ///< Primary role (from domain/security/UserRole.h)
    QStringList permissions;     ///< List of granted permissions (as strings)
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

/**
 * @class IUserManagementService
 * @brief Interface for hospital user authentication and authorization.
 *
 * Provides abstraction for querying hospital user management server
 * to authenticate healthcare workers and retrieve their roles/permissions.
 *
 * @note All methods are asynchronous and return results via signals
 * @note Implementations must support network timeouts and retry logic
 * @note This interface uses Qt types for async communication (exception to domain layer no-Qt rule)
 *
 * @ingroup DomainInterfaces
 */
class IUserManagementService : public QObject {
    Q_OBJECT

public:
    /**
     * @brief Constructor.
     *
     * @param parent Parent QObject (for Qt object hierarchy)
     */
    explicit IUserManagementService(QObject* parent = nullptr) : QObject(parent) {}

    /**
     * @brief Virtual destructor.
     */
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
     *
     * @note Result is returned asynchronously via authenticationCompleted() signal
     * @note This method returns immediately and is non-blocking
     *
     * @performance Target latency: < 2 seconds (network-dependent)
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
     *
     * @performance Target latency: < 500ms
     */
    virtual void healthCheck() = 0;

signals:
    /**
     * @brief Emitted when authentication completes.
     *
     * @param userId User ID that was authenticated
     * @param profile User profile if successful, nullopt if failed
     * @param error Error details if authentication failed
     */
    void authenticationCompleted(const QString& userId,
                                 const std::optional<UserProfile>& profile,
                                 const std::optional<AuthenticationError>& error);

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
};

} // namespace zmon

