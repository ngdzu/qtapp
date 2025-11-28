/**
 * @file SecurityService.cpp
 * @brief Implementation of SecurityService.
 *
 * @author Z Monitor Team
 * @date 2025-01-15
 */

#include "SecurityService.h"
#include "domain/security/PermissionRegistry.h"
#include "domain/security/Permission.h"
#include "infrastructure/adapters/SettingsManager.h"
#include <QJsonObject>
#include <QJsonDocument>
#include <QDateTime>
#include <QTimer>
#include <QJsonObject>
#include <QJsonDocument>
#include <QDateTime>
#include <QTimer>

namespace zmon {

SecurityService::SecurityService(
    IUserManagementService* userMgmtService,
    IAuditRepository* auditRepo,
    SettingsManager* settingsManager,
    QObject* parent)
    : QObject(parent)
    , m_userMgmtService(userMgmtService)
    , m_auditRepo(auditRepo)
    , m_settingsManager(settingsManager)
    , m_sessionMonitoringTimer(new QTimer(this))
{
    // Connect to user management service signals
    connect(m_userMgmtService, &IUserManagementService::authenticationCompleted,
            this, &SecurityService::onAuthenticationCompleted);
    connect(m_userMgmtService, &IUserManagementService::sessionValidationCompleted,
            this, &SecurityService::onSessionValidationCompleted);
    connect(m_userMgmtService, &IUserManagementService::logoutCompleted,
            this, &SecurityService::onLogoutCompleted);

    // Load session timeout from settings
    if (m_settingsManager) {
        m_sessionTimeoutMinutes = m_settingsManager->getValue("session_timeout_minutes", 60).toInt();
    }

    // Initialize session monitoring
    initializeSessionMonitoring();
}

void SecurityService::login(const QString& userId, const QString& secretCode) {
    // Get device ID from settings
    QString deviceId;
    if (m_settingsManager) {
        deviceId = m_settingsManager->getValue("deviceId", "").toString();
    }

    // Log login attempt
    logAuditEvent("LOGIN_ATTEMPT", userId, QString("{\"deviceId\":\"%1\"}").arg(deviceId));

    // Initiate authentication
    m_userMgmtService->authenticate(userId, secretCode, deviceId);
}

void SecurityService::logout() {
    if (!m_currentSession.has_value()) {
        return; // No active session
    }

    QString userId = m_currentSession->userProfile.userId;
    QString sessionToken = m_currentSession->userProfile.sessionToken;

    // Log logout
    logAuditEvent("USER_LOGOUT", userId);

    // Invalidate session on server
    m_userMgmtService->logout(sessionToken, userId);

    // Clear local session immediately (don't wait for server response)
    m_currentSession = std::nullopt;

    // Stop session monitoring
    m_sessionMonitoringTimer->stop();

    // Emit signal
    emit userLoggedOut();
}

bool SecurityService::hasPermission(Permission permission) const {
    if (!m_currentSession.has_value() || !m_currentSession->isValid()) {
        return false; // No active session
    }

    // Get permissions for user's role from PermissionRegistry
    const auto& registry = PermissionRegistry::instance();
    PermissionSet rolePermissions = registry.permissionsForRole(m_currentSession->userProfile.role);

    // Check if permission is in role's permission set
    return hasPermission(rolePermissions, permission);
}

bool SecurityService::hasPermission(const QString& permissionStr) const {
    if (!m_currentSession.has_value() || !m_currentSession->isValid()) {
        return false; // No active session
    }

    // Convert permission string to enum
    const auto& registry = PermissionRegistry::instance();
    std::string permStr = permissionStr.toStdString();
    Permission permission = registry.fromString(permStr);

    if (permission == Permission::Count) {
        return false; // Invalid permission string
    }

    return hasPermission(permission);
}

QString SecurityService::getCurrentUserId() const {
    if (m_currentSession.has_value() && m_currentSession->isValid()) {
        return m_currentSession->userProfile.userId;
    }
    return QString();
}

UserRole SecurityService::getCurrentRole() const {
    if (m_currentSession.has_value() && m_currentSession->isValid()) {
        return m_currentSession->userProfile.role;
    }
    return UserRole::Count;
}

QString SecurityService::getCurrentUserDisplayName() const {
    if (m_currentSession.has_value() && m_currentSession->isValid()) {
        return m_currentSession->userProfile.displayName;
    }
    return QString();
}

bool SecurityService::isLoggedIn() const {
    return m_currentSession.has_value() && m_currentSession->isValid();
}

QString SecurityService::getSessionToken() const {
    if (m_currentSession.has_value() && m_currentSession->isValid()) {
        return m_currentSession->userProfile.sessionToken;
    }
    return QString();
}

void SecurityService::refreshActivity() {
    if (m_currentSession.has_value()) {
        m_currentSession->lastActivityTime = QDateTime::currentDateTimeUtc();
    }
}

void SecurityService::checkSessionValidity() {
    if (!m_currentSession.has_value()) {
        return; // No active session
    }

    // Check local expiry first (fast check)
    if (m_currentSession->isExpired()) {
        handleSessionExpired("local_timeout");
        return;
    }

    // Check inactivity timeout
    QDateTime now = QDateTime::currentDateTimeUtc();
    int inactivitySeconds = m_currentSession->lastActivityTime.secsTo(now);
    int inactivityTimeoutSeconds = m_sessionTimeoutMinutes * 60;

    if (inactivitySeconds > inactivityTimeoutSeconds) {
        handleSessionExpired("inactivity_timeout");
        return;
    }

    // Validate with server (authoritative)
    m_userMgmtService->validateSession(m_currentSession->userProfile.sessionToken);
}

void SecurityService::setSessionTimeout(int timeoutMinutes) {
    m_sessionTimeoutMinutes = timeoutMinutes;
    if (m_settingsManager) {
        m_settingsManager->setValue("session_timeout_minutes", timeoutMinutes);
    }
}

void SecurityService::onAuthenticationCompleted(
    const QString& userId,
    const std::optional<UserProfile>& profile,
    const std::optional<AuthenticationError>& error) {
    if (error.has_value()) {
        // Authentication failed
        AuthenticationError authError = error.value();
        logAuditEvent("LOGIN_FAILED", userId, QString("{\"reason\":\"%1\",\"message\":\"%2\"}")
                      .arg(QString::number(static_cast<int>(authError.reason)), authError.message));
        emit loginFailed(userId, authError.message, authError.remainingAttempts);
        return;
    }

    if (!profile.has_value()) {
        // No profile returned (should not happen)
        logAuditEvent("LOGIN_FAILED", userId, "{\"reason\":\"NO_PROFILE\"}");
        emit loginFailed(userId, "Authentication failed: No profile returned", 0);
        return;
    }

    // Authentication succeeded - create session
    UserProfile userProfile = profile.value();
    UserSession session;
    session.userProfile = userProfile;
    session.createdAt = QDateTime::currentDateTimeUtc();
    session.lastActivityTime = QDateTime::currentDateTimeUtc();
    session.expiresAt = userProfile.sessionExpiry;

    m_currentSession = session;

    // Start session monitoring
    m_sessionMonitoringTimer->start();

    // Log successful login
    logAuditEvent("LOGIN_SUCCESS", userId, QString("{\"role\":\"%1\",\"displayName\":\"%2\"}")
                  .arg(roleToString(userProfile.role), userProfile.displayName));

    // Emit success signal
    emit userLoggedIn(userId, userProfile.role, userProfile.displayName);
}

void SecurityService::onSessionValidationCompleted(
    const QString& sessionToken,
    bool isValid,
    const std::optional<ValidationError>& error) {
    if (!m_currentSession.has_value()) {
        return; // No active session
    }

    if (m_currentSession->userProfile.sessionToken != sessionToken) {
        return; // Different session token
    }

    if (!isValid) {
        // Session invalidated by server
        QString reason = "server_validation_failed";
        if (error.has_value()) {
            ValidationError valError = error.value();
            switch (valError.reason) {
                case ValidationError::Reason::SESSION_EXPIRED:
                    reason = "server_expired";
                    break;
                case ValidationError::Reason::SESSION_REVOKED:
                    reason = "server_revoked";
                    break;
                case ValidationError::Reason::SESSION_INVALID:
                    reason = "server_invalid";
                    break;
                default:
                    reason = "server_error";
                    break;
            }
        }
        handleSessionExpired(reason);
    }
    // If valid, session continues - no action needed
}

void SecurityService::onLogoutCompleted(const QString& sessionToken, bool success) {
    Q_UNUSED(sessionToken);
    Q_UNUSED(success);
    // Logout already handled in logout() method
    // This signal is just for confirmation
}

void SecurityService::onSessionMonitoringTimeout() {
    checkSessionValidity();
}

void SecurityService::initializeSessionMonitoring() {
    // Set up timer for periodic session validation
    m_sessionMonitoringTimer->setInterval(m_sessionValidationInterval * 1000); // Convert to milliseconds
    connect(m_sessionMonitoringTimer, &QTimer::timeout,
            this, &SecurityService::onSessionMonitoringTimeout);
    // Timer will be started when user logs in
}

void SecurityService::handleSessionExpired(const QString& reason) {
    if (!m_currentSession.has_value()) {
        return; // No active session
    }

    QString userId = m_currentSession->userProfile.userId;

    // Clear session
    m_currentSession = std::nullopt;

    // Stop session monitoring
    m_sessionMonitoringTimer->stop();

    // Log event
    logAuditEvent("SESSION_EXPIRED", userId, QString("{\"reason\":\"%1\"}").arg(reason));

    // Emit signal
    emit sessionExpired(reason);
}

void SecurityService::logAuditEvent(const QString& eventType,
                                     const QString& userId,
                                     const QString& details) {
    if (!m_auditRepo) {
        return; // No audit repository available
    }

    IAuditRepository::AuditEntry entry;
    entry.timestampMs = QDateTime::currentMSecsSinceEpoch();
    entry.userId = userId.toStdString();
    
    // Get user role if session exists
    if (m_currentSession.has_value()) {
        entry.userRole = roleToString(m_currentSession->userProfile.role).toStdString();
    } else {
        entry.userRole = "UNKNOWN";
    }

    entry.actionType = eventType.toStdString();
    entry.targetType = "AUTHENTICATION";
    entry.targetId = userId.toStdString();
    entry.details = details.toStdString();

    // Get previous hash for hash chain
    IAuditRepository::AuditEntry lastEntry = m_auditRepo->getLastEntry();
    entry.previousHash = lastEntry.entryHash;

    // Calculate entry hash (simplified - should use proper hash function)
    // TODO: Implement proper hash calculation
    entry.entryHash = "hash_" + std::to_string(entry.timestampMs);

    // Save to audit repository
    m_auditRepo->save(entry);
}

QString SecurityService::roleToString(UserRole role) const {
    switch (role) {
        case UserRole::Observer:
            return "OBSERVER";
        case UserRole::Technician:
            return "TECHNICIAN";
        case UserRole::Nurse:
            return "NURSE";
        case UserRole::Physician:
            return "PHYSICIAN";
        case UserRole::Administrator:
            return "ADMINISTRATOR";
        case UserRole::Count:
            return "UNKNOWN";
    }
    return "UNKNOWN";
}

} // namespace zmon

