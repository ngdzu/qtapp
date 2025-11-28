/**
 * @file AuthenticationController.cpp
 * @brief Implementation of AuthenticationController.
 *
 * @author Z Monitor Team
 * @date 2025-01-15
 */

#include "AuthenticationController.h"
#include "application/services/SecurityService.h"
#include "domain/security/UserRole.h"
#include <QString>

namespace zmon {

AuthenticationController::AuthenticationController(SecurityService* securityService, QObject* parent)
    : QObject(parent)
    , m_securityService(securityService)
{
    // Connect to SecurityService signals
    if (m_securityService) {
        connect(m_securityService, &SecurityService::userLoggedIn,
                this, &AuthenticationController::onUserLoggedIn);
        connect(m_securityService, &SecurityService::loginFailed,
                this, &AuthenticationController::onLoginFailed);
        connect(m_securityService, &SecurityService::userLoggedOut,
                this, &AuthenticationController::onUserLoggedOut);
        connect(m_securityService, &SecurityService::sessionExpired,
                this, &AuthenticationController::onSessionExpired);

        // Initialize state from SecurityService
        updateLoginState();
    }
}

void AuthenticationController::login(const QString& userId, const QString& secretCode) {
    if (!m_securityService) {
        m_loginError = "Security service not available";
        emit loginErrorChanged();
        emit loginFailed();
        return;
    }

    // Clear previous error
    m_loginError.clear();
    emit loginErrorChanged();

    // Set authenticating state
    m_isAuthenticating = true;
    emit isAuthenticatingChanged();

    // Initiate login
    m_securityService->login(userId, secretCode);
}

void AuthenticationController::logout() {
    if (!m_securityService) {
        return;
    }

    m_securityService->logout();
}

void AuthenticationController::clearError() {
    m_loginError.clear();
    emit loginErrorChanged();
}

bool AuthenticationController::hasPermission(const QString& permission) const {
    if (!m_securityService) {
        return false;
    }

    return m_securityService->hasPermission(permission);
}

void AuthenticationController::onUserLoggedIn(const QString& userId, UserRole role, const QString& displayName) {
    m_isAuthenticating = false;
    emit isAuthenticatingChanged();

    // Update state
    updateLoginState();

    // Emit success signal
    emit loginSucceeded();
}

void AuthenticationController::onLoginFailed(const QString& userId, const QString& errorMessage, int remainingAttempts) {
    Q_UNUSED(userId);

    m_isAuthenticating = false;
    emit isAuthenticatingChanged();

    // Update error message
    m_loginError = errorMessage;
    if (remainingAttempts > 0) {
        m_loginError += QString(" (%1 attempts remaining)").arg(remainingAttempts);
    } else {
        m_loginError += " (Account locked)";
        // TODO: Start lockout timer
    }

    emit loginErrorChanged();
    emit loginFailed();
}

void AuthenticationController::onUserLoggedOut() {
    // Clear state
    m_isLoggedIn = false;
    m_currentUser.clear();
    m_currentRole.clear();
    m_currentUserDisplayName.clear();
    m_loginError.clear();
    m_remainingLockoutTime = 0;

    emit isLoggedInChanged();
    emit currentUserChanged();
    emit currentRoleChanged();
    emit currentUserDisplayNameChanged();
    emit loginErrorChanged();
    emit remainingLockoutTimeChanged();
}

void AuthenticationController::onSessionExpired(const QString& reason) {
    Q_UNUSED(reason);

    // Clear state
    m_isLoggedIn = false;
    m_currentUser.clear();
    m_currentRole.clear();
    m_currentUserDisplayName.clear();
    m_loginError = "Your session has expired. Please log in again.";
    m_remainingLockoutTime = 0;

    emit isLoggedInChanged();
    emit currentUserChanged();
    emit currentRoleChanged();
    emit currentUserDisplayNameChanged();
    emit loginErrorChanged();
    emit remainingLockoutTimeChanged();
    emit sessionExpired();
}

void AuthenticationController::updateLoginState() {
    if (!m_securityService) {
        return;
    }

    bool wasLoggedIn = m_isLoggedIn;
    m_isLoggedIn = m_securityService->isLoggedIn();

    if (m_isLoggedIn) {
        m_currentUser = m_securityService->getCurrentUserId();
        UserRole role = m_securityService->getCurrentRole();
        
        // Convert role to string
        switch (role) {
            case UserRole::Observer:
                m_currentRole = "OBSERVER";
                break;
            case UserRole::Technician:
                m_currentRole = "TECHNICIAN";
                break;
            case UserRole::Nurse:
                m_currentRole = "NURSE";
                break;
            case UserRole::Physician:
                m_currentRole = "PHYSICIAN";
                break;
            case UserRole::Administrator:
                m_currentRole = "ADMINISTRATOR";
                break;
            default:
                m_currentRole = "UNKNOWN";
                break;
        }

        m_currentUserDisplayName = m_securityService->getCurrentUserDisplayName();
    } else {
        m_currentUser.clear();
        m_currentRole.clear();
        m_currentUserDisplayName.clear();
    }

    // Emit signals only if state changed
    if (wasLoggedIn != m_isLoggedIn) {
        emit isLoggedInChanged();
    }
    
    // Always emit user info changes when logged in state changes
    if (wasLoggedIn != m_isLoggedIn) {
        emit currentUserChanged();
        emit currentRoleChanged();
        emit currentUserDisplayNameChanged();
    }
}

} // namespace zmon

