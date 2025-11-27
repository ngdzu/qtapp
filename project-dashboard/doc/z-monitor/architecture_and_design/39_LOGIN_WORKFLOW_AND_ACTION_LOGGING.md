# Login Workflow and Action Logging

**Document ID:** DESIGN-039  
**Version:** 1.0  
**Status:** Draft  
**Last Updated:** 2025-11-27

---

## 1. Overview

This document defines the updated login workflow for Z Monitor, specifying which actions require authentication and which do not. It also details the action logging strategy, including schema design and dependency injection for logging services.

**Key Principles:**
- **Viewing vitals does NOT require login** - Device displays vitals normally after patient assignment
- **Configuration actions REQUIRE login** - Settings changes, patient admission/discharge require authentication
- **Auto-logout after inactivity** - 5 minutes of inactivity after configuration actions triggers automatic logout
- **All actions are logged** - Login, logout, auto-logout, and all configuration actions are persisted to action log
- **Dependency injection for logging** - No global log objects, all services receive logging dependencies via constructor

---

## 2. Login Workflow

### 2.1 Pre-Patient Assignment

**Before assigning device to a patient:**
- ✅ **Nurse MUST login** before admitting a patient
- Login screen is shown when attempting to access Admission Modal
- After successful login, nurse can proceed with patient admission

**Flow:**
```
User attempts to admit patient
  ↓
Check if user is logged in
  ↓
If NOT logged in → Show Login Screen
  ↓
User enters credentials
  ↓
Authentication successful → Show Admission Modal
  ↓
Nurse admits patient
  ↓
Patient assigned → Vitals display normally (NO login required for viewing)
```

### 2.2 Post-Patient Assignment

**After patient is assigned:**
- ✅ **Vitals display normally** - No login required to view vitals, waveforms, trends
- ✅ **Alarms display normally** - No login required to view alarms
- ✅ **Notifications accessible** - No login required to view notifications
- ❌ **Configuration requires login** - Settings changes, patient admission/discharge require authentication
- ❌ **Clearing notifications requires login** - Clearing recent notifications requires authentication

### 2.3 Configuration Actions (Require Login)

**Actions that require authentication:**
1. **Patient Management:**
   - Admit patient (assign patient to device)
   - Discharge patient (remove patient from device)
   - Transfer patient (change patient assignment)

2. **Settings Changes:**
   - Adjust alarm thresholds
   - Change device configuration
   - Modify system settings
   - Access provisioning mode
   - Export data

3. **Notification Management:**
   - Clear recent notifications
   - Dismiss notification history

4. **Administrative Actions:**
   - View audit logs
   - Access diagnostics
   - System maintenance
   - Firmware updates

### 2.4 View-Only Actions (No Login Required)

**Actions that do NOT require authentication:**
1. **Vitals Monitoring:**
   - View current vitals
   - View waveforms
   - View trends
   - View historical data

2. **Alarm Monitoring:**
   - View active alarms
   - View alarm history
   - View alarm details

3. **Notifications:**
   - View notifications
   - View notification history
   - Read notification details

4. **Patient Information:**
   - View patient name
   - View patient MRN
   - View patient demographics (if already displayed)

---

## 3. Auto-Logout Behavior

### 3.1 Inactivity Timer

**After configuration actions:**
- ⏱️ **5-minute inactivity timer** starts after any configuration action
- Timer resets on any configuration action
- Timer cancels on manual logout
- After 5 minutes of inactivity → **Automatic logout**

**Configuration Actions that trigger inactivity timer:**
- Admit/discharge/transfer patient
- Change any setting
- Adjust alarm thresholds
- Clear notifications
- Access diagnostics
- Any administrative action

**Actions that do NOT reset inactivity timer:**
- Viewing vitals
- Viewing alarms
- Viewing notifications
- Viewing trends
- Scrolling through data

### 3.2 Auto-Logout Flow

```
User performs configuration action (e.g., adjust threshold)
  ↓
Action completed successfully
  ↓
Start 5-minute inactivity timer
  ↓
[User views vitals/alarms - timer continues]
  ↓
[5 minutes pass with no configuration actions]
  ↓
Auto-logout triggered
  ↓
Log "AUTO_LOGOUT" to action_log
  ↓
Clear user session
  ↓
Return to view-only mode (vitals still display)
  ↓
Show notification: "Session expired due to inactivity"
```

