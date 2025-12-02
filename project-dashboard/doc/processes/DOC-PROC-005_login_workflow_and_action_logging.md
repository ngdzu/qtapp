---
doc_id: DOC-PROC-005
title: Login Workflow and Action Logging
version: 1.0
category: Process
subcategory: Security
status: Draft
owner: Security Team
reviewers:
  - Security Team
last_reviewed: 2025-12-01
next_review: 2026-03-01
related_docs:
  - DOC-PROC-004 # Authentication & Authorization Workflow
  - DOC-REQ-008  # Security Requirements
  - DOC-REQ-010  # Requirements Traceability
related_requirements:
  - REQ-SEC-AUTH-001
  - REQ-SEC-AUTHZ-001
  - REQ-SEC-AUDIT-001
tags:
  - process
  - workflow
  - login
  - session
  - logging
  - audit
diagram_files:
  - processes/diagrams/39_LOGIN_WORKFLOW.svg
---

# DOC-PROC-005: Login Workflow and Action Logging

## 1. Overview

**Purpose:** Define how login is enforced for privileged actions, how session timeout/auto‑logout behaves, and how actions are logged for audit and compliance.

**Scope:** Applies to all UI flows that perform configuration, administrative, or patient‑affecting actions. View‑only operations are explicitly out of scope for login gating but are in scope for read‑only access rules.

**Stakeholders:** Clinical staff (Nurse, Physician), Administrators, Security/Compliance, QA.

**Triggers:** Attempting a privileged action; session inactivity thresholds; authentication state changes.

**Outcomes:** Authorized actions proceed; unauthorized users are challenged; all actions produce immutable audit entries; inactive sessions automatically terminate.

---

## 2. Access Model

- View‑only (no login): vitals, waveforms, trends, alarms, notifications, patient info
- Requires login: patient admit/discharge/transfer; alarm threshold changes; device/system settings; diagnostics; data export; clearing notifications; provisioning; administrative screens

---

## 2. Process Flow

![Process Flowchart](diagrams/DOC-PROC-005_workflow.svg)

## 3. Steps

### 4.1 Step 1: Initiate Privileged Action
**Responsible:** User (Nurse/Physician/Admin)

**Prerequisites:** Device provisioned; app running; user at view‑only context.

**Actions:**
1. User attempts action that requires elevated permission (e.g., admit patient, change thresholds).
2. System evaluates current session state and permissions.

**Outputs:** Decision on whether to prompt for login.

**Success Criteria:** Correctly identifies actions that require login.

### 4.2 Step 2: Authenticate
**Responsible:** User; AuthenticationService

**Actions:**
1. Display login prompt (PIN or configured method).
2. Verify credentials; log `LOGIN` or `LOGIN_FAILED`.

**Outputs:** Authenticated session token (on success).

**Success Criteria:** Auth < 5s; brute‑force protections enforced.

### 4.3 Step 3: Execute Action
**Responsible:** Feature Owner; SecurityService (for action logging)

**Actions:** Perform requested change; log action with context and result (`SUCCESS`/`FAILURE`).

**Outputs:** Domain change; audit entry in `action_log`.

**Success Criteria:** Correct mutation; durable audit entry.

### 4.4 Step 4: Inactivity Timer and Auto‑Logout
**Responsible:** SessionManager

**Actions:**
1. Start/reset inactivity timer (5 minutes) on each privileged action.
2. Issue T‑1 minute warning; on timeout, clear session and log `AUTO_LOGOUT`.

**Outputs:** Session termination; return to view‑only.

**Success Criteria:** No privileged action possible post‑timeout.

---

## 4. Decision Points

### 5.1 Requires Login?
- If action is view‑only → proceed without challenge.
- If action is privileged and session not authenticated → prompt for login.
- Else proceed with authorization check (RBAC).

### 5.2 Auto‑Logout Warning
- If inactivity ≥ 4 minutes → show warning dialog; any privileged action resets timer.

### 5.3 Emergency Override
- If emergency mode enabled → allow minimal set of actions, flag entries, escalate review.

## 5. Action Logging

### 4.1 Required Events
Authentication: `LOGIN`, `LOGIN_FAILED`, `LOGOUT`, `AUTO_LOGOUT`, `SESSION_EXPIRED`
Patient: `ADMIT_PATIENT`, `DISCHARGE_PATIENT`, `TRANSFER_PATIENT`
Settings: `CHANGE_SETTING`, `ADJUST_ALARM_THRESHOLD`, `RESET_SETTINGS`
Notifications: `CLEAR_NOTIFICATIONS`, `DISMISS_NOTIFICATION`
Admin: `VIEW_AUDIT_LOG`, `EXPORT_DATA`, `ACCESS_DIAGNOSTICS`, `PROVISIONING_MODE_ENTERED`

