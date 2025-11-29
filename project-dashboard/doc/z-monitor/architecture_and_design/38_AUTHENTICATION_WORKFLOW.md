# Authentication and Authorization Workflow

**Document ID:** DESIGN-038  
**Version:** 1.0  
**Status:** Draft  
**Last Updated:** 2025-11-27

---

## 1. Overview

This document describes the authentication and authorization workflow for the Z Monitor, including:
- Healthcare worker authentication (nurses, physicians, technicians, administrators)
- Integration with hospital user management server (LDAP/Active Directory/REST API)
- Role-based access control (RBAC) and permission enforcement
- Mock server for development and testing
- Session management and timeout handling

**Key Requirements:**
- Nurses, physicians, technicians, and administrators enter secret codes to access device
- Device queries hospital server to validate credentials and retrieve roles/permissions
- Mock server implementation for development/testing without real hospital infrastructure
- Centralized user management (users managed on hospital server, not individual devices)
- Audit trail of all authentication events

---

## 2. Architecture Overview

### 2.1 Component Diagram

```
┌─────────────────────────────────────────────────────────────┐
│                        Z Monitor Device                      │
├─────────────────────────────────────────────────────────────┤
│                                                               │
│  ┌────────────┐   ┌──────────────┐   ┌──────────────────┐  │
│  │ LoginView  │   │ SecurityServ │   │ IUserManagement  │  │
│  │  (QML UI)  │-->│   ice        │-->│     Service      │  │
│  │            │   │ (Application)│   │   (Interface)    │  │
│  └────────────┘   └──────────────┘   └──────────────────┘  │
│                                                ↑              │
│                                                │              │
│                          ┌─────────────────────┴────────┐    │
│                          │                              │    │
│                ┌─────────▼──────────┐     ┌────────────▼────┐│
│                │ HospitalUserMgmt   │     │ MockUserMgmt    ││
│                │    Adapter         │     │   Service       ││
│                │ (Production)       │     │ (Dev/Test)      ││
│                └─────────┬──────────┘     └─────────────────┘│
│                          │                                    │
└──────────────────────────┼────────────────────────────────────┘
                           │ HTTPS/TLS
                           ▼
                  ┌────────────────────┐
                  │ 🏥 Hospital Server │
                  │  User Management   │
                  │  (LDAP/AD/REST)    │
                  └────────────────────┘
```

### 2.2 Sequence Diagram

[View Authentication Workflow (Mermaid)](./38_AUTHENTICATION_WORKFLOW.mmd)  
[View Authentication Workflow (SVG)](./38_AUTHENTICATION_WORKFLOW.svg)

---

## 3. User Roles and Permissions

### 3.1 Role Hierarchy

```
Administrator (Full Access)
    ↑
Physician (Clinical + Advanced Settings)
    ↑
Nurse (Basic Clinical Operations)
    ↑
Technician (Device Configuration)
    ↑
Observer (Read-Only)
```

### 3.2 Role-Permission Matrix

All permissions are defined in `Permission` enum (`Permission.h`) and serialized as strings when communicating with the hospital server. Controllers and services must always reference the enum values shown below; never gate logic on string literals.

