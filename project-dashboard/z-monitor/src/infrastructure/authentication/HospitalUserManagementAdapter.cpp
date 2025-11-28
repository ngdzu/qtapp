/**
 * @file HospitalUserManagementAdapter.cpp
 * @brief Implementation of HospitalUserManagementAdapter.
 *
 * @author Z Monitor Team
 * @date 2025-01-15
 */

#include "HospitalUserManagementAdapter.h"
#include "domain/security/PermissionRegistry.h"
#include "domain/security/Permission.h"
#include "domain/security/UserRole.h"
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QUrl>
#include <QUrlQuery>
#include <QSslConfiguration>
#include <QDateTime>
#include <QUuid>

namespace zmon {

HospitalUserManagementAdapter::HospitalUserManagementAdapter(
    const QString& serverUrl,
    CertificateManager* certManager,
    QObject* parent)
    : IUserManagementService(parent)
    , m_serverUrl(serverUrl)
    , m_certManager(certManager)
    , m_networkManager(new QNetworkAccessManager(this))
    , m_healthCheckTimer(new QTimer(this))
{
    // Configure SSL/TLS if certificate manager provided
    if (m_certManager) {
        // TODO: Configure SSL with client certificates for mTLS
        // This would be done via QSslConfiguration
    }

    // Set up periodic health checks (every 5 minutes)
    m_healthCheckTimer->setInterval(300000); // 5 minutes
    connect(m_healthCheckTimer, &QTimer::timeout, this, &HospitalUserManagementAdapter::healthCheck);
    m_healthCheckTimer->start();
}

void HospitalUserManagementAdapter::authenticate(const QString& userId,
                                                 const QString& secretCode,
                                                 const QString& deviceId) {
    // Create request
    QNetworkRequest request = createRequest("/api/v1/auth/login");

    // Create JSON payload
    QJsonObject payload;
    payload["userId"] = userId;
    payload["secretCode"] = secretCode;
    payload["deviceId"] = deviceId;

    QJsonDocument doc(payload);
    QByteArray data = doc.toJson(QJsonDocument::Compact);

    // Send POST request
    QNetworkReply* reply = m_networkManager->post(request, data);
    m_pendingAuths[reply] = userId;

    // Connect signals
    connect(reply, &QNetworkReply::finished, this, &HospitalUserManagementAdapter::onAuthenticationResponse);
    connect(reply, QOverload<QNetworkReply::NetworkError>::of(&QNetworkReply::errorOccurred),
            this, [this, reply, userId](QNetworkReply::NetworkError error) {
        Q_UNUSED(error);
        // Error will be handled in onAuthenticationResponse
    });

    // Set timeout
    QTimer::singleShot(m_networkTimeout, this, [this, reply]() {
        if (m_pendingAuths.contains(reply)) {
            reply->abort();
            onNetworkTimeout();
        }
    });
}

void HospitalUserManagementAdapter::validateSession(const QString& sessionToken) {
    // Create request
    QNetworkRequest request = createRequest("/api/v1/auth/validate");
    request.setRawHeader("Authorization", ("Bearer " + sessionToken).toUtf8());

    // Send GET request
    QNetworkReply* reply = m_networkManager->get(request);
    m_pendingValidations[reply] = sessionToken;

    // Connect signals
    connect(reply, &QNetworkReply::finished, this, &HospitalUserManagementAdapter::onValidationResponse);

    // Set timeout
    QTimer::singleShot(m_networkTimeout, this, [this, reply]() {
        if (m_pendingValidations.contains(reply)) {
            reply->abort();
            onNetworkTimeout();
        }
    });
}

void HospitalUserManagementAdapter::logout(const QString& sessionToken, const QString& userId) {
    // Create request
    QNetworkRequest request = createRequest("/api/v1/auth/logout");
    request.setRawHeader("Authorization", ("Bearer " + sessionToken).toUtf8());

    // Create JSON payload
    QJsonObject payload;
    payload["userId"] = userId;

    QJsonDocument doc(payload);
    QByteArray data = doc.toJson(QJsonDocument::Compact);

    // Send POST request
    QNetworkReply* reply = m_networkManager->post(request, data);

    // Connect signals
    connect(reply, &QNetworkReply::finished, this, &HospitalUserManagementAdapter::onLogoutResponse);

    // Remove from cache
    m_sessionCache.remove(sessionToken);
    m_permissionCache.remove(sessionToken);
}

void HospitalUserManagementAdapter::checkPermission(const QString& sessionToken,
                                                     const QString& permission) {
    // Check cache first
    if (m_permissionCache.contains(sessionToken)) {
        QStringList permissions = m_permissionCache[sessionToken];
        bool granted = permissions.contains(permission);
        emit permissionCheckCompleted(permission, granted);
        return;
    }

    // Create request
    QNetworkRequest request = createRequest("/api/v1/auth/permission");
    request.setRawHeader("Authorization", ("Bearer " + sessionToken).toUtf8());

    QUrl url = request.url();
    QUrlQuery query;
    query.addQueryItem("permission", permission);
    url.setQuery(query);
    request.setUrl(url);

    // Send GET request
    QNetworkReply* reply = m_networkManager->get(request);
    m_pendingPermissionChecks[reply] = qMakePair(sessionToken, permission);

    // Connect signals
    connect(reply, &QNetworkReply::finished, this, &HospitalUserManagementAdapter::onPermissionCheckResponse);
}

void HospitalUserManagementAdapter::getPermissions(const QString& sessionToken) {
    // Check cache first
    if (m_permissionCache.contains(sessionToken)) {
        emit permissionsRetrieved(m_permissionCache[sessionToken]);
        return;
    }

    // Create request
    QNetworkRequest request = createRequest("/api/v1/auth/permissions");
    request.setRawHeader("Authorization", ("Bearer " + sessionToken).toUtf8());

    // Send GET request
    QNetworkReply* reply = m_networkManager->get(request);
    m_pendingPermissionsRetrievals[reply] = sessionToken;

    // Connect signals
    connect(reply, &QNetworkReply::finished, this, &HospitalUserManagementAdapter::onPermissionsRetrievedResponse);
}

void HospitalUserManagementAdapter::healthCheck() {
    // Create request
    QNetworkRequest request = createRequest("/api/v1/health");
    auto startTime = QDateTime::currentMSecsSinceEpoch();

    // Send GET request
    QNetworkReply* reply = m_networkManager->get(request);

    // Connect signals
    connect(reply, &QNetworkReply::finished, this, [this, startTime, reply]() {
        auto latency = QDateTime::currentMSecsSinceEpoch() - startTime;
        bool available = (reply->error() == QNetworkReply::NoError);
        emit healthCheckCompleted(available, static_cast<int>(latency));
        reply->deleteLater();
    });
}

void HospitalUserManagementAdapter::onAuthenticationResponse() {
    QNetworkReply* reply = qobject_cast<QNetworkReply*>(sender());
    if (!reply || !m_pendingAuths.contains(reply)) {
        return;
    }

    QString userId = m_pendingAuths.take(reply);
    QByteArray data = reply->readAll();

    if (reply->error() != QNetworkReply::NoError) {
        AuthenticationError error = handleNetworkError(reply, AuthenticationError::Reason::NETWORK_ERROR);
        emit authenticationCompleted(userId, std::nullopt, error);
        reply->deleteLater();
        return;
    }

    // Parse JSON response
    QJsonParseError parseError;
    QJsonDocument doc = QJsonDocument::fromJson(data, &parseError);
    if (parseError.error != QJsonParseError::NoError) {
        AuthenticationError error;
        error.reason = AuthenticationError::Reason::SERVER_ERROR;
        error.message = "Invalid JSON response from server";
        emit authenticationCompleted(userId, std::nullopt, error);
        reply->deleteLater();
        return;
    }

    QJsonObject json = doc.object();
    QString status = json["status"].toString();

    if (status == "SUCCESS") {
        QJsonObject userObj = json["user"].toObject();
        UserProfile profile = parseUserProfile(userObj);
        cacheUserSession(profile);
        emit authenticationCompleted(userId, profile, std::nullopt);
    } else {
        QJsonObject errorObj = json["error"].toObject();
        AuthenticationError error = parseAuthenticationError(errorObj);
        emit authenticationCompleted(userId, std::nullopt, error);
    }

    reply->deleteLater();
}

void HospitalUserManagementAdapter::onValidationResponse() {
    QNetworkReply* reply = qobject_cast<QNetworkReply*>(sender());
    if (!reply || !m_pendingValidations.contains(reply)) {
        return;
    }

    QString sessionToken = m_pendingValidations.take(reply);
    QByteArray data = reply->readAll();

    if (reply->error() != QNetworkReply::NoError) {
        ValidationError error;
        error.reason = ValidationError::Reason::NETWORK_ERROR;
        error.message = reply->errorString();
        emit sessionValidationCompleted(sessionToken, false, error);
        reply->deleteLater();
        return;
    }

    // Parse JSON response
    QJsonParseError parseError;
    QJsonDocument doc = QJsonDocument::fromJson(data, &parseError);
    if (parseError.error != QJsonParseError::NoError) {
        ValidationError error;
        error.reason = ValidationError::Reason::SERVER_ERROR;
        error.message = "Invalid JSON response";
        emit sessionValidationCompleted(sessionToken, false, error);
        reply->deleteLater();
        return;
    }

    QJsonObject json = doc.object();
    bool isValid = json["valid"].toBool();

    if (!isValid) {
        ValidationError error;
        QString reason = json["reason"].toString();
        if (reason == "EXPIRED") {
            error.reason = ValidationError::Reason::SESSION_EXPIRED;
        } else if (reason == "REVOKED") {
            error.reason = ValidationError::Reason::SESSION_REVOKED;
        } else {
            error.reason = ValidationError::Reason::SESSION_INVALID;
        }
        error.message = json["message"].toString();
        emit sessionValidationCompleted(sessionToken, false, error);
    } else {
        emit sessionValidationCompleted(sessionToken, true, std::nullopt);
    }

    reply->deleteLater();
}

void HospitalUserManagementAdapter::onLogoutResponse() {
    QNetworkReply* reply = qobject_cast<QNetworkReply*>(sender());
    if (!reply) {
        return;
    }

    bool success = (reply->error() == QNetworkReply::NoError);
    // Extract session token from request headers if needed
    QString sessionToken; // TODO: Extract from request
    emit logoutCompleted(sessionToken, success);
    reply->deleteLater();
}

void HospitalUserManagementAdapter::onPermissionCheckResponse() {
    QNetworkReply* reply = qobject_cast<QNetworkReply*>(sender());
    if (!reply || !m_pendingPermissionChecks.contains(reply)) {
        return;
    }

    auto pair = m_pendingPermissionChecks.take(reply);
    QString sessionToken = pair.first;
    QString permission = pair.second;
    QByteArray data = reply->readAll();

    if (reply->error() != QNetworkReply::NoError) {
        emit permissionCheckCompleted(permission, false);
        reply->deleteLater();
        return;
    }

    // Parse JSON response
    QJsonParseError parseError;
    QJsonDocument doc = QJsonDocument::fromJson(data, &parseError);
    if (parseError.error != QJsonParseError::NoError) {
        emit permissionCheckCompleted(permission, false);
        reply->deleteLater();
        return;
    }

    QJsonObject json = doc.object();
    bool granted = json["granted"].toBool();
    emit permissionCheckCompleted(permission, granted);
    reply->deleteLater();
}

void HospitalUserManagementAdapter::onPermissionsRetrievedResponse() {
    QNetworkReply* reply = qobject_cast<QNetworkReply*>(sender());
    if (!reply || !m_pendingPermissionsRetrievals.contains(reply)) {
        return;
    }

    QString sessionToken = m_pendingPermissionsRetrievals.take(reply);
    QByteArray data = reply->readAll();

    if (reply->error() != QNetworkReply::NoError) {
        emit permissionsRetrieved(QStringList());
        reply->deleteLater();
        return;
    }

    // Parse JSON response
    QJsonParseError parseError;
    QJsonDocument doc = QJsonDocument::fromJson(data, &parseError);
    if (parseError.error != QJsonParseError::NoError) {
        emit permissionsRetrieved(QStringList());
        reply->deleteLater();
        return;
    }

    QJsonObject json = doc.object();
    QJsonArray permsArray = json["permissions"].toArray();
    QStringList permissions;
    for (const QJsonValue& value : permsArray) {
        permissions.append(value.toString());
    }

    // Cache permissions
    m_permissionCache[sessionToken] = permissions;
    emit permissionsRetrieved(permissions);
    reply->deleteLater();
}

void HospitalUserManagementAdapter::onHealthCheckResponse() {
    // Handled inline in healthCheck()
}

void HospitalUserManagementAdapter::onNetworkTimeout() {
    // Handle timeout - this is a placeholder
    // Actual timeout handling is done per-request
}

UserProfile HospitalUserManagementAdapter::parseUserProfile(const QJsonObject& json) const {
    UserProfile profile;
    profile.userId = json["userId"].toString();
    profile.displayName = json["displayName"].toString();

    // Parse role
    QString roleStr = json["role"].toString();
    const auto& registry = PermissionRegistry::instance();
    // Convert role string to UserRole enum
    if (roleStr == "NURSE") {
        profile.role = UserRole::Nurse;
    } else if (roleStr == "PHYSICIAN") {
        profile.role = UserRole::Physician;
    } else if (roleStr == "TECHNICIAN" || roleStr == "TECH") {
        profile.role = UserRole::Technician;
    } else if (roleStr == "ADMINISTRATOR" || roleStr == "ADMIN") {
        profile.role = UserRole::Administrator;
    } else if (roleStr == "OBSERVER") {
        profile.role = UserRole::Observer;
    } else {
        profile.role = UserRole::Count; // Unknown
    }

    // Parse permissions
    QJsonArray permsArray = json["permissions"].toArray();
    for (const QJsonValue& value : permsArray) {
        profile.permissions.append(value.toString());
    }

    profile.sessionToken = json["sessionToken"].toString();
    QString expiryStr = json["sessionExpiry"].toString();
    profile.sessionExpiry = QDateTime::fromString(expiryStr, Qt::ISODate);
    profile.departmentId = json["departmentId"].toString();
    profile.badgeId = json["badgeId"].toString();

    return profile;
}

AuthenticationError HospitalUserManagementAdapter::parseAuthenticationError(const QJsonObject& json) const {
    AuthenticationError error;
    QString reasonStr = json["reason"].toString();

    if (reasonStr == "INVALID_CREDENTIALS") {
        error.reason = AuthenticationError::Reason::INVALID_CREDENTIALS;
    } else if (reasonStr == "ACCOUNT_LOCKED") {
        error.reason = AuthenticationError::Reason::ACCOUNT_LOCKED;
    } else if (reasonStr == "ACCOUNT_DISABLED") {
        error.reason = AuthenticationError::Reason::ACCOUNT_DISABLED;
    } else if (reasonStr == "NETWORK_ERROR") {
        error.reason = AuthenticationError::Reason::NETWORK_ERROR;
    } else if (reasonStr == "SERVER_ERROR") {
        error.reason = AuthenticationError::Reason::SERVER_ERROR;
    } else if (reasonStr == "TIMEOUT") {
        error.reason = AuthenticationError::Reason::TIMEOUT;
    } else if (reasonStr == "PERMISSION_DENIED") {
        error.reason = AuthenticationError::Reason::PERMISSION_DENIED;
    } else if (reasonStr == "LICENSE_EXPIRED") {
        error.reason = AuthenticationError::Reason::LICENSE_EXPIRED;
    } else if (reasonStr == "INVALID_DEPARTMENT") {
        error.reason = AuthenticationError::Reason::INVALID_DEPARTMENT;
    } else {
        error.reason = AuthenticationError::Reason::SERVER_ERROR;
    }

    error.message = json["message"].toString();
    error.remainingAttempts = json["remainingAttempts"].toInt();
    QString lockoutStr = json["lockoutExpiry"].toString();
    if (!lockoutStr.isEmpty()) {
        error.lockoutExpiry = QDateTime::fromString(lockoutStr, Qt::ISODate);
    }
    error.serverMessage = json["serverMessage"].toString();

    return error;
}

QNetworkRequest HospitalUserManagementAdapter::createRequest(const QString& endpoint) const {
    QUrl url(m_serverUrl + endpoint);
    QNetworkRequest request(url);

    // Set headers
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    request.setHeader(QNetworkRequest::UserAgentHeader, "Z-Monitor/1.0");

    // Configure SSL/TLS
    QSslConfiguration sslConfig = QSslConfiguration::defaultConfiguration();
    sslConfig.setProtocol(QSsl::TlsV1_2OrLater);
    // TODO: Add client certificate configuration if m_certManager is provided
    request.setSslConfiguration(sslConfig);

    return request;
}

AuthenticationError HospitalUserManagementAdapter::handleNetworkError(
    QNetworkReply* reply,
    AuthenticationError::Reason errorType) const {
    AuthenticationError error;
    error.reason = errorType;

    switch (reply->error()) {
        case QNetworkReply::TimeoutError:
            error.reason = AuthenticationError::Reason::TIMEOUT;
            error.message = "Request timed out";
            break;
        case QNetworkReply::ConnectionRefusedError:
        case QNetworkReply::HostNotFoundError:
        case QNetworkReply::NetworkAccessDeniedError:
            error.reason = AuthenticationError::Reason::NETWORK_ERROR;
            error.message = "Cannot connect to hospital server";
            break;
        default:
            error.message = reply->errorString();
            break;
    }

    return error;
}

void HospitalUserManagementAdapter::cacheUserSession(const UserProfile& profile) {
    m_sessionCache[profile.sessionToken] = profile;
    m_permissionCache[profile.sessionToken] = profile.permissions;
}

std::optional<UserProfile> HospitalUserManagementAdapter::getCachedSession(const QString& sessionToken) const {
    if (m_sessionCache.contains(sessionToken)) {
        UserProfile profile = m_sessionCache[sessionToken];
        if (profile.isSessionValid()) {
            return profile;
        }
    }
    return std::nullopt;
}

} // namespace zmon