### 6.2 Schema
```sql
CREATE TABLE IF NOT EXISTS action_log (
  id INTEGER PRIMARY KEY AUTOINCREMENT,
  timestamp_ms INTEGER NOT NULL,
  timestamp_iso TEXT NOT NULL,
  user_id TEXT NULL,
  user_role TEXT NULL,
  action_type TEXT NOT NULL,
  target_type TEXT NULL,
  target_id TEXT NULL,
  details TEXT NULL,
  result TEXT NOT NULL,
  error_code TEXT NULL,
  error_message TEXT NULL,
  device_id TEXT NOT NULL,
  session_token_hash TEXT NULL,
  ip_address TEXT NULL,
  previous_hash TEXT NULL
);
```

### 6.3 DI, Not Singletons
```cpp
class SecurityService {
public:
  SecurityService(IActionLogRepository* actionLog, IAuditRepository* audit, QObject* parent=nullptr);
  // ...
  void logLoginSuccess(const UserProfile& user) {
    ActionLogEntry e; e.userId=user.userId; e.userRole=user.role; e.actionType="LOGIN"; e.result="SUCCESS";
    e.details = QJsonObject{{"device_id", getDeviceLabel()}};
    e.sessionTokenHash = hashSessionToken(user.sessionToken);
    m_actionLog->logAction(e);
  }
private:
  IActionLogRepository* m_actionLog;
};
```

---

## 6. State Machines (summary)
- App: STARTUP → LOGIN_REQUIRED (for config) or VIEW_ONLY (if patient) → CONFIGURATION_MODE → VIEW_ONLY (logout or timeout)
- Login: LOGGED_OUT → LOGGING_IN → LOGGED_IN (timer) → INACTIVITY_WARNING → AUTO_LOGGING_OUT → LOGGED_OUT

Diagrams (ensure copied):
- `processes/diagrams/39_LOGIN_WORKFLOW.svg`

---

## 7. Testing

- Unit: inactivity timer; logging repository writes; permission gates
- Integration: admit/discharge actions log entries; settings changes log
- E2E: auto‑logout returns to view‑only; warning dialog at T‑1 minute

---

## 12. Related Documentation
- DOC-PROC-004 Authentication and Authorization Workflow

---

## 10. Quality Gates

### 10.1 Security Review
Criteria:
- [ ] Auth challenges for all privileged actions
- [ ] Auto‑logout at 5 minutes of inactivity
- [ ] 100% of privileged actions logged with result
- [ ] Log entries include user, action, target, timestamp, device ID

Reviewers: Security Team

Evidence: Test reports; database samples of `action_log`.

## 11. Error Handling

| Issue                | Symptoms                               | Resolution                                       |
| -------------------- | -------------------------------------- | ------------------------------------------------ |
| Brute‑force lockouts | Frequent `LOGIN_FAILED`, user blocked  | Admin reset with audit trail; verify rate limits |
| Logging failure      | Missing entries in `action_log`        | Fail‑safe: queue and retry; alert admin          |
| Clock drift          | Timestamps inconsistent across devices | NTP sync; store both ms and ISO timestamps       |

## 12. Metrics & KPIs

| Metric                  | Target                     | Measurement             |
| ----------------------- | -------------------------- | ----------------------- |
| Login success latency   | < 5s p95                   | E2E timing tests        |
| Auto‑logout reliability | 100%                       | Automated session tests |
| Action log coverage     | 100% of privileged actions | Query sampling          |

## 13. Compliance & Audit

- Satisfies: REQ‑SEC‑AUTH‑001 (authentication), REQ‑SEC‑AUTHZ‑001 (RBAC), REQ‑SEC‑AUDIT‑001 (audit trail)
- Traceability: DOC‑REQ‑010

## 14. Examples

### 14.1 Admit Patient (Privileged)
1. User taps Admit → Login prompt → Success → Admission Modal → `ADMIT_PATIENT` logged.

### 14.2 Auto‑Logout
1. 5 minutes after last privileged action → warning → session cleared → `AUTO_LOGOUT` logged.

## 15. Related

- DOC‑PROC‑004 Authentication and Authorization Workflow

## 16. Changelog

| Version | Date       | Author         | Changes                                                |
| ------- | ---------- | -------------- | ------------------------------------------------------ |
| 1.0     | 2025-12-01 | Z Monitor Team | Migrated from DESIGN-039; added DI patterns and schema |
