---
doc_id: DOC-COMP-017
title: IProvisioningRepository
version: v1.0
category: Component
subcategory: Application Layer / Repository Interface
status: Draft
owner: Application Layer Team
reviewers: 
  - Architecture Team
last_reviewed: 2025-01-26
next_review: 2026-01-26
related_docs:
  - DOC-ARCH-002  # System architecture
  - DOC-COMP-003  # DeviceAggregate
  - DOC-COMP-012  # ProvisioningService
related_tasks:
  - TASK-3B-001  # Phase 3B Migration
tags:
  - repository
  - persistence
  - provisioning
  - device
diagram_files: []
---

# DOC-COMP-017: IProvisioningRepository

## 1. Overview

**Purpose:** Repository interface for persisting and retrieving device provisioning data (device configurations and provisioning sessions with pairing codes), following DDD repository pattern.

**Responsibilities:**
- Save device configuration to persistence store
- Find device by device ID
- Save provisioning session (pairing code, QR code metadata)
- Find provisioning session by pairing code (for QR code validation)
- Delete expired provisioning sessions (cleanup after 10-minute QR expiry)

**Layer:** Application Layer (Repository Interface)

**Module:** `z-monitor/src/domain/repositories/IProvisioningRepository.h`

**Implementation:** SQLiteProvisioningRepository (infrastructure layer)

**Thread Affinity:** Database I/O Thread (single writer thread for SQLite thread safety)

## 2. Public API

```cpp
class IProvisioningRepository {
public:
    virtual ~IProvisioningRepository() = default;

    /**
     * @brief Save device configuration.
     * @param device Device aggregate to save
     * @return Result<void, Error> Success or error
     */
    virtual Result<void, Error> saveDevice(std::shared_ptr<DeviceAggregate> device) = 0;

    /**
     * @brief Find device by device ID.
     * @param deviceId Device unique identifier
     * @return Result<std::shared_ptr<DeviceAggregate>> Device or error
     */
    virtual Result<std::shared_ptr<DeviceAggregate>, Error> findDeviceById(const QString& deviceId) const = 0;

    /**
     * @brief Save provisioning session.
     * @param session Provisioning session to save (pairing code, QR metadata, expiry)
     * @return Result<void, Error> Success or error
     */
    virtual Result<void, Error> saveSession(const ProvisioningSession& session) = 0;

    /**
     * @brief Find provisioning session by pairing code.
     * @param pairingCode 6-character pairing code (e.g., "X7Y9Z2")
     * @return Result<ProvisioningSession> Session or error
     */
    virtual Result<ProvisioningSession, Error> findSessionByCode(const QString& pairingCode) const = 0;

    /**
     * @brief Delete expired provisioning sessions.
     * @return Result<int, Error> Number of sessions deleted or error
     */
    virtual Result<int, Error> deleteExpiredSessions() = 0;
};
```

## 3. Implementation Details

**SQLiteProvisioningRepository Implementation:**
- Database table 1: `devices` (columns: device_id, device_label, server_url, provisioning_status, created_at, updated_at)
- Database table 2: `provisioning_sessions` (columns: pairing_code, device_id, device_ip, created_at, expires_at)
- saveSession(): Inserts session with 10-minute expiry
- findSessionByCode(): Queries `WHERE pairing_code=? AND expires_at > current_timestamp`
- deleteExpiredSessions(): Deletes `WHERE expires_at < current_timestamp`

**Error Handling:**
- Uses `Result<T, Error>` type for all operations
- Database errors: Returns `Error::DATABASE_QUERY_FAILED`
- Session not found: Returns `Error::SESSION_NOT_FOUND`
- Device not found: Returns `Error::DEVICE_NOT_FOUND`

## 4. Usage Examples

### 4.1 Save Provisioning Session

```cpp
ProvisioningSession session;
session.pairingCode = "X7Y9Z2";
session.deviceId = "ZM-ICU-MON-04";
session.deviceIp = "10.1.50.104";
session.createdAt = QDateTime::currentDateTimeUtc();
session.expiresAt = session.createdAt.addSecs(600); // 10 minutes

auto result = provisioningRepo->saveSession(session);
if (result.isSuccess()) {
    qInfo() << "Provisioning session saved";
}
```

### 4.2 Find Session by Pairing Code

```cpp
auto result = provisioningRepo->findSessionByCode("X7Y9Z2");
if (result.isSuccess()) {
    auto session = result.value();
    qInfo() << "Found session for device:" << session.deviceId;
} else {
    qWarning() << "Session not found or expired:" << result.error().message();
}
```

### 4.3 Clean Up Expired Sessions

```cpp
auto result = provisioningRepo->deleteExpiredSessions();
if (result.isSuccess()) {
    qInfo() << "Deleted" << result.value() << "expired sessions";
}
```

## 5. Related Documentation

- DOC-COMP-003: DeviceAggregate - Device domain entity
- DOC-COMP-012: ProvisioningService - Provisioning workflow
- SQLiteProvisioningRepository Implementation - Infrastructure layer persistence

## 6. Changelog

| Version | Date       | Author      | Changes                                              |
| ------- | ---------- | ----------- | ---------------------------------------------------- |
| v1.0    | 2025-01-26 | Dustin Wind | Initial documentation from IProvisioningRepository.h |