### 3.3 Manual Logout

**User-initiated logout:**
- User clicks "Logout" button
- Session cleared immediately
- Log "USER_LOGOUT" to action_log
- Return to view-only mode
- No inactivity timer (already logged out)

---

## 4. Action Logging Requirements

### 4.1 Actions That Must Be Logged

**Authentication Actions:**
- `LOGIN` - User successfully logged in
- `LOGIN_FAILED` - Login attempt failed
- `LOGOUT` - User manually logged out
- `AUTO_LOGOUT` - Automatic logout due to inactivity
- `SESSION_EXPIRED` - Session expired (timeout)

**Patient Management Actions:**
- `ADMIT_PATIENT` - Patient admitted to device
- `DISCHARGE_PATIENT` - Patient discharged from device
- `TRANSFER_PATIENT` - Patient transferred to different device

**Settings Actions:**
- `CHANGE_SETTING` - Any setting changed (include setting name and value)
- `ADJUST_ALARM_THRESHOLD` - Alarm threshold adjusted
- `RESET_SETTINGS` - Settings reset to defaults

**Notification Actions:**
- `CLEAR_NOTIFICATIONS` - Recent notifications cleared
- `DISMISS_NOTIFICATION` - Single notification dismissed

**Administrative Actions:**
- `VIEW_AUDIT_LOG` - Audit log accessed
- `EXPORT_DATA` - Data exported
- `ACCESS_DIAGNOSTICS` - Diagnostics view accessed
- `PROVISIONING_MODE_ENTERED` - Device entered provisioning mode

**View Actions (Optional - for analytics):**
- `VIEW_VITALS` - Vitals viewed (optional, can be throttled)
- `VIEW_ALARMS` - Alarms viewed (optional)
- `VIEW_NOTIFICATIONS` - Notifications viewed (optional)

### 4.2 Log Entry Format

Each action log entry must include:
- **Timestamp** - When action occurred
- **User ID** - Who performed action (NULL if no login required)
- **Action Type** - Type of action (see above)
- **Target ID** - What was acted upon (patient MRN, setting name, etc.)
- **Details** - JSON string with additional context
- **Result** - SUCCESS, FAILURE, or PARTIAL
- **Device ID** - Device identifier

---

## 5. Action Log Schema Design

### 5.1 Enhanced `action_log` Table

**Current `audit_log` table is too generic. We need a dedicated `action_log` table for user actions:**

```sql
CREATE TABLE IF NOT EXISTS action_log (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    timestamp_ms INTEGER NOT NULL,              -- Unix timestamp in milliseconds
    timestamp_iso TEXT NOT NULL,                 -- ISO 8601 timestamp for readability
    user_id TEXT NULL,                           -- User who performed action (NULL if no login)
    user_role TEXT NULL,                          -- User role (NURSE, PHYSICIAN, etc.)
    action_type TEXT NOT NULL,                   -- Action type (see Section 4.1)
    target_type TEXT NULL,                        -- Type of target (PATIENT, SETTING, NOTIFICATION, etc.)
    target_id TEXT NULL,                          -- Target identifier (MRN, setting name, notification ID)
    details TEXT NULL,                            -- JSON string with additional context
    result TEXT NOT NULL,                         -- SUCCESS, FAILURE, PARTIAL
    error_code TEXT NULL,                         -- Error code if result is FAILURE
    error_message TEXT NULL,                      -- Error message if result is FAILURE
    device_id TEXT NOT NULL,                      -- Device identifier
    session_token_hash TEXT NULL,                 -- SHA-256 hash of session token (for audit trail)
    ip_address TEXT NULL,                          -- IP address (if available, for network actions)
    previous_hash TEXT NULL                       -- SHA-256 hash of previous entry (hash chain for tamper detection)
);

-- Indexes for common queries
CREATE INDEX IF NOT EXISTS idx_action_log_timestamp ON action_log(timestamp_ms DESC);
CREATE INDEX IF NOT EXISTS idx_action_log_user ON action_log(user_id, timestamp_ms DESC);
CREATE INDEX IF NOT EXISTS idx_action_log_action_type ON action_log(action_type, timestamp_ms DESC);
CREATE INDEX IF NOT EXISTS idx_action_log_target ON action_log(target_type, target_id, timestamp_ms DESC);
CREATE INDEX IF NOT EXISTS idx_action_log_device ON action_log(device_id, timestamp_ms DESC);
```

