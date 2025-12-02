---
owner: TBD Team
reviewers:
  - Process stakeholders
last_reviewed: 2025-12-01
next_review: 2026-03-01
diagram_files:
  - diagrams/DOC-PROC-002_workflow.mmd
  - diagrams/DOC-PROC-002_workflow.svg
doc_id: DOC-PROC-002
title: Certificate Provisioning Process
version: 1.0
category: Process
subcategory: Security
status: Approved
created: 2025-11-27
updated: 2025-11-27
tags: [process, workflow, certificates, mtls, security, ca, provisioning]
related_docs:
  - DOC-PROC-001 # Device Provisioning Workflow
  - DOC-API-002 # ITelemetryServer
  - DOC-API-005 # IProvisioningService
authors:
  - Z Monitor Team
reviewers:
  - Security Team
---

# DOC-PROC-002: Certificate Provisioning Process

## 1. Overview

## 2. Process Flow

![Process Flowchart](diagrams/DOC-PROC-002_workflow.svg)

## 3. Steps

### 3.1 Step 1: {Step Name}

**Responsible:** {Role}

**Prerequisites:**
- {Prerequisite}

**Actions:**
1. {Action}

**Outputs:**
- {Output}

**Success Criteria:**
- {Criterion}


This document provides a comprehensive guide for certificate provisioning and device signing in the Z Monitor system. It covers the complete workflow from Certificate Authority (CA) creation to device certificate installation and validation.

**Purpose:**
The Z Monitor uses **Mutual TLS (mTLS)** for secure device-to-server communication. Each device must be provisioned with:

- **Client Certificate**: Unique certificate for the device, signed by the CA
- **Client Private Key**: Private key corresponding to the client certificate
- **CA Certificate**: Certificate Authority certificate for verifying the server's identity
- **Certificate Revocation List (CRL)**: Optional, list of revoked certificates

---

## 2. Certificate Provisioning Workflows

### 2.1 Initial Provisioning

1. **CA Setup:** Create Certificate Authority (if not exists)
2. **Device Certificate Generation:** Generate unique client certificate for device
3. **Certificate Signing:** Sign device certificate with CA private key
4. **Certificate Installation:** Install certificates on device (via QR code provisioning)
5. **Validation:** Verify certificates installed correctly
6. **Connection Test:** Test mTLS connection to central server

### 2.2 Certificate Renewal

1. **Expiry Check:** Monitor certificate expiry (90 days before expiration)
2. **Renewal Request:** Device requests certificate renewal from server
3. **Generate New Certificate:** CA generates new certificate with same device ID
4. **Push to Device:** Server pushes new certificate to device
5. **Install and Activate:** Device installs new certificate and activates
6. **Retire Old Certificate:** Old certificate retired after grace period (7 days)

### 2.3 Certificate Revocation

1. **Revocation Trigger:** Device compromised, stolen, or decommissioned
2. **Revoke Certificate:** CA adds certificate to CRL
3. **Distribute CRL:** CRL pushed to all servers and devices
4. **Block Connections:** Server blocks connections from revoked certificates
5. **Audit Log:** Log all revocation events for compliance

---

## 3. Certificate Authority (CA) Setup

### Step 1: Create CA Directory Structure

```bash
mkdir -p ca/{private,certs,crl,newcerts}
cd ca
chmod 700 private
touch index.txt
echo 1000 > serial
```

### Step 2: Generate CA Private Key

```bash
openssl genrsa -aes256 -out private/ca.key 4096
chmod 600 private/ca.key
```

**Parameters:**
- `-aes256`: Encrypts the private key with AES-256
- `4096`: Key size in bits (4096-bit RSA for strong security)

### Step 3: Create CA Certificate

```bash
openssl req -new -x509 -days 3650 \
    -key private/ca.key \
    -out certs/ca.crt \
    -subj "/C=US/ST=State/L=City/O=Hospital/OU=IT/CN=Z-Monitor-CA"
```

---

## 4. Device Certificate Generation

### Step 1: Generate Device Private Key

```bash
openssl genrsa -out device-ZM-001.key 2048
```

### Step 2: Create Certificate Signing Request (CSR)

```bash
openssl req -new \
    -key device-ZM-001.key \
    -out device-ZM-001.csr \
    -subj "/C=US/ST=State/L=City/O=Hospital/OU=ICU/CN=ZM-001"
```