| Permission (enum) | Nurse | Physician | Technician | Administrator |
|-------------------|:-----:|:---------:|:----------:|:-------------:|
| **Monitoring** |||||
| `Permission::ViewVitals` | ✅ | ✅ | ❌ | ✅ |
| `Permission::ViewWaveforms` | ✅ | ✅ | ❌ | ✅ |
| `Permission::ViewTrends` | ✅ | ✅ | ❌ | ✅ |
| **Alarms** |||||
| `Permission::ViewAlarms` | ✅ | ✅ | ❌ | ✅ |
| `Permission::AcknowledgeAlarm` | ✅ | ✅ | ❌ | ✅ |
| `Permission::SilenceAlarmShort` | ✅ | ✅ | ❌ | ✅ |
| `Permission::SilenceAlarmExtended` | ❌ | ✅ | ❌ | ✅ |
| `Permission::AdjustAlarmThresholds` | ❌ | ✅ | ❌ | ✅ |
| `Permission::OverrideAlarm` | ❌ | ✅ | ❌ | ✅ |
| **Patient Management** |||||
| `Permission::ViewPatientData` | ✅ | ✅ | ❌ | ✅ |
| `Permission::AdmitPatient` | ✅ | ✅ | ❌ | ✅ |
| `Permission::DischargePatient` | ✅ | ✅ | ❌ | ✅ |
| `Permission::TransferPatient` | ✅ | ✅ | ❌ | ✅ |
| **Data Export** |||||
| `Permission::ExportVitals` | ❌ | ✅ | ❌ | ✅ |
| `Permission::ExportTrends` | ❌ | ✅ | ❌ | ✅ |
| **Device Configuration** |||||
| `Permission::AccessSystemSettings` | ❌ | ❌ | ✅ | ✅ |
| `Permission::ConfigureDevice` | ❌ | ❌ | ✅ | ✅ |
| `Permission::EnterProvisioningMode` | ❌ | ❌ | ✅ | ✅ |
| `Permission::ViewDiagnostics` | ❌ | ❌ | ✅ | ✅ |
| `Permission::ViewLogs` | ❌ | ❌ | ✅ | ✅ |
| `Permission::ExportLogs` | ❌ | ❌ | ✅ | ✅ |
| `Permission::CalibrateDevice` | ❌ | ❌ | ✅ | ✅ |
| **Administration** |||||
| `Permission::ManageUsers` | ❌ | ❌ | ❌ | ✅ |
| `Permission::ViewAuditLogs` | ❌ | ❌ | ❌ | ✅ |
| `Permission::ManageSettings` | ❌ | ❌ | ❌ | ✅ |
| `Permission::ResetDevice` | ❌ | ❌ | ❌ | ✅ |
| `Permission::UpdateFirmware` | ❌ | ❌ | ❌ | ✅ |

### 3.3 Permission Enforcement

```cpp
enum class Permission {
    SilenceAlarmShort,
    SilenceAlarmExtended,
    AdjustAlarmThresholds,
    // ...
};

// Before performing any sensitive action
bool SecurityService::checkPermission(Permission permission) const {
    if (!hasActiveSession()) {
        return false;  // Not logged in
    }
    
    // Check if user's role has this permission
    return m_currentSession->userProfile.hasPermission(permission);
}

// Example usage in AlarmController
void AlarmController::silenceAlarm(const QString& alarmId, int durationSeconds) {
    // Different permissions based on silence duration
    Permission requiredPermission = (durationSeconds <= 60) 
        ? Permission::SilenceAlarmShort 
        : Permission::SilenceAlarmExtended;
    
    if (!m_securityService->checkPermission(requiredPermission)) {
        emit permissionDenied(tr("You do not have permission to silence alarms for %1 seconds.")
            .arg(durationSeconds));
        return;
    }
    
    // Perform action
    m_alarmService->silenceAlarm(alarmId, durationSeconds);
    
    // Log action
    m_securityService->logAuditEvent(
        "ALARM_SILENCED",
        QString("alarmId=%1, duration=%2s").arg(alarmId).arg(durationSeconds)
    );
}
```

### 3.4 Permission Registry (Default Role Mapping)

`SecurityService` should never hardcode string-based permission lists. Instead, we introduce a dedicated `PermissionRegistry` component that owns the canonical mapping between `UserRole` values and the set of `Permission` enums granted to that role. The registry is responsible for:

- Defining the compile-time mapping shown in the matrix above (stored as `std::array<std::bitset<P kPermissionCount>, kRoleCount>` or similar).
- Providing helper APIs to resolve role → permissions, permission → localized label, and optional string serialization for telemetry/audit messages.
- Serving as the single source of truth for both `SecurityService` and UI controllers when they need to display human-readable permission descriptions.

```cpp
// PermissionRegistry is implemented in domain/security/PermissionRegistry.h/cpp
// Note: Uses std::string (not QString) to keep domain layer Qt-free
class PermissionRegistry {
public:
    static const PermissionRegistry& instance();

    PermissionSet permissionsForRole(UserRole role) const;
    std::string toString(Permission perm) const;           // e.g. "VIEW_VITALS"
    std::string toDisplayName(Permission perm) const;      // e.g. "View Vitals"
    Permission fromString(const std::string& permissionStr) const;

private:
    PermissionRegistry();
    std::array<PermissionSet, static_cast<size_t>(UserRole::Count)> m_roleMatrix;
    std::array<std::string, static_cast<size_t>(Permission::Count)> m_permissionStrings;
    std::array<std::string, static_cast<size_t>(Permission::Count)> m_permissionDisplayNames;
};

// Usage example:
const auto& registry = PermissionRegistry::instance();
PermissionSet nursePerms = registry.permissionsForRole(UserRole::Nurse);
bool canViewVitals = hasPermission(nursePerms, Permission::ViewVitals);

// Application layer can convert std::string to QString when needed:
QString permStr = QString::fromStdString(registry.toString(Permission::ViewVitals));
```

