/**
 * @file HospitalUserManagementAdapter.h
 * @brief Production implementation of IUserManagementService connecting to hospital server.
 *
 * This class provides a production implementation that connects to a hospital
 * user management server via HTTPS (REST API) or LDAP/Active Directory to
 * authenticate healthcare workers and retrieve their roles/permissions.
 *
 * @author Z Monitor Team
 * @date 2025-01-15
 */

#pragma once

#include "domain/interfaces/IUserManagementService.h"
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QTimer>
#include <QMap>
#include <QString>

namespace zmon {

// Forward declaration (CertificateManager may not exist yet)
class CertificateManager;

/**
 * @class HospitalUserManagementAdapter
 * @brief Production implementation connecting to hospital user management server.
 *
 * Authenticates users against hospital Active Directory, LDAP, or REST API server
 * using industry-standard protocols. Handles network errors, timeouts, and retries.
 *
 * @note Runs on Network I/O Thread
 * @thread Network I/O Thread
 * @ingroup Infrastructure
 */
class HospitalUserManagementAdapter : public IUserManagementService {
    Q_OBJECT

public:
    /**
     * @brief Constructor.
     *
     * @param serverUrl Hospital server URL (e.g., "https://hospital.example.com/api/auth")
     * @param certManager Certificate manager for mTLS (optional, can be nullptr)
     * @param parent Parent QObject (for Qt parent-child ownership)
     */
    explicit HospitalUserManagementAdapter(
        const QString& serverUrl,
        CertificateManager* certManager = nullptr,
        QObject* parent = nullptr);

    /**
     * @brief Destructor.
     */
    ~HospitalUserManagementAdapter() override = default;

    /**
     * @brief Authenticate user with secret code/PIN.
     *
     * Sends POST request to hospital server to validate credentials.
     *
     * @param userId User identifier
     * @param secretCode User's secret code/PIN
     * @param deviceId Device identifier
     */
    void authenticate(const QString& userId,
                     const QString& secretCode,
                     const QString& deviceId) override;

    /**
     * @brief Validate active session token.
     *
     * Sends GET request to hospital server to validate session.
     *
     * @param sessionToken Session token to validate
     */
    void validateSession(const QString& sessionToken) override;

    /**
     * @brief Logout user (invalidate session on server).
     *
     * Sends POST request to hospital server to invalidate session.
     *
     * @param sessionToken Session token to invalidate
     * @param userId User ID (for audit logging)
     */
    void logout(const QString& sessionToken, const QString& userId) override;

    /**
     * @brief Get user permissions for specific action.
     *
     * Queries server for permission check (may use cached permissions).
     *
     * @param sessionToken Active session token
     * @param permission Permission to check
     */
    void checkPermission(const QString& sessionToken,
                         const QString& permission) override;

    /**
     * @brief Get list of available permissions for role.
     *
     * Retrieves all permissions for user's role (may use cached permissions).
     *
     * @param sessionToken Active session token
     */
    void getPermissions(const QString& sessionToken) override;

    /**
     * @brief Check if service is available (health check).
     *
     * Pings hospital server to verify connectivity.
     */
    void healthCheck() override;

    /**
     * @brief Set network timeout.
     *
     * @param timeoutMs Timeout in milliseconds (default: 10000ms)
     */
    void setNetworkTimeout(int timeoutMs) { m_networkTimeout = timeoutMs; }

    /**
     * @brief Set maximum retry attempts.
     *
     * @param maxRetries Maximum number of retry attempts (default: 3)
     */
    void setMaxRetries(int maxRetries) { m_maxRetries = maxRetries; }

private slots:
    /**
     * @brief Handle authentication response.
     */
    void onAuthenticationResponse();

    /**
     * @brief Handle session validation response.
     */
    void onValidationResponse();

    /**
     * @brief Handle logout response.
     */
    void onLogoutResponse();

    /**
     * @brief Handle permission check response.
     */
    void onPermissionCheckResponse();

    /**
     * @brief Handle permissions retrieval response.
     */
    void onPermissionsRetrievedResponse();

    /**
     * @brief Handle health check response.
     */
    void onHealthCheckResponse();

    /**
     * @brief Handle network timeout.
     */
    void onNetworkTimeout();

private:
    /**
     * @brief Parse user profile from JSON response.
     *
     * @param json JSON object containing user profile
     * @return User profile
     */
    UserProfile parseUserProfile(const QJsonObject& json) const;

    /**
     * @brief Parse authentication error from JSON response.
     *
     * @param json JSON object containing error details
     * @return Authentication error
     */
    AuthenticationError parseAuthenticationError(const QJsonObject& json) const;

    /**
     * @brief Create network request with proper headers and SSL configuration.
     *
     * @param endpoint API endpoint (relative to server URL)
     * @return Configured network request
     */
    QNetworkRequest createRequest(const QString& endpoint) const;

    /**
     * @brief Handle network error and convert to appropriate error type.
     *
     * @param reply Network reply that failed
     * @param errorType Type of error to create
     * @return Error object
     */
    AuthenticationError handleNetworkError(QNetworkReply* reply,
                                           AuthenticationError::Reason errorType) const;

    /**
     * @brief Cache user session locally.
     *
     * @param profile User profile to cache
     */
    void cacheUserSession(const UserProfile& profile);

    /**
     * @brief Get cached user session.
     *
     * @param sessionToken Session token
     * @return User profile if found, nullopt otherwise
     */
    std::optional<UserProfile> getCachedSession(const QString& sessionToken) const;

    QString m_serverUrl;                              ///< Hospital server URL
    CertificateManager* m_certManager;                ///< Certificate manager (for mTLS) - may be nullptr
    QNetworkAccessManager* m_networkManager;          ///< Network access manager
    QMap<QString, UserProfile> m_sessionCache;        ///< Cached user sessions
    QMap<QString, QStringList> m_permissionCache;     ///< Cached permissions (key: sessionToken)
    QMap<QNetworkReply*, QString> m_pendingAuths;    ///< Pending authentication requests
    QMap<QNetworkReply*, QString> m_pendingValidations; ///< Pending validation requests
    QMap<QNetworkReply*, QPair<QString, QString>> m_pendingPermissionChecks; ///< Pending permission checks
    QMap<QNetworkReply*, QString> m_pendingPermissionsRetrievals; ///< Pending permissions retrievals
    int m_networkTimeout = 10000;                     ///< Network timeout (ms)
    int m_maxRetries = 3;                             ///< Maximum retry attempts
    QTimer* m_healthCheckTimer;                       ///< Timer for periodic health checks
};

} // namespace zmon

