# Network Module: Class Designs

**Document ID:** DESIGN-009e  
**Version:** 1.0  
**Status:** Approved  
**Last Updated:** 2025-11-27

---

This document provides detailed class designs for the **Network Module**, which handles secure network communication on the Network I/O Thread.

> **📋 Related Documents:**
> - [Class Designs Overview (09_CLASS_DESIGNS_OVERVIEW.md)](./09_CLASS_DESIGNS_OVERVIEW.md) - High-level module architecture
> - [Thread Model (12_THREAD_MODEL.md)](./12_THREAD_MODEL.md) - Thread architecture (Section 4.5: Network I/O Thread)
> - [Security (06_SECURITY.md)](./06_SECURITY.md) - Security architecture
> - [Certificate Provisioning (15_CERTIFICATE_PROVISIONING.md)](./15_CERTIFICATE_PROVISIONING.md) - Certificate management

---

## 1. Module Overview

**Thread:** Network I/O Thread  
**Priority:** Normal (background, but not low)  
**Component Count:** 11 components

**Purpose:**
- Transmit telemetry data to central server (mTLS)
- Manage certificates (loading, validation, renewal)
- Encrypt and sign payloads
- Lookup patients from external systems (HIS/EHR)
- Handle provisioning configuration from Central Station

---

## 2. Module Diagram

[View Network Module Diagram (Mermaid)](./09e_NETWORK_MODULE.mmd)  
[View Network Module Diagram (SVG)](./09e_NETWORK_MODULE.svg)

---

## 3. Network Adapters

### 3.1. NetworkTelemetryServer / NetworkManager

**Responsibility:** Production telemetry transmission (HTTPS/mTLS), network connectivity management.

**Thread:** Network I/O Thread

**Interface:** `ITelemetryServer`

**Note:** `NetworkTelemetryServer` implements the `ITelemetryServer` interface. `NetworkManager` (if it exists as a separate service) would be a higher-level service that manages network connectivity and coordinates with `NetworkTelemetryServer`. Both would run on the Network I/O Thread. For detailed `NetworkManager` class design, see the legacy content in [09_CLASS_DESIGNS.md](./09_CLASS_DESIGNS.md) section 2.3.

**Key Methods:**
- `sendBatch(const TelemetryBatch& batch)`: Transmits telemetry batch to central server
  - **CRITICAL:** Automatically includes current patient MRN from `PatientManager`
  - Signs payload via `SignatureService`
  - Encrypts if needed via `EncryptionService`
  - Validates certificates via `CertificateManager`
  - Handles retries with exponential backoff
  - Records timing metrics to `telemetry_metrics` table
- `sendAlarm(const AlarmEvent& alarm)`: Transmits alarm event to central server
- `registerDevice(const DeviceSnapshot& device)`: Registers device with central server

**Dependencies:**
- `CertificateManager` - Certificate validation
- `EncryptionService` - Payload encryption
- `SignatureService` - Data signing
- `DatabaseManager` - Timing metrics persistence (via queued call)

**See:** [ITelemetryServer.md](./interfaces/ITelemetryServer.md) for interface definition.

---

### 3.2. MockTelemetryServer

**Responsibility:** Testing/development telemetry endpoint (swallows data without network).

**Thread:** Network I/O Thread

**Interface:** `ITelemetryServer`

**Key Methods:**
- Same interface as `NetworkTelemetryServer`
- No actual network transmission (in-memory only)

---

### 3.3. HISPatientLookupAdapter

**Responsibility:** Real HIS/EHR integration for patient lookup.

**Thread:** Network I/O Thread

**Interface:** `IPatientLookupService`

**Key Methods:**
- `lookupPatient(const QString& mrn)`: Looks up patient by MRN
- `searchByName(const QString& name)`: Searches patients by name

**Dependencies:**
- `QNetworkAccessManager` - HTTPS client
- External HIS/EHR server (HTTPS/REST API)

**See:** [IPatientLookupService.md](./interfaces/IPatientLookupService.md) for interface definition.

---

### 3.4. MockPatientLookupService

**Responsibility:** Testing/development patient lookup (hardcoded test patients).

**Thread:** Network I/O Thread

**Interface:** `IPatientLookupService`

**Key Methods:**
- Same interface as `HISPatientLookupAdapter`
- Returns hardcoded test patient data

---

### 3.5. CentralStationClient

**Responsibility:** Provisioning payload receiver from Central Station.

**Thread:** Network I/O Thread

**Key Methods:**
- `receiveProvisioningPayload(const QByteArray& encryptedPayload)`: Receives encrypted configuration payload
- `validateProvisioningSignature(const QByteArray& payload, const QByteArray& signature)`: Validates Central Station signature