`UserProfile::hasPermission()` simply checks membership within the `PermissionSet` returned by the registry. During authentication, once a role is resolved from the hospital server, `SecurityService` seeds the user profile with `PermissionRegistry::permissionsForRole(resolvedRole)` and optionally merges in any extra per-user overrides supplied by the server. This guarantees that every device start has the same default permission set defined by the design table above, without duplicating string literals throughout the codebase.

---

## 4. Hospital Server Integration

### 4.1 REST API Protocol (Recommended)

**Base URL:** `https://hospital.example.com/api/v1/auth`

#### 4.1.1 Login Endpoint

**Request:**
```http
POST /api/v1/auth/login
Content-Type: application/json

{
  "userId": "NURSE001",
  "secretCode": "1234",
  "deviceId": "ZM-ICU-04"
}
```

**Response (Success):**
```http
HTTP/1.1 200 OK
Content-Type: application/json

{
  "status": "SUCCESS",
  "user": {
    "userId": "NURSE001",
    "displayName": "Sarah Johnson, RN",
    "role": "NURSE",
    "permissions": [
      "VIEW_VITALS",
      "VIEW_WAVEFORMS",
      "VIEW_ALARMS",
      "ACKNOWLEDGE_ALARM",
      "SILENCE_ALARM",
      "ADMIT_PATIENT",
      "DISCHARGE_PATIENT",
      "TRANSFER_PATIENT",
      "VIEW_TRENDS",
      "VIEW_PATIENT_DATA"
    ],
    "sessionToken": "eyJhbGciOiJIUzI1NiIsInR5cCI6IkpXVCJ9.eyJzdWIiOiJOVVJTRTAwMSIsInJvbGUiOiJOVVJTRSIsImlhdCI6MTYzMjUwMDAwMCwiZXhwIjoxNjMyNTAzNjAwfQ.abc123",
    "sessionExpiry": "2025-11-27T20:00:00Z",
    "departmentId": "ICU",
    "badgeId": "12345"
  }
}
```

**Response (Failure - Invalid Credentials):**
```http
HTTP/1.1 401 Unauthorized
Content-Type: application/json

{
  "status": "ERROR",
  "error": {
    "reason": "INVALID_CREDENTIALS",
    "message": "Invalid user ID or secret code",
    "remainingAttempts": 2
  }
}
```

**Response (Failure - Account Locked):**
```http
HTTP/1.1 403 Forbidden
Content-Type: application/json

{
  "status": "ERROR",
  "error": {
    "reason": "ACCOUNT_LOCKED",
    "message": "Account locked due to too many failed login attempts",
    "lockoutExpiry": "2025-11-27T14:30:00Z",
    "remainingAttempts": 0
  }
}
```

#### 4.1.2 Session Validation Endpoint

**Request:**
```http
POST /api/v1/auth/validate
Content-Type: application/json
Authorization: Bearer eyJhbGciOiJIUzI1NiIsInR5cCI6IkpXVCJ9...

{
  "sessionToken": "eyJhbGciOiJIUzI1NiIsInR5cCI6IkpXVCJ9..."
}
```

**Response:**
```http
HTTP/1.1 200 OK
Content-Type: application/json

{
  "status": "VALID",
  "expiresIn": 3600
}
```

#### 4.1.3 Logout Endpoint

**Request:**
```http
POST /api/v1/auth/logout
Content-Type: application/json
Authorization: Bearer eyJhbGciOiJIUzI1NiIsInR5cCI6IkpXVCJ9...

{
  "sessionToken": "eyJhbGciOiJIUzI1NiIsInR5cCI6IkpXVCJ9...",
  "userId": "NURSE001"
}
```

**Response:**
```http
HTTP/1.1 200 OK
Content-Type: application/json

{
  "status": "SUCCESS",
  "message": "Session invalidated successfully"
}
```