### 5.2 Hash Chain for Tamper Detection

**Same as `security_audit_log`, implement hash chain:**

```sql
-- Compute hash for entry N
-- previous_hash = SHA-256(
--   prev.id || 
--   prev.timestamp_ms || 
--   prev.action_type || 
--   prev.user_id || 
--   prev.target_id || 
--   prev.details || 
--   prev.result
-- )
```

### 5.3 Example Log Entries

**Login Action:**
```json
{
  "timestamp_ms": 1632500000123,
  "timestamp_iso": "2025-11-27T14:30:00.123Z",
  "user_id": "NURSE001",
  "user_role": "NURSE",
  "action_type": "LOGIN",
  "target_type": null,
  "target_id": null,
  "details": "{\"login_method\":\"secret_code\",\"device_id\":\"ZM-ICU-04\"}",
  "result": "SUCCESS",
  "error_code": null,
  "error_message": null,
  "device_id": "ZM-ICU-04",
  "session_token_hash": "a1b2c3d4e5f6...",
  "ip_address": null,
  "previous_hash": "previous-entry-hash"
}
```

**Admit Patient Action:**
```json
{
  "timestamp_ms": 1632500100123,
  "timestamp_iso": "2025-11-27T14:31:40.123Z",
  "user_id": "NURSE001",
  "user_role": "NURSE",
  "action_type": "ADMIT_PATIENT",
  "target_type": "PATIENT",
  "target_id": "MRN-12345",
  "details": "{\"patient_name\":\"John Doe\",\"admission_method\":\"manual\",\"bed_location\":\"ICU-4B\"}",
  "result": "SUCCESS",
  "error_code": null,
  "error_message": null,
  "device_id": "ZM-ICU-04",
  "session_token_hash": "a1b2c3d4e5f6...",
  "ip_address": null,
  "previous_hash": "previous-entry-hash"
}
```

**Auto-Logout Action:**
```json
{
  "timestamp_ms": 1632500400123,
  "timestamp_iso": "2025-11-27T14:40:00.123Z",
  "user_id": "NURSE001",
  "user_role": "NURSE",
  "action_type": "AUTO_LOGOUT",
  "target_type": null,
  "target_id": null,
  "details": "{\"inactivity_duration_seconds\":300,\"last_action\":\"ADJUST_ALARM_THRESHOLD\",\"last_action_timestamp\":1632500100123}",
  "result": "SUCCESS",
  "error_code": null,
  "error_message": null,
  "device_id": "ZM-ICU-04",
  "session_token_hash": "a1b2c3d4e5f6...",
  "ip_address": null,
  "previous_hash": "previous-entry-hash"
}
```

**Clear Notifications Action:**
```json
{
  "timestamp_ms": 1632500200123,
  "timestamp_iso": "2025-11-27T14:33:20.123Z",
  "user_id": "NURSE001",
  "user_role": "NURSE",
  "action_type": "CLEAR_NOTIFICATIONS",
  "target_type": "NOTIFICATION",
  "target_id": null,
  "details": "{\"notification_count\":5,\"cleared_types\":[\"ALARM\",\"SYSTEM\"]}",
  "result": "SUCCESS",
  "error_code": null,
  "error_message": null,
  "device_id": "ZM-ICU-04",
  "session_token_hash": "a1b2c3d4e5f6...",
  "ip_address": null,
  "previous_hash": "previous-entry-hash"
}
```

---

## 6. Dependency Injection for Logging

### 6.1 Problem with Global Log Objects

**Current approach (problematic):**
```cpp
// ❌ BAD: Global singleton
LogService::instance()->info("User logged in");
```