**Dependencies:**
- `CertificateManager` - Certificate validation
- `EncryptionService` - Payload decryption

---

## 4. Security Adapters

### 4.1. CertificateManager

**Responsibility:** Certificate loading, validation, renewal.

**Thread:** Network I/O Thread

**Key Methods:**
- `loadCertificates()`: Loads client and CA certificates from secure storage
- `validateCertificates()`: Validates client certificate (expiration, revocation, device ID match)
- `checkCertificateRevocation()`: Checks certificate revocation list (CRL)
- `renewCertificate()`: Renews expiring certificate
- `installCertificate(const QSslCertificate& cert, const QSslKey& key)`: Installs new certificate

**Dependencies:**
- `SecureStorage` - Certificate storage (Background Module)
- `KeyManager` - Private key management (Background Module)

**See:** [15_CERTIFICATE_PROVISIONING.md](./15_CERTIFICATE_PROVISIONING.md) for certificate provisioning workflow.

---

### 4.2. EncryptionService

**Responsibility:** Payload encryption/decryption.

**Thread:** Network I/O Thread

**Key Methods:**
- `encrypt(const QByteArray& data, const QSslCertificate& recipientCert)`: Encrypts payload
- `decrypt(const QByteArray& encryptedData, const QSslKey& privateKey)`: Decrypts payload

**Technology:** OpenSSL via Qt (AES-256-GCM)

---

### 4.3. SignatureService

**Responsibility:** Data signing/verification.

**Thread:** Network I/O Thread

**Key Methods:**
- `sign(const QByteArray& data, const QSslKey& privateKey)`: Creates digital signature
- `verify(const QByteArray& data, const QByteArray& signature, const QSslCertificate& cert)`: Verifies signature

**Technology:** OpenSSL via Qt (ECDSA or RSA)

---

## 5. Domain Aggregates

### 5.1. DeviceAggregate

**Responsibility:** Device provisioning state, credential lifecycle.

**Thread:** Network I/O Thread

**Key Methods:**
- `applyProvisioningPayload(const ProvisioningConfig& config)`: Applies provisioning configuration
- `markProvisioned()`: Marks device as provisioned
- `rotateCredentials()`: Rotates device credentials

**Domain Events:**
- `ProvisioningCompleted`, `ProvisioningFailed`

---

## 6. Value Objects

### 6.1. DeviceSnapshot

**Responsibility:** Device identifier and status (immutable value object).

**Properties:**
- `deviceId`: `QString` - Device identifier
- `deviceLabel`: `QString` - Device label/asset tag
- `firmwareVersion`: `QString` - Firmware version
- `provisioningStatus`: `ProvisioningStatus` - Provisioning state

---

### 6.2. MeasurementUnit

**Responsibility:** Metric or Imperial unit preference (immutable value object).

**Properties:**
- `unit`: `QString` - "metric" or "imperial"

---

## 7. Module Communication

### 7.1. Inbound (From Other Modules)

**From Real-Time Processing Module (RT Thread):**
- `MPSC Queue` for telemetry batches (for transmission)

**From Application Services Module (App Services Thread):**
- `Qt::QueuedConnection` calls for patient lookup (HIS/EHR)
- `Qt::QueuedConnection` calls for user authentication (hospital server)

**From Interface Module (UI Thread):**
- `Qt::QueuedConnection` calls for connection testing

### 7.2. Outbound (To Other Modules)

**To Interface Module (UI Thread):**
- `Qt::QueuedConnection` signals for connection status changes

**To Database Module (Database I/O Thread):**
- `Qt::QueuedConnection` calls for timing metrics persistence

**To External Systems:**
- HTTPS/mTLS to Central Server
- HTTPS to HIS/EHR
- HTTPS to Central Station

---

## 8. Related Documents

- **[09_CLASS_DESIGNS_OVERVIEW.md](./09_CLASS_DESIGNS_OVERVIEW.md)** - High-level module architecture
- **[12_THREAD_MODEL.md](./12_THREAD_MODEL.md)** - Thread architecture (Section 4.5: Network I/O Thread)
- **[06_SECURITY.md](./06_SECURITY.md)** - Security architecture
- **[15_CERTIFICATE_PROVISIONING.md](./15_CERTIFICATE_PROVISIONING.md)** - Certificate provisioning workflow
- **[ITelemetryServer.md](./interfaces/ITelemetryServer.md)** - Telemetry server interface
- **[IPatientLookupService.md](./interfaces/IPatientLookupService.md)** - Patient lookup interface

---

*This document provides detailed class designs for the Network Module. For other modules, see the module-specific documents listed in [09_CLASS_DESIGNS_OVERVIEW.md](./09_CLASS_DESIGNS_OVERVIEW.md).*