### 4.2 LDAP/Active Directory Protocol (Alternative)

**Connection:**
```
Server: ldap://hospital.example.com:389 (or ldaps://hospital.example.com:636)
Base DN: dc=hospital,dc=example,dc=com
```

**Authentication (Bind):**
```
Bind DN: cn=NURSE001,ou=Nurses,ou=Users,dc=hospital,dc=example,dc=com
Password: <secret code>
```

**User Lookup:**
```
Search Base: ou=Users,dc=hospital,dc=example,dc=com
Filter: (uid=NURSE001)
Attributes: cn, mail, departmentNumber, employeeType, memberOf
```

**Role Mapping:**
```
memberOf: cn=Nurses,ou=Groups,dc=hospital,dc=example,dc=com → Role: NURSE
memberOf: cn=Physicians,ou=Groups,dc=hospital,dc=example,dc=com → Role: PHYSICIAN
memberOf: cn=Technicians,ou=Groups,dc=hospital,dc=example,dc=com → Role: TECHNICIAN
```

---

## 5. Mock Server for Development/Testing

### 5.1 Mock Service Implementation

The `MockUserManagementService` provides hardcoded test users without requiring a real hospital server.

**Test Users:**

| User ID | Secret Code | Role | Display Name | Permissions |
|---------|-------------|------|--------------|-------------|
| `NURSE001` | `1234` | Nurse | Sarah Johnson, RN | Basic clinical ops |
| `PHYSICIAN001` | `5678` | Physician | Dr. Michael Chen, MD | All clinical + advanced |
| `TECH001` | `9999` | Technician | James Smith, BMET | Device configuration |
| `ADMIN001` | `0000` | Administrator | System Administrator | Full access |
| `OBSERVER001` | `1111` | Observer | Medical Student | Read-only |

**Features:**
- ✅ No network connection required
- ✅ Simulated network latency (configurable, default 500ms)
- ✅ Simulate random failures (for error handling testing)
- ✅ Works offline (air-gapped development)
- ✅ Instant session creation (no server round-trip)

**Configuration:**

```cpp
// In main.cpp
void Application::initializeUserManagementService() {
    bool useMock = SettingsManager::instance()->getValue("user_mgmt_use_mock", true).toBool();
    
    if (useMock) {
        m_logService->info("Using MOCK user management service (development mode)", {});
        auto mockService = new MockUserManagementService(this);
        
        // Configure mock behavior
        mockService->setSimulatedLatency(500);  // 500ms simulated latency
        mockService->setSimulateFailures(false);  // Disable random failures
        
        m_userMgmtService = mockService;
    } else {
        QString serverUrl = SettingsManager::instance()->getValue("user_mgmt_server_url").toString();
        m_logService->info("Using PRODUCTION user management service", {
            {"serverUrl", serverUrl}
        });
        m_userMgmtService = new HospitalUserManagementAdapter(serverUrl, m_certManager, this);
    }
}
```

**Switching Between Mock and Production:**

```bash
# Development mode (mock server)
./z-monitor --user-mgmt-mock

# Production mode (real hospital server)
./z-monitor --user-mgmt-server https://hospital.example.com/api/auth
```

### 5.2 Mock HTTP Server (Optional)

For testing with real HTTP calls (e.g., integration tests):

```python
# project-dashboard/mock-servers/user_management_mock_server.py
from flask import Flask, request, jsonify
import time

app = Flask(__name__)

# Hardcoded test users
TEST_USERS = {
    "NURSE001:1234": {
        "userId": "NURSE001",
        "displayName": "Sarah Johnson, RN",
        "role": "NURSE",
        "permissions": ["VIEW_VITALS", "ACKNOWLEDGE_ALARM", "ADMIT_PATIENT"],
        "departmentId": "ICU",
        "badgeId": "12345"
    },
    "PHYSICIAN001:5678": {
        "userId": "PHYSICIAN001",
        "displayName": "Dr. Michael Chen, MD",
        "role": "PHYSICIAN",
        "permissions": ["VIEW_VITALS", "ACKNOWLEDGE_ALARM", "ADJUST_ALARM_THRESHOLDS", "EXPORT_DATA"],
        "departmentId": "ICU",
        "badgeId": "67890"
    }
}

@app.route('/api/v1/auth/login', methods=['POST'])
def login():
    data = request.json
    user_key = f"{data['userId']}:{data['secretCode']}"
    
    # Simulate network latency
    time.sleep(0.5)
    
    if user_key in TEST_USERS:
        user = TEST_USERS[user_key]
        return jsonify({
            "status": "SUCCESS",
            "user": {
                **user,
                "sessionToken": f"mock-token-{data['userId']}",
                "sessionExpiry": "2025-11-27T20:00:00Z"
            }
        }), 200
    else:
        return jsonify({
            "status": "ERROR",
            "error": {
                "reason": "INVALID_CREDENTIALS",
                "message": "Invalid user ID or secret code",
                "remainingAttempts": 2
            }
        }), 401

if __name__ == '__main__':
    app.run(host='0.0.0.0', port=8080)
```

