/**
 * @file MockUserManagementService.cpp
 * @brief Implementation of MockUserManagementService.
 *
 * @author Z Monitor Team
 * @date 2025-01-15
 */

#include "MockUserManagementService.h"
#include "domain/security/PermissionRegistry.h"
#include "domain/security/Permission.h"
#include "domain/security/UserRole.h"
#include <QRandomGenerator>
#include <QDateTime>
#include <QUuid>

namespace zmon {

MockUserManagementService::MockUserManagementService(QObject* parent)
    : IUserManagementService(parent)
{
    initializeTestUsers();
}

void MockUserManagementService::initializeTestUsers() {
    const auto& registry = PermissionRegistry::instance();

    // Test Nurse: NURSE001 / 1234
    {
        UserProfile nurse;
        nurse.userId = "NURSE001";
        nurse.displayName = "Sarah Johnson, RN";
        nurse.role = UserRole::Nurse;
        PermissionSet nursePerms = registry.permissionsForRole(UserRole::Nurse);
        nurse.permissions = permissionSetToStringList(nursePerms);
        nurse.departmentId = "ICU";
        nurse.badgeId = "12345";
        m_testUsers["NURSE001:1234"] = nurse;
    }

    // Test Physician: PHYSICIAN001 / 5678
    {
        UserProfile physician;
        physician.userId = "PHYSICIAN001";
        physician.displayName = "Dr. Michael Chen, MD";
        physician.role = UserRole::Physician;
        PermissionSet physicianPerms = registry.permissionsForRole(UserRole::Physician);
        physician.permissions = permissionSetToStringList(physicianPerms);
        physician.departmentId = "ICU";
        physician.badgeId = "67890";
        m_testUsers["PHYSICIAN001:5678"] = physician;
    }

    // Test Technician: TECH001 / 9999
    {
        UserProfile technician;
        technician.userId = "TECH001";
        technician.displayName = "James Smith, BMET";
        technician.role = UserRole::Technician;
        PermissionSet techPerms = registry.permissionsForRole(UserRole::Technician);
        technician.permissions = permissionSetToStringList(techPerms);
        technician.departmentId = "BIOMED";
        technician.badgeId = "99999";
        m_testUsers["TECH001:9999"] = technician;
    }

    // Test Administrator: ADMIN001 / 0000
    {
        UserProfile admin;
        admin.userId = "ADMIN001";
        admin.displayName = "System Administrator";
        admin.role = UserRole::Administrator;
        PermissionSet adminPerms = registry.permissionsForRole(UserRole::Administrator);
        admin.permissions = permissionSetToStringList(adminPerms);
        admin.badgeId = "00000";
        m_testUsers["ADMIN001:0000"] = admin;
    }
}

QStringList MockUserManagementService::permissionSetToStringList(PermissionSet permissions) const {
    QStringList result;
    const auto& registry = PermissionRegistry::instance();

    // Iterate through all permissions and check if they're set
    for (size_t i = 0; i < static_cast<size_t>(Permission::Count); ++i) {
        Permission perm = static_cast<Permission>(i);
        if (hasPermission(permissions, perm)) {
            std::string permStr = registry.toString(perm);
            result.append(QString::fromStdString(permStr));
        }
    }

    return result;
}

QString MockUserManagementService::roleToString(UserRole role) const {
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

void MockUserManagementService::authenticate(const QString& userId,
                                              const QString& secretCode,
                                              const QString& deviceId) {
    // Simulate random failures if enabled
    if (m_simulateFailures && QRandomGenerator::global()->bounded(100) < 20) {
        QTimer::singleShot(m_simulatedLatency, this, [this, userId]() {
            AuthenticationError error;
            error.reason = AuthenticationError::Reason::NETWORK_ERROR;
            error.message = "Simulated network error";
            emit authenticationCompleted(userId, std::nullopt, error);
        });
        return;
    }

    // Look up user
    QString key = userId + ":" + secretCode;
    if (!m_testUsers.contains(key)) {
        // Invalid credentials
        QTimer::singleShot(m_simulatedLatency, this, [this, userId]() {
            AuthenticationError error;
            error.reason = AuthenticationError::Reason::INVALID_CREDENTIALS;
            error.message = "Invalid user ID or secret code";
            error.remainingAttempts = 2;
            emit authenticationCompleted(userId, std::nullopt, error);
        });
        return;
    }

    // Valid credentials - create session
    UserProfile profile = m_testUsers[key];
    profile.sessionToken = QUuid::createUuid().toString(QUuid::WithoutBraces);
    profile.sessionExpiry = QDateTime::currentDateTimeUtc().addSecs(3600); // 1 hour

    // Store active session
    m_activeSessions[profile.sessionToken] = profile;

    // Emit success after simulated latency
    QTimer::singleShot(m_simulatedLatency, this, [this, userId, profile]() {
        emit authenticationCompleted(userId, profile, std::nullopt);
    });
}

void MockUserManagementService::validateSession(const QString& sessionToken) {
    // Simulate random failures if enabled
    if (m_simulateFailures && QRandomGenerator::global()->bounded(100) < 20) {
        QTimer::singleShot(m_simulatedLatency, this, [this, sessionToken]() {
            ValidationError error;
            error.reason = ValidationError::Reason::NETWORK_ERROR;
            error.message = "Simulated network error";
            emit sessionValidationCompleted(sessionToken, false, error);
        });
        return;
    }

    // Check if session exists and is valid
    bool isValid = false;
    if (m_activeSessions.contains(sessionToken)) {
        UserProfile profile = m_activeSessions[sessionToken];
        isValid = profile.isSessionValid();
        
        if (!isValid) {
            // Remove expired session
            m_activeSessions.remove(sessionToken);
        }
    }

    // Emit result after simulated latency
    QTimer::singleShot(m_simulatedLatency, this, [this, sessionToken, isValid]() {
        if (isValid) {
            emit sessionValidationCompleted(sessionToken, true, std::nullopt);
        } else {
            ValidationError error;
            error.reason = ValidationError::Reason::SESSION_EXPIRED;
            error.message = "Session expired or invalid";
            emit sessionValidationCompleted(sessionToken, false, error);
        }
    });
}

void MockUserManagementService::logout(const QString& sessionToken, const QString& userId) {
    // Remove session
    m_activeSessions.remove(sessionToken);

    // Emit success after simulated latency
    QTimer::singleShot(m_simulatedLatency, this, [this, sessionToken]() {
        emit logoutCompleted(sessionToken, true);
    });
}

void MockUserManagementService::checkPermission(const QString& sessionToken,
                                                 const QString& permission) {
    // Simulate random failures if enabled
    if (m_simulateFailures && QRandomGenerator::global()->bounded(100) < 20) {
        QTimer::singleShot(m_simulatedLatency, this, [this, permission]() {
            emit permissionCheckCompleted(permission, false);
        });
        return;
    }

    // Check if session exists
    bool granted = false;
    if (m_activeSessions.contains(sessionToken)) {
        UserProfile profile = m_activeSessions[sessionToken];
        if (profile.isSessionValid()) {
            granted = profile.hasPermission(permission);
        }
    }

    // Emit result after simulated latency
    QTimer::singleShot(m_simulatedLatency, this, [this, permission, granted]() {
        emit permissionCheckCompleted(permission, granted);
    });
}

void MockUserManagementService::getPermissions(const QString& sessionToken) {
    // Simulate random failures if enabled
    if (m_simulateFailures && QRandomGenerator::global()->bounded(100) < 20) {
        QTimer::singleShot(m_simulatedLatency, this, [this]() {
            emit permissionsRetrieved(QStringList());
        });
        return;
    }

    // Get permissions for session
    QStringList permissions;
    if (m_activeSessions.contains(sessionToken)) {
        UserProfile profile = m_activeSessions[sessionToken];
        if (profile.isSessionValid()) {
            permissions = profile.permissions;
        }
    }

    // Emit result after simulated latency
    QTimer::singleShot(m_simulatedLatency, this, [this, permissions]() {
        emit permissionsRetrieved(permissions);
    });
}

void MockUserManagementService::healthCheck() {
    // Always return available for mock service
    QTimer::singleShot(m_simulatedLatency, this, [this]() {
        emit healthCheckCompleted(true, m_simulatedLatency);
    });
}

} // namespace zmon