**Problems:**
- Hard to test (can't mock global object)
- Tight coupling
- Hidden dependencies
- Difficult to swap implementations

### 6.2 Dependency Injection Approach

**New approach (recommended):**
```cpp
// ✅ GOOD: Dependency injection
class SecurityService {
public:
    SecurityService(IAuditRepository* auditRepo, 
                   IActionLogRepository* actionLogRepo,
                   QObject* parent = nullptr);
    
    void login(const QString& userId, const QString& secretCode) {
        // ... authentication logic ...
        
        // Log action via injected dependency
        m_actionLogRepo->logAction(ActionLogEntry{
            .actionType = "LOGIN",
            .userId = userId,
            .result = "SUCCESS"
        });
    }
    
private:
    IAuditRepository* m_auditRepo;
    IActionLogRepository* m_actionLogRepo;
};
```

### 6.3 IActionLogRepository Interface

```cpp
/**
 * @interface IActionLogRepository
 * @brief Repository for persisting user actions to action_log table.
 * 
 * Provides abstraction for logging user actions (login, logout, configuration changes, etc.)
 * to the action_log table for audit and compliance purposes.
 * 
 * @note All methods are asynchronous and non-blocking
 * @note Runs on Database I/O Thread
 * 
 * @ingroup RepositoryInterfaces
 */
class IActionLogRepository : public QObject {
    Q_OBJECT
public:
    virtual ~IActionLogRepository() = default;
    
    /**
     * @brief Log a user action to action_log table.
     * 
     * @param entry Action log entry to persist
     * @return Result<void, Error> - Success or error
     * 
     * @note This method is asynchronous and non-blocking
     * @note Entry is queued and written to database on background thread
     */
    virtual void logAction(const ActionLogEntry& entry) = 0;
    
    /**
     * @brief Log multiple actions in a batch (for performance).
     * 
     * @param entries List of action log entries
     * @return Result<void, Error>
     */
    virtual void logActions(const QList<ActionLogEntry>& entries) = 0;
    
    /**
     * @brief Query action log entries.
     * 
     * @param filter Filter criteria (user_id, action_type, date range, etc.)
     * @return Result<QList<ActionLogEntry>, Error>
     */
    virtual void queryActions(const ActionLogFilter& filter) = 0;
    
signals:
    /**
     * @brief Emitted when action is successfully logged.
     */
    void actionLogged(const ActionLogEntry& entry);
    
    /**
     * @brief Emitted when action logging fails.
     */
    void actionLogFailed(const ActionLogEntry& entry, const Error& error);
};

/**
 * @struct ActionLogEntry
 * @brief Represents a single action log entry.
 */
struct ActionLogEntry {
    QString userId;              ///< User who performed action (empty if no login)
    QString userRole;            ///< User role (NURSE, PHYSICIAN, etc.)
    QString actionType;          ///< Action type (LOGIN, ADMIT_PATIENT, etc.)
    QString targetType;          ///< Type of target (PATIENT, SETTING, etc.)
    QString targetId;            ///< Target identifier (MRN, setting name, etc.)
    QJsonObject details;         ///< Additional context (JSON object)
    QString result;              ///< SUCCESS, FAILURE, PARTIAL
    QString errorCode;           ///< Error code if result is FAILURE
    QString errorMessage;       ///< Error message if result is FAILURE
    QString sessionTokenHash;    ///< SHA-256 hash of session token
    QString ipAddress;           ///< IP address (if available)
};
```

### 6.4 Implementation: SQLiteActionLogRepository

```cpp
/**
 * @class SQLiteActionLogRepository
 * @brief SQLite implementation of IActionLogRepository.
 * 
 * Persists action log entries to action_log table in SQLite database.
 * Runs on Database I/O Thread for non-blocking writes.
 * 
 * @thread Database I/O Thread
 */
class SQLiteActionLogRepository : public IActionLogRepository {
    Q_OBJECT
public:
    explicit SQLiteActionLogRepository(DatabaseManager* dbManager, QObject* parent = nullptr);
    
    void logAction(const ActionLogEntry& entry) override;
    void logActions(const QList<ActionLogEntry>& entries) override;
    void queryActions(const ActionLogFilter& filter) override;
    
private:
    DatabaseManager* m_dbManager;
    QQueue<ActionLogEntry> m_pendingEntries;  ///< Queue for batch writes
    QTimer* m_flushTimer;                     ///< Timer for periodic batch writes
    
    void flushPendingEntries();
    QString computePreviousHash(qint64 previousId);
};
```

### 6.5 Usage Example

```cpp
// In SecurityService constructor
SecurityService::SecurityService(
    IUserManagementService* userMgmtService,
    IAuditRepository* auditRepo,
    IActionLogRepository* actionLogRepo,  // ← Injected dependency
    QObject* parent)
    : QObject(parent)
    , m_userMgmtService(userMgmtService)
    , m_auditRepo(auditRepo)
    , m_actionLogRepo(actionLogRepo)  // ← Store injected dependency
{
}

void SecurityService::login(const QString& userId, const QString& secretCode) {
    // ... authentication logic ...
    
    if (authenticationSuccessful) {
        // Log to action_log via injected repository
        ActionLogEntry entry;
        entry.userId = userId;
        entry.userRole = userProfile.role;
        entry.actionType = "LOGIN";
        entry.result = "SUCCESS";
        entry.details = QJsonObject{
            {"login_method", "secret_code"},
            {"device_id", getDeviceLabel()}
        };
        entry.sessionTokenHash = hashSessionToken(userProfile.sessionToken);
        
        m_actionLogRepo->logAction(entry);
        
        emit userLoggedIn(userId, userProfile.role);
    }
}

void SecurityService::handleAutoLogout() {
    // ... logout logic ...
    
    // Log auto-logout
    ActionLogEntry entry;
    entry.userId = m_currentSession->userProfile.userId;
    entry.userRole = m_currentSession->userProfile.role;
    entry.actionType = "AUTO_LOGOUT";
    entry.result = "SUCCESS";
    entry.details = QJsonObject{
        {"inactivity_duration_seconds", m_inactivityTimer->remainingTime() / 1000},
        {"last_action", m_lastActionType},
        {"last_action_timestamp", m_lastActionTimestamp}
    };
    entry.sessionTokenHash = hashSessionToken(m_currentSession->sessionToken);
    
    m_actionLogRepo->logAction(entry);
    
    // Clear session
    m_currentSession = std::nullopt;
    emit userLoggedOut();
}
```

---

## 7. Updated State Machine

**State Machine Diagrams:**
- [View Application State Machine (Mermaid)](./05_STATE_MACHINES.mmd) - Updated with VIEW_ONLY and CONFIGURATION_MODE states
- [View Application State Machine (SVG)](./05_STATE_MACHINES.svg)
- [View Login Workflow State Machine (Mermaid)](./39_LOGIN_WORKFLOW.mmd) - Complete login workflow with inactivity timer
- [View Login Workflow State Machine (SVG)](./39_LOGIN_WORKFLOW.svg)

### 7.1 Application State Machine (Updated)

**States:**
- `STARTUP` - Application starting
- `LOGIN_REQUIRED` - Login screen shown (before patient assignment or for configuration)
- `VIEW_ONLY` - Viewing vitals/alarms (no login required, patient assigned)
- `CONFIGURATION_MODE` - Configuration active (user logged in, inactivity timer running)
- `MONITORING` - Normal monitoring (patient assigned, vitals displaying)

**Transitions:**
```
STARTUP
  ↓
[Check if patient assigned]
  ↓
If NO patient → LOGIN_REQUIRED (to admit patient)
If YES patient → VIEW_ONLY (vitals display normally)
  ↓
[User attempts configuration action]
  ↓
VIEW_ONLY → LOGIN_REQUIRED (show login screen)
  ↓
[User logs in successfully]
  ↓
LOGIN_REQUIRED → CONFIGURATION_MODE (inactivity timer starts)
  ↓
[User performs configuration action]
  ↓
CONFIGURATION_MODE → CONFIGURATION_MODE (timer resets)
  ↓
[5 minutes inactivity OR user logs out]
  ↓
CONFIGURATION_MODE → VIEW_ONLY (auto-logout or manual logout)
  ↓
[User attempts configuration action again]
  ↓
VIEW_ONLY → LOGIN_REQUIRED (cycle repeats)
```

### 7.2 Login State Machine (Updated)

**States:**
- `LOGGED_OUT` - No active session
- `LOGGING_IN` - Authentication in progress
- `LOGGED_IN` - Active session (configuration mode)
- `INACTIVITY_WARNING` - 4 minutes passed (1 minute before auto-logout)
- `AUTO_LOGGING_OUT` - Auto-logout in progress

**Transitions:**
```
LOGGED_OUT
  ↓
[User attempts configuration action]
  ↓
LOGGING_IN (show login screen, authenticate)
  ↓
[Authentication successful]
  ↓
LOGGED_IN (start 5-minute inactivity timer)
  ↓
[Configuration action performed]
  ↓
LOGGED_IN (reset inactivity timer)
  ↓
[4 minutes pass]
  ↓
INACTIVITY_WARNING (show "1 minute until logout" warning)
  ↓
[1 more minute passes OR user performs action]
  ↓
If action → LOGGED_IN (reset timer)
If timeout → AUTO_LOGGING_OUT
  ↓
AUTO_LOGGING_OUT (log action, clear session)
  ↓
LOGGED_OUT (return to view-only mode)
```

---

## 8. Action Permission Matrix

### 8.1 Complete Action List

| Action | Requires Login? | Logged? | Notes |
|--------|----------------|---------|-------|
| **Viewing Actions** |
| View Vitals | ❌ No | ⚠️ Optional | View-only, no login required |
| View Waveforms | ❌ No | ⚠️ Optional | View-only, no login required |
| View Trends | ❌ No | ⚠️ Optional | View-only, no login required |
| View Alarms | ❌ No | ⚠️ Optional | View-only, no login required |
| View Notifications | ❌ No | ⚠️ Optional | View-only, no login required |
| View Patient Info | ❌ No | ⚠️ Optional | View-only, no login required |
| **Patient Management** |
| Admit Patient | ✅ Yes | ✅ Yes | Configuration action |
| Discharge Patient | ✅ Yes | ✅ Yes | Configuration action |
| Transfer Patient | ✅ Yes | ✅ Yes | Configuration action |
| **Settings** |
| Adjust Alarm Thresholds | ✅ Yes | ✅ Yes | Configuration action |
| Change Device Settings | ✅ Yes | ✅ Yes | Configuration action |
| Access System Settings | ✅ Yes | ✅ Yes | Configuration action |
| Reset Settings | ✅ Yes | ✅ Yes | Configuration action |
| **Notifications** |
| Clear Notifications | ✅ Yes | ✅ Yes | Configuration action |
| Dismiss Notification | ✅ Yes | ✅ Yes | Configuration action |
| **Administrative** |
| View Audit Log | ✅ Yes | ✅ Yes | Administrative action |
| Export Data | ✅ Yes | ✅ Yes | Administrative action |
| Access Diagnostics | ✅ Yes | ✅ Yes | Administrative action |
| Enter Provisioning Mode | ✅ Yes | ✅ Yes | Administrative action |
| **Authentication** |
| Login | N/A | ✅ Yes | Authentication action |
| Logout | N/A | ✅ Yes | Authentication action |
| Auto-Logout | N/A | ✅ Yes | Authentication action |

---

## 9. Implementation Checklist

- [ ] Create `action_log` table schema (DDL)
- [ ] Create `IActionLogRepository` interface
- [ ] Implement `SQLiteActionLogRepository`
- [ ] Update `SecurityService` to use dependency injection for `IActionLogRepository`
- [ ] Update `AdmissionService` to log patient management actions
- [ ] Update `SettingsController` to log settings changes
- [ ] Update `NotificationController` to log notification clearing
- [ ] Implement inactivity timer in `SecurityService` (5 minutes)
- [ ] Implement inactivity warning (4 minutes, show "1 minute remaining")
- [ ] Update state machine diagrams (Mermaid)
- [ ] Update `LoginView.qml` to show inactivity warning
- [ ] Update application state machine to handle VIEW_ONLY vs CONFIGURATION_MODE
- [ ] Add permission checks before configuration actions
- [ ] Update all services to use dependency injection for logging (remove global LogService::instance())
- [ ] Write unit tests for action logging
- [ ] Write integration tests for auto-logout workflow
- [ ] Update documentation (this file)

---

## 10. Related Documents

- [38_AUTHENTICATION_WORKFLOW.md](./38_AUTHENTICATION_WORKFLOW.md) - Hospital authentication integration
- [10_DATABASE_DESIGN.md](./10_DATABASE_DESIGN.md) - Database schema (update with `action_log` table)
- [21_LOGGING_STRATEGY.md](./21_LOGGING_STRATEGY.md) - Logging strategy (update with dependency injection)
- [05_STATE_MACHINES.md](./05_STATE_MACHINES.md) - State machine designs (update login state machine)
- [09_CLASS_DESIGNS.md](./09_CLASS_DESIGNS.md) - Class designs (update SecurityService, add IActionLogRepository)
- [13_DEPENDENCY_INJECTION.md](./13_DEPENDENCY_INJECTION.md) - Dependency injection strategy

---

**Document Version:** 1.0  
**Status:** Draft - Ready for Review  
**Last Updated:** 2025-11-27

*This document defines the updated login workflow, action logging requirements, and dependency injection strategy for logging services.*