**Run mock server:**
```bash
cd project-dashboard/mock-servers
pip3 install flask
python3 user_management_mock_server.py

# Test with curl
curl -X POST http://localhost:8080/api/v1/auth/login \
  -H "Content-Type: application/json" \
  -d '{"userId":"NURSE001","secretCode":"1234","deviceId":"ZM-TEST-01"}'
```

---

## 6. Session Management

### 6.1 Session Lifecycle

```
┌─────────┐
│ No      │
│ Session │
└────┬────┘
     │ login()
     ▼
┌─────────┐
│ Active  │───────► checkPermission() [Allowed]
│ Session │
└────┬────┘
     │
     │ Timeout (1-2 hours)
     │ OR logout()
     │ OR session revoked
     ▼
┌─────────┐
│ Expired │
│ Session │
└─────────┘
     │
     │ Return to login
     ▼
┌─────────┐
│ No      │
│ Session │
└─────────┘
```

### 6.2 Session Validation

**Periodic Validation (Every 5 minutes):**

```cpp
void SecurityService::startSessionMonitoring() {
    // Check session every 5 minutes
    m_sessionTimer = new QTimer(this);
    connect(m_sessionTimer, &QTimer::timeout, 
            this, &SecurityService::checkSessionValidity);
    m_sessionTimer->start(300000);  // 5 minutes
}

void SecurityService::checkSessionValidity() {
    if (!m_currentSession.has_value()) {
        return;  // No active session
    }
    
    // Check local expiry first (fast check)
    if (!m_currentSession->userProfile.isSessionValid()) {
        handleSessionExpired("local_timeout");
        return;
    }
    
    // Validate with server (authoritative)
    m_userMgmtService->validateSession(m_currentSession->userProfile.sessionToken);
}
```

### 6.3 Session Timeout Handling

```cpp
void SecurityService::handleSessionExpired(const QString& reason) {
    QString userId = m_currentSession->userProfile.userId;
    
    // Clear session
    m_currentSession = std::nullopt;
    
    // Log event
    logAuditEvent("SESSION_EXPIRED", QString("reason=%1, userId=%2").arg(reason, userId));
    
    // Emit signals
    emit sessionExpired();
    emit requestLoginScreen();
    
    // Show notification to user
    NotificationController::instance()->showWarning(
        "Session Expired",
        "Your session has expired. Please log in again."
    );
}
```

---

## 7. Security Considerations

### 7.1 Secret Code Protection

**Requirements:**
- ✅ Secret codes transmitted over HTTPS (TLS 1.3)
- ✅ Secret codes never logged in plaintext
- ✅ Secret codes cleared from memory after use
- ✅ Certificate pinning to prevent MITM attacks

**Implementation:**

```cpp
void SecurityService::login(const QString& userId, const QString& secretCode) {
    // Log login attempt (WITHOUT secret code)
    logAuditEvent("LOGIN_ATTEMPT", QString("userId=%1, deviceId=%2")
        .arg(userId, SettingsManager::instance()->getDeviceLabel()));
    
    // Send to hospital server (over HTTPS)
    m_userMgmtService->authenticate(userId, secretCode, getDeviceLabel());
    
    // Clear secret code from memory immediately
    // (QSslSocket will clear it after transmission)
}
```

### 7.2 Brute Force Protection

**Server-Side Protection (Hospital Server):**
- Lock account after 3 failed attempts
- Lockout duration: 15 minutes (configurable)
- Exponential backoff for repeated lockouts