### Step 3: Sign Certificate with CA

```bash
openssl ca -config ca.conf \
    -in device-ZM-001.csr \
    -out device-ZM-001.crt \
    -days 365
```

---

## 5. Certificate Installation on Device

Certificates are installed during device provisioning (see DOC-PROC-001):

1. **Central Station includes certificates in provisioning payload**
2. **Payload encrypted and signed**
3. **Device receives payload via HTTPS POST**
4. **Device validates signature and pairing code**
5. **Device installs certificates to encrypted database**
6. **Device tests mTLS connection to server**

---

## 6. Certificate Validation

### 6.1 Client-Side Validation (Device)

```cpp
// Verify server certificate against CA
QSslConfiguration sslConfig;
sslConfig.setCaCertificates({m_caCertificate});
sslConfig.setPeerVerifyMode(QSslSocket::VerifyPeer);

// Verify certificate not expired
if (m_clientCertificate.expiryDate() < QDate::currentDate()) {
    throw CertificateExpiredException();
}
```

### 6.2 Server-Side Validation (Central Server)

```python
# Verify client certificate signed by CA
ca_cert = load_certificate('ca.crt')
client_cert = load_certificate_from_request()

verify_certificate(client_cert, ca_cert)

# Check CRL
if certificate_is_revoked(client_cert, crl):
    reject_connection()
```

---

## 7. Certificate Lifecycle Management

### 7.1 Certificate Expiry Timeline

- **365 days:** Certificate valid period
- **90 days before expiry:** Warning notification
- **30 days before expiry:** Critical notification  
- **7 days before expiry:** Auto-renewal triggered
- **0 days:** Certificate expired, connection blocked

### 7.2 Renewal Process Automation

```cpp
// Check certificate expiry daily
void CertificateManager::checkCertificateExpiry() {
    int daysUntilExpiry = m_clientCertificate.expiryDate().daysTo(QDate::currentDate());
    
    if (daysUntilExpiry <= 7) {
        // Critical: auto-renew
        requestCertificateRenewal();
    } else if (daysUntilExpiry <= 30) {
        // Warning
        emit certificateExpiringWarning(daysUntilExpiry);
    } else if (daysUntilExpiry <= 90) {
        // Info
        emit certificateExpiringInfo(daysUntilExpiry);
    }
}
```

---

## 8. Security Considerations

### 8.1 Private Key Protection

- **Encryption:** Private keys encrypted at rest (AES-256)
- **Access Control:** Only system process can access private keys
- **No Export:** Private keys never exported from device
- **Audit Logging:** All key access logged

### 8.2 Certificate Pinning (Optional)

```cpp
// Pin specific server certificate
QSslConfiguration sslConfig;
sslConfig.setCaCertificates({m_pinnedServerCertificate});
sslConfig.setPeerVerifyMode(QSslSocket::VerifyPeer);
```

### 8.3 CRL Distribution

- **Push Mechanism:** CRL pushed to devices every 24 hours
- **Pull Mechanism:** Devices check CRL on connection
- **Offline Handling:** Cached CRL valid for 48 hours

---

## 9. Troubleshooting

### Common Issues

| Issue               | Symptom                                        | Solution                                  |
| ------------------- | ---------------------------------------------- | ----------------------------------------- |
| Certificate expired | Connection rejected with "Certificate expired" | Request certificate renewal               |
| CA mismatch         | "Certificate not signed by trusted CA"         | Verify CA certificate installed correctly |
| Revoked certificate | "Certificate revoked"                          | Provision new certificate                 |
| Time sync issue     | "Certificate not yet valid"                    | Sync device clock with NTP server         |

---

## 10. Related Documents

- **DOC-PROC-001:** Device Provisioning Workflow - Includes certificate installation
- **DOC-API-002:** ITelemetryServer - mTLS configuration
- **DOC-API-005:** IProvisioningService - Certificate delivery mechanism

---

## 11. Changelog

| Version | Date       | Author         | Changes                                      |
| ------- | ---------- | -------------- | -------------------------------------------- |
| 1.0     | 2025-11-27 | Z Monitor Team | Migrated from 15_CERTIFICATE_PROVISIONING.md |

---

*This process ensures secure device-to-server communication through comprehensive certificate lifecycle management.*
