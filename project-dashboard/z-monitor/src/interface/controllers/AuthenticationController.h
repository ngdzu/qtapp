/**
 * @file AuthenticationController.h
 * @brief QML controller for user authentication UI.
 *
 * This file contains the AuthenticationController class which provides QML bindings
 * for user authentication including login, logout, and session management.
 *
 * @author Z Monitor Team
 * @date 2025-01-15
 */

#pragma once

#include <QObject>
#include <QString>

namespace zmon {

// Forward declarations
class SecurityService;
class UserRole;

/**
 * @class AuthenticationController
 * @brief QML controller for user authentication UI.
 *
 * Provides QML bindings for user authentication. Exposes login state,
 * user information, and authentication methods for the Login View.
 *
 * @thread Main/UI Thread
 * @ingroup Interface
 */
class AuthenticationController : public QObject {
    Q_OBJECT

    Q_PROPERTY(bool isLoggedIn READ isLoggedIn NOTIFY isLoggedInChanged)
    Q_PROPERTY(QString currentUser READ currentUser NOTIFY currentUserChanged)
    Q_PROPERTY(QString currentRole READ currentRole NOTIFY currentRoleChanged)
    Q_PROPERTY(QString currentUserDisplayName READ currentUserDisplayName NOTIFY currentUserDisplayNameChanged)
    Q_PROPERTY(QString loginError READ loginError NOTIFY loginErrorChanged)
    Q_PROPERTY(bool isAuthenticating READ isAuthenticating NOTIFY isAuthenticatingChanged)
    Q_PROPERTY(int remainingLockoutTime READ remainingLockoutTime NOTIFY remainingLockoutTimeChanged)

public:
    /**
     * @brief Constructor.
     *
     * @param securityService Security service for authentication
     * @param parent Parent QObject
     */
    explicit AuthenticationController(SecurityService* securityService, QObject* parent = nullptr);

    /**
     * @brief Destructor.
     */
    ~AuthenticationController() override = default;

    /**
     * @brief Gets whether user is logged in.
     *
     * @return true if logged in, false otherwise
     */
    bool isLoggedIn() const { return m_isLoggedIn; }

    /**
     * @brief Gets current user ID.
     *
     * @return Current user ID, or empty string if not logged in
     */
    QString currentUser() const { return m_currentUser; }

    /**
     * @brief Gets current user role.
     *
     * @return Current user role as string, or empty string if not logged in
     */
    QString currentRole() const { return m_currentRole; }

    /**
     * @brief Gets current user display name.
     *
     * @return Current user display name, or empty string if not logged in
     */
    QString currentUserDisplayName() const { return m_currentUserDisplayName; }

    /**
     * @brief Gets last login error message.
     *
     * @return Error message, or empty string if no error
     */
    QString loginError() const { return m_loginError; }

    /**
     * @brief Gets whether authentication is in progress.
     *
     * @return true if authenticating, false otherwise
     */
    bool isAuthenticating() const { return m_isAuthenticating; }

    /**
     * @brief Gets remaining lockout time in seconds.
     *
     * @return Remaining lockout time, or 0 if not locked out
     */
    int remainingLockoutTime() const { return m_remainingLockoutTime; }

public slots:
    /**
     * @brief Handle user logged in signal from SecurityService.
     */
    void onUserLoggedIn(const QString& userId, UserRole role, const QString& displayName);

    /**
     * @brief Handle login failed signal from SecurityService.
     */
    void onLoginFailed(const QString& userId, const QString& errorMessage, int remainingAttempts);

    /**
     * @brief Handle user logged out signal from SecurityService.
     */
    void onUserLoggedOut();

    /**
     * @brief Handle session expired signal from SecurityService.
     */
    void onSessionExpired(const QString& reason);

public:
    /**
     * @brief Authenticate user with user ID and secret code.
     *
     * @param userId User identifier
     * @param secretCode User's secret code/PIN
     *
     * @note This method is callable from QML
     */
    Q_INVOKABLE void login(const QString& userId, const QString& secretCode);

    /**
     * @brief Logout current user.
     *
     * @note This method is callable from QML
     */
    Q_INVOKABLE void logout();

    /**
     * @brief Clear login error message.
     *
     * @note This method is callable from QML
     */
    Q_INVOKABLE void clearError();

    /**
     * @brief Check if current user has permission.
     *
     * @param permission Permission string (e.g., "VIEW_VITALS")
     * @return true if user has permission, false otherwise
     *
     * @note This method is callable from QML
     */
    Q_INVOKABLE bool hasPermission(const QString& permission) const;

signals:
    /**
     * @brief Emitted when login state changes.
     */
    void isLoggedInChanged();

    /**
     * @brief Emitted when current user changes.
     */
    void currentUserChanged();

    /**
     * @brief Emitted when current role changes.
     */
    void currentRoleChanged();

    /**
     * @brief Emitted when current user display name changes.
     */
    void currentUserDisplayNameChanged();

    /**
     * @brief Emitted when login error changes.
     */
    void loginErrorChanged();

    /**
     * @brief Emitted when authentication state changes.
     */
    void isAuthenticatingChanged();

    /**
     * @brief Emitted when remaining lockout time changes.
     */
    void remainingLockoutTimeChanged();

    /**
     * @brief Emitted when login succeeds.
     */
    void loginSucceeded();

    /**
     * @brief Emitted when login fails.
     */
    void loginFailed();

    /**
     * @brief Emitted when session expires.
     */
    void sessionExpired();

private:
    /**
     * @brief Update login state properties.
     */
    void updateLoginState();

    SecurityService* m_securityService;       ///< Security service
    bool m_isLoggedIn = false;                ///< Login state
    QString m_currentUser;                    ///< Current user ID
    QString m_currentRole;                    ///< Current user role (as string)
    QString m_currentUserDisplayName;         ///< Current user display name
    QString m_loginError;                     ///< Last login error
    bool m_isAuthenticating = false;          ///< Authentication in progress
    int m_remainingLockoutTime = 0;           ///< Remaining lockout time (seconds)
};

} // namespace zmon