**Client-Side Protection (Z Monitor):**
- Disable login button for 5 seconds after failed attempt
- Show countdown timer
- Log all failed attempts to `security_audit_log`

### 7.3 Session Token Security

**Protection:**
- ✅ Short expiry (1-2 hours)
- ✅ Stored in memory only (not persisted to disk)
- ✅ Invalidated on logout
- ✅ Server-side revocation supported
- ✅ Device-specific (token tied to deviceId)

---

## 8. Audit Logging

### 8.1 Required Events

All authentication events must be logged to `security_audit_log`:

| Event | Details Logged |
|-------|----------------|
| `LOGIN_ATTEMPT` | userId, deviceId, timestamp, clientIP (if available) |
| `LOGIN_SUCCESS` | userId, role, sessionToken (hashed), deviceId, timestamp |
| `LOGIN_FAILED` | userId, failureReason, remainingAttempts, deviceId, timestamp |
| `LOGIN_BLOCKED` | userId, reason (account locked, disabled), deviceId, timestamp |
| `LOGIN_NETWORK_ERROR` | userId, error details, timestamp |
| `SESSION_EXPIRED` | userId, sessionToken (hashed), reason, timestamp |
| `SESSION_REVOKED` | userId, sessionToken (hashed), reason (admin revoked, logout), timestamp |
| `USER_LOGOUT` | userId, sessionToken (hashed), timestamp |
| `PERMISSION_DENIED` | userId, permission, resource, timestamp |
| `PERMISSION_GRANTED` | userId, permission, resource, timestamp |

### 8.2 Audit Log Format

```sql
INSERT INTO security_audit_log (
    event_type,
    event_category,
    user_id,
    user_role,
    device_id,
    resource,
    action,
    result,
    severity,
    details,
    client_ip,
    session_token_hash,
    timestamp_ms,
    previous_hash
) VALUES (
    'LOGIN_SUCCESS',
    'AUTHENTICATION',
    'NURSE001',
    'NURSE',
    'ZM-ICU-04',
    NULL,
    'LOGIN',
    'SUCCESS',
    'INFO',
    '{"departmentId":"ICU","badgeId":"12345"}',
    '192.168.1.100',
    'a1b2c3d4e5f6...',  -- SHA-256 hash of session token
    1632500000123,
    'previous-entry-hash'
);
```

---

## 9. Error Handling

### 9.1 Network Errors

**Scenario:** Cannot reach hospital server

**Handling:**
```cpp
void SecurityService::onConnectionError(const NetworkError& error) {
    logAuditEvent("LOGIN_NETWORK_ERROR", QString("error=%1").arg(error.message));
    
    emit loginFailed(m_pendingUserId, "Cannot connect to hospital server. Please check network connection or contact IT support.", 0);
    
    // Optionally: Show "Switch to Offline Mode" button (if supported)
    if (m_offlineModeSupported) {
        emit offlineModeAvailable();
    }
}
```

### 9.2 Server Errors

**Scenario:** Hospital server returns 500 Internal Server Error

**Handling:**
```cpp
void HospitalUserManagementAdapter::handleAuthenticationResponse(QNetworkReply* reply, const QString& userId) {
    if (reply->error() == QNetworkReply::InternalServerError) {
        AuthenticationError error;
        error.reason = AuthenticationError::Reason::SERVER_ERROR;
        error.message = "Hospital server is temporarily unavailable. Please try again later.";
        error.serverMessage = reply->readAll();
        
        emit authenticationCompleted(userId, Result<UserProfile, AuthenticationError>::failure(error));
        
        // Log for IT troubleshooting
        m_logService->critical("Hospital server error", {
            {"error", error.serverMessage},
            {"errorCode", QString::number(error.code)}
        });
    }
}
```

---

## 10. UI/UX Design

### 10.1 Login Screen

**Layout:**
```
┌─────────────────────────────────────────┐
│          Z Monitor                       │
│          Patient Monitoring System       │
│                                           │
│  ┌────────────────────────────────────┐ │
│  │ User ID:                            │ │
│  │ [NURSE001_________________]         │ │
│  │                                     │ │
│  │ Secret Code:                        │ │
│  │ [••••______________________]        │ │
│  │                                     │ │
│  │           [  Login  ]               │ │
│  │                                     │ │
│  │ Device: ZM-ICU-04 | Status: ● Ready│ │
│  └────────────────────────────────────┘ │
│                                           │
│  🧪 TEST MODE (Mock Server)              │
│                                           │
│  v1.0.0 | ICU Ward 4                     │
└─────────────────────────────────────────┘
```

