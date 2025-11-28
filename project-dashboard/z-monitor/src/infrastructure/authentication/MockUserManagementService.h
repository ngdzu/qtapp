/**
 * @file MockUserManagementService.h
 * @brief Mock implementation of IUserManagementService for development and testing.
 *
 * This class provides a mock implementation of the user management service
 * that uses hardcoded test users. It does not require network connectivity
 * and is useful for development, testing, and demonstrations.
 *
 * @author Z Monitor Team
 * @date 2025-01-15
 */

#pragma once

#include "domain/interfaces/IUserManagementService.h"
#include <QTimer>
#include <QMap>
#include <QString>

namespace zmon {

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
 * @ingroup Infrastructure
 */
class MockUserManagementService : public IUserManagementService {
    Q_OBJECT

public:
    /**
     * @brief Constructor.
     *
     * Initializes hardcoded test users and sets up async response simulation.
     *
     * @param parent Parent QObject (for Qt parent-child ownership)
     */
    explicit MockUserManagementService(QObject* parent = nullptr);

    /**
     * @brief Destructor.
     */
    ~MockUserManagementService() override = default;

    /**
     * @brief Authenticate user with secret code/PIN.
     *
     * Simulates authentication against hardcoded test users.
     * Emits authenticationCompleted() signal after simulated latency.
     *
     * @param userId User identifier (e.g., "NURSE001")
     * @param secretCode User's secret code/PIN
     * @param deviceId Device identifier (ignored in mock)
     */
    void authenticate(const QString& userId,
                      const QString& secretCode,
                      const QString& deviceId) override;

    /**
     * @brief Validate active session token.
     *
     * Checks if session token exists in active sessions.
     *
     * @param sessionToken Session token to validate
     */
    void validateSession(const QString& sessionToken) override;

    /**
     * @brief Logout user (invalidate session on server).
     *
     * Removes session from active sessions.
     *
     * @param sessionToken Session token to invalidate
     * @param userId User ID (for audit logging)
     */
    void logout(const QString& sessionToken, const QString& userId) override;

    /**
     * @brief Get user permissions for specific action.
     *
     * Checks if user's role has the requested permission.
     *
     * @param sessionToken Active session token
     * @param permission Permission to check
     */
    void checkPermission(const QString& sessionToken,
                         const QString& permission) override;

    /**
     * @brief Get list of available permissions for role.
     *
     * Returns all permissions for the user's role.
     *
     * @param sessionToken Active session token
     */
    void getPermissions(const QString& sessionToken) override;

    /**
     * @brief Check if service is available (health check).
     *
     * Always returns true for mock service.
     */
    void healthCheck() override;

    /**
     * @brief Set simulated network latency (for testing).
     *
     * @param latencyMs Latency in milliseconds (default: 500ms)
     */
    void setSimulatedLatency(int latencyMs) { m_simulatedLatency = latencyMs; }

    /**
     * @brief Enable/disable simulated failures (for testing error handling).
     *
     * @param enabled If true, 20% of requests will fail randomly
     */
    void setSimulateFailures(bool enabled) { m_simulateFailures = enabled; }

private slots:
    /**
     * @brief Handle delayed authentication response.
     */
    void onAuthenticationDelayed();

    /**
     * @brief Handle delayed session validation response.
     */
    void onValidationDelayed();

    /**
     * @brief Handle delayed permission check response.
     */
    void onPermissionCheckDelayed();

    /**
     * @brief Handle delayed permissions retrieval response.
     */
    void onPermissionsRetrievedDelayed();

    /**
     * @brief Handle delayed health check response.
     */
    void onHealthCheckDelayed();

private:
    /**
     * @brief Initialize hardcoded test users.
     */
    void initializeTestUsers();

    /**
     * @brief Convert PermissionSet to QStringList of permission strings.
     *
     * @param permissions Permission set to convert
     * @return List of permission strings
     */
    QStringList permissionSetToStringList(PermissionSet permissions) const;

    /**
     * @brief Convert UserRole enum to string for session token generation.
     *
     * @param role User role
     * @return String representation
     */
    QString roleToString(UserRole role) const;

    QMap<QString, UserProfile> m_testUsers;          ///< Hardcoded test users (key: "userId:secretCode")
    QMap<QString, UserProfile> m_activeSessions;     ///< Active mock sessions (key: sessionToken)
    int m_simulatedLatency = 500;                     ///< Simulated network latency (ms)
    bool m_simulateFailures = false;                 ///< Simulate random failures

    // Pending request data (for delayed responses)
    struct PendingAuth {
        QString userId;
        QString deviceId;
    };
    QMap<QTimer*, PendingAuth> m_pendingAuths;

    struct PendingValidation {
        QString sessionToken;
    };
    QMap<QTimer*, PendingValidation> m_pendingValidations;

    struct PendingPermissionCheck {
        QString sessionToken;
        QString permission;
    };
    QMap<QTimer*, PendingPermissionCheck> m_pendingPermissionChecks;

    struct PendingPermissionsRetrieval {
        QString sessionToken;
    };
    QMap<QTimer*, PendingPermissionsRetrieval> m_pendingPermissionsRetrievals;
};

} // namespace zmon