**States:**
1. **Ready:** Waiting for input
2. **Authenticating:** Spinner shown, button disabled
3. **Success:** Brief "Welcome!" message, then navigate to Dashboard
4. **Failure:** Error message shown, fields cleared (or userId retained), focus on secret code

**Error Messages:**
- "Invalid user ID or secret code. 2 attempts remaining."
- "Account locked until 2:30 PM. Contact administrator."
- "Cannot connect to hospital server. Contact IT support."
- "Session expired. Please log in again."

### 10.2 Session Indicator

**Header Bar (After Login):**
```
┌─────────────────────────────────────────────────────────────┐
│ 👩‍⚕️ Sarah Johnson, RN | ZM-ICU-04 | 🔋 95% | [Logout] │
└─────────────────────────────────────────────────────────────┘
```

**Session Timeout Warning:**
```
┌─────────────────────────────────────────┐
│ ⚠️  Session Timeout Warning             │
│                                          │
│ Your session will expire in 5 minutes.  │
│                                          │
│ [  Extend Session  ] [ Logout ]         │
└─────────────────────────────────────────┘
```

---

## 11. Configuration

### 11.1 Settings

**Location:** `settings` table or configuration file

| Setting | Type | Default | Description |
|---------|------|---------|-------------|
| `user_mgmt_use_mock` | Boolean | `true` | Use mock service (development mode) |
| `user_mgmt_server_url` | String | `""` | Hospital user management server URL |
| `user_mgmt_protocol` | Enum | `REST` | Protocol (`REST`, `LDAP`, `AD`) |
| `session_timeout_minutes` | Integer | `60` | Session timeout (minutes) |
| `session_validation_interval` | Integer | `300` | Session validation interval (seconds) |
| `max_failed_attempts` | Integer | `3` | Max failed login attempts (client-side) |
| `login_retry_delay_seconds` | Integer | `5` | Delay after failed login (seconds) |
| `network_timeout_seconds` | Integer | `10` | Network request timeout |
| `certificate_pinning_enabled` | Boolean | `true` | Enable certificate pinning |
| `offline_mode_supported` | Boolean | `false` | Allow offline mode fallback |

### 11.2 Command-Line Arguments

```bash
# Development mode (mock server)
./z-monitor --user-mgmt-mock

# Production mode (REST API)
./z-monitor --user-mgmt-server https://hospital.example.com/api/auth

# Production mode (LDAP)
./z-monitor --user-mgmt-server ldaps://hospital.example.com:636 --user-mgmt-protocol LDAP
```

---

## 12. Implementation Checklist

- [ ] Create `IUserManagementService` interface
- [ ] Implement `MockUserManagementService` with hardcoded test users
- [ ] Implement `HospitalUserManagementAdapter` (REST API version)
- [ ] Integrate with `SecurityService`
- [ ] Update `LoginView` QML
- [ ] Update `AuthenticationController`
- [ ] Implement session management (creation, validation, expiry)
- [ ] Implement permission checking
- [ ] Add audit logging for all authentication events
- [ ] Create unit tests for `MockUserManagementService`
- [ ] Create integration tests for authentication workflow
- [ ] Create mock HTTP server (Python Flask) for testing
- [ ] Update `settings` table with new configuration options
- [ ] Update UI to show current user and role
- [ ] Implement session timeout warning dialog
- [ ] Implement logout functionality
- [ ] Add error handling for network failures
- [ ] Add error handling for server failures
- [ ] Update documentation (this file)
- [ ] Update `ZTODO.md` with implementation tasks

---

## 13. Testing Strategy

### 13.1 Unit Tests

```cpp
TEST_F(MockUserManagementServiceTest, Authenticate_ValidNurse_ReturnsNurseProfile) {
    MockUserManagementService service;
    QSignalSpy spy(&service, &IUserManagementService::authenticationCompleted);
    
    service.authenticate("NURSE001", "1234", "ZM-TEST-01");
    
    ASSERT_EQ(spy.count(), 1);
    auto result = spy.at(0).at(1).value<Result<UserProfile, AuthenticationError>>();
    ASSERT_TRUE(result.isSuccess());
    EXPECT_EQ(result.value().userId, "NURSE001");
    EXPECT_EQ(result.value().role, UserRole::NURSE);
    EXPECT_TRUE(result.value().hasPermission("VIEW_VITALS"));
    EXPECT_FALSE(result.value().hasPermission("ADJUST_ALARM_THRESHOLDS"));
}

TEST_F(MockUserManagementServiceTest, Authenticate_InvalidCredentials_ReturnsError) {
    MockUserManagementService service;
    QSignalSpy spy(&service, &IUserManagementService::authenticationCompleted);
    
    service.authenticate("NURSE001", "wrong", "ZM-TEST-01");
    
    ASSERT_EQ(spy.count(), 1);
    auto result = spy.at(0).at(1).value<Result<UserProfile, AuthenticationError>>();
    ASSERT_FALSE(result.isSuccess());
    EXPECT_EQ(result.error().reason, AuthenticationError::Reason::INVALID_CREDENTIALS);
}
```

### 13.2 Integration Tests

```cpp
TEST_F(AuthenticationIntegrationTest, LoginWorkflow_ValidNurse_CreatesSessionAndGrantsAccess) {
    // Setup
    MockUserManagementService mockService;
    SecurityService security(&mockService);
    AuthenticationController controller(&security);
    
    // Act: Simulate login
    controller.login("NURSE001", "1234");
    QTest::qWait(600);  // Wait for async response
    
    // Assert: Session created
    ASSERT_TRUE(security.hasActiveSession());
    EXPECT_EQ(security.getCurrentUserId(), "NURSE001");
    EXPECT_EQ(security.getCurrentRole(), UserRole::NURSE);
    
    // Assert: Permissions granted
    EXPECT_TRUE(security.checkPermission("VIEW_VITALS"));
    EXPECT_TRUE(security.checkPermission("ACKNOWLEDGE_ALARM"));
    EXPECT_FALSE(security.checkPermission("ADJUST_ALARM_THRESHOLDS"));
    
    // Assert: Audit logged
    auto auditLogs = AuditRepository::instance()->getRecentLogs(10);
    EXPECT_TRUE(std::any_of(auditLogs.begin(), auditLogs.end(), [](const AuditEntry& entry) {
        return entry.eventType == "LOGIN_SUCCESS" && entry.userId == "NURSE001";
    }));
}
```

---

## 14. Related Documents

### 14.1 Requirements

- [08_SECURITY_REQUIREMENTS.md](../../requirements/08_SECURITY_REQUIREMENTS.md) - REQ-SEC-AUTH-001 (PIN Authentication), REQ-SEC-AUTHZ-001 (RBAC)
- [03_FUNCTIONAL_REQUIREMENTS.md](../../requirements/03_FUNCTIONAL_REQUIREMENTS.md) - REQ-FUN-USER-001 (User Authentication)
- [04_NON_FUNCTIONAL_REQUIREMENTS.md](../../requirements/04_NON_FUNCTIONAL_REQUIREMENTS.md) - REQ-NFR-SEC-001 (Authentication Security)

### 14.2 Design

- [09_CLASS_DESIGNS.md](./09_CLASS_DESIGNS.md) - SecurityService, AuthenticationService
- [10_DATABASE_DESIGN.md](./10_DATABASE_DESIGN.md) - `security_audit_log` table
- [12_THREAD_MODEL.md](./12_THREAD_MODEL.md) - Network I/O Thread (authentication runs here)
- [29_SYSTEM_COMPONENTS.md](./29_SYSTEM_COMPONENTS.md) - SecurityService, AuthenticationController

### 14.3 Interfaces

- **[interfaces/IUserManagementService.md](./interfaces/IUserManagementService.md)** - **Complete interface specification** ⭐
- [interfaces/IPatientLookupService.md](./interfaces/IPatientLookupService.md) - Similar external service pattern
- [45_ITELEMETRY_SERVER.md](./45_ITELEMETRY_SERVER.md) - Similar network service pattern

---

**Document Version:** 1.0  
**Status:** Draft - Ready for Review  
**Last Updated:** 2025-11-27

*This document defines the authentication and authorization workflow for Z Monitor, including hospital server integration and mock service for development/testing.*

