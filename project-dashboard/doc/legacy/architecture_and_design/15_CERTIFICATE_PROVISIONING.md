# Certificate Provisioning Guide

**Document ID:** DESIGN-015  
**Version:** 1.0  
**Status:** Approved  
**Last Updated:** 2025-11-27

---

This document provides a comprehensive, step-by-step guide for certificate provisioning and device signing in the Z Monitor system. It covers the complete workflow from Certificate Authority (CA) creation to device certificate installation and validation.

## Table of Contents

1. [Overview](#overview)
2. [Prerequisites](#prerequisites)
3. [Certificate Authority (CA) Setup](#certificate-authority-ca-setup)
4. [Device Certificate Generation](#device-certificate-generation)
5. [Certificate Installation on Device](#certificate-installation-on-device)
6. [Certificate Validation and Verification](#certificate-validation-and-verification)
7. [Certificate Renewal Process](#certificate-renewal-process)
8. [Certificate Revocation Process](#certificate-revocation-process)
9. [Workflow Diagrams](#workflow-diagrams)
10. [Troubleshooting](#troubleshooting)

## Overview

The Z Monitor uses **Mutual TLS (mTLS)** for secure device-to-server communication. Each device must be provisioned with:

- **Client Certificate**: Unique certificate for the device, signed by the CA
- **Client Private Key**: Private key corresponding to the client certificate
- **CA Certificate**: Certificate Authority certificate for verifying the server's identity
- **Certificate Revocation List (CRL)**: Optional, list of revoked certificates

The provisioning process ensures:
- Each device has a unique identity
- Certificates are cryptographically bound to device IDs
- Certificates can be validated, renewed, and revoked
- Secure storage and handling of private keys

## Prerequisites

Before beginning certificate provisioning, ensure you have:

- **OpenSSL** (version 1.1.1 or later) installed
- **Certificate Authority (CA)** established or access to existing CA
- **Device Information**:
  - Device ID (e.g., "ZM-001")
  - Device serial number
  - Organization information (for certificate subject)
- **Secure Storage**: Protected location for CA private key and device certificates
- **Access Permissions**: Appropriate file system permissions (600 for private keys, 644 for certificates)

## Certificate Authority (CA) Setup

### Step 1: Create CA Directory Structure

```bash
mkdir -p ca/{private,certs,crl,newcerts}
cd ca
chmod 700 private
touch index.txt
echo 1000 > serial
```

**Directory Structure:**
- `private/`: Stores CA private key (restricted access)
- `certs/`: Stores issued certificates
- `crl/`: Certificate Revocation Lists
- `newcerts/`: Archive of issued certificates
- `index.txt`: Database of issued certificates
- `serial`: Serial number counter

### Step 2: Generate CA Private Key

```bash
openssl genrsa -aes256 -out private/ca.key 4096
chmod 600 private/ca.key
```

**Parameters:**
- `-aes256`: Encrypts the private key with AES-256
- `-out private/ca.key`: Output file path
- `4096`: Key size in bits (4096-bit RSA for strong security)

**Security Note:** The CA private key must be kept extremely secure. If compromised, all certificates issued by this CA become untrustworthy.

### Step 3: Create CA Certificate

```bash
openssl req -new -x509 -days 3650 \
    -key private/ca.key \
    -out certs/ca.crt \
    -subj "/C=US/ST=State/L=City/O=Hospital/OU=IT/CN=Z-Monitor-CA" \
    -extensions v3_ca \
    -config <(cat <<EOF
[req]
distinguished_name = req_distinguished_name
x509_extensions = v3_ca

[v3_ca]
basicConstraints = critical,CA:TRUE
keyUsage = critical,keyCertSign,cRLSign
subjectKeyIdentifier = hash
authorityKeyIdentifier = keyid:always,issuer:always

[req_distinguished_name]
EOF
)
```

**Parameters:**
- `-new -x509`: Creates a self-signed certificate
- `-days 3650`: Certificate validity (10 years)
- `-subj`: Certificate subject (customize with your organization details)
- `-extensions v3_ca`: Applies CA extensions

**Subject Fields:**
- `C`: Country (2-letter code)
- `ST`: State/Province
- `L`: Locality/City
- `O`: Organization
- `OU`: Organizational Unit
- `CN`: Common Name (CA identifier)

### Step 4: Verify CA Certificate

```bash
openssl x509 -in certs/ca.crt -text -noout
```

Verify:
- Certificate is valid
- Subject matches your organization
- Key usage includes `keyCertSign` and `cRLSign`
- Basic constraints indicate `CA:TRUE`

## Device Certificate Generation

### Step 1: Prepare Device Information

For each device, collect:
- **Device ID**: Unique identifier (e.g., "ZM-001")
- **Device Serial Number**: Hardware serial number
- **Device Type**: "Z-Monitor"
- **Organization Details**: Same as CA or device-specific

### Step 2: Generate Device Private Key

```bash
DEVICE_ID="ZM-001"
openssl genrsa -aes256 -out device_${DEVICE_ID}.key 2048
chmod 600 device_${DEVICE_ID}.key
```

**Parameters:**
- `-aes256`: Encrypts private key (password-protected)
- `2048`: Key size (2048-bit RSA sufficient for device certificates)
- **Security**: Store encrypted key securely; password should be strong and unique per device

### Step 3: Create Certificate Signing Request (CSR)

```bash
openssl req -new \
    -key device_${DEVICE_ID}.key \
    -out device_${DEVICE_ID}.csr \
    -subj "/C=US/ST=State/L=City/O=Hospital/OU=Medical-Devices/CN=Z-Monitor-${DEVICE_ID}" \
    -config <(cat <<EOF
[req]
distinguished_name = req_distinguished_name
req_extensions = v3_req

[v3_req]
basicConstraints = CA:FALSE
keyUsage = digitalSignature,keyEncipherment
extendedKeyUsage = clientAuth
subjectAltName = @alt_names

[alt_names]
DNS.1 = device-${DEVICE_ID}
otherName.1 = 1.3.6.1.4.1.12345.1.1;UTF8:${DEVICE_ID}

[req_distinguished_name]
EOF
)
```

**Key Points:**
- **Subject Alternative Name (SAN)**: Includes device ID in `otherName` field for device binding
- **Extended Key Usage**: `clientAuth` indicates this is a client certificate for mTLS
- **Key Usage**: `digitalSignature` and `keyEncipherment` for TLS operations

### Step 4: Sign Device Certificate with CA

```bash
openssl ca -config <(cat <<EOF
[ca]
default_ca = CA_default

[CA_default]
dir = .
certs = certs
new_certs_dir = newcerts
database = index.txt
serial = serial
RANDFILE = private/.rand
private_key = private/ca.key
certificate = certs/ca.crt
default_days = 365
default_md = sha256
policy = policy_match
x509_extensions = v3_device

[policy_match]
countryName = match
stateOrProvinceName = optional
localityName = optional
organizationName = match
organizationalUnitName = optional
commonName = supplied

[v3_device]
basicConstraints = CA:FALSE
keyUsage = digitalSignature,keyEncipherment
extendedKeyUsage = clientAuth
subjectAltName = @alt_names
authorityKeyIdentifier = keyid:always,issuer:always

[alt_names]
DNS.1 = device-${DEVICE_ID}
otherName.1 = 1.3.6.1.4.1.12345.1.1;UTF8:${DEVICE_ID}
EOF
) \
    -in device_${DEVICE_ID}.csr \
    -out device_${DEVICE_ID}.crt \
    -days 365 \
    -extensions v3_device
```

**Parameters:**
- `-days 365`: Certificate validity (1 year, adjust as needed)
- `-extensions v3_device`: Applies device-specific extensions
- Certificate is signed by CA and stored in `newcerts/` directory

### Step 5: Verify Device Certificate

```bash
# Verify certificate chain
openssl verify -CAfile certs/ca.crt device_${DEVICE_ID}.crt

# View certificate details
openssl x509 -in device_${DEVICE_ID}.crt -text -noout

# Extract device ID from certificate
openssl x509 -in device_${DEVICE_ID}.crt -noout -text | grep -A1 "otherName"
```

**Verification Checklist:**
- ✅ Certificate is valid and not expired
- ✅ Certificate is signed by CA (chain verification passes)
- ✅ Device ID is present in Subject Alternative Name
- ✅ Extended Key Usage includes `clientAuth`
- ✅ Serial number is unique

### Step 6: Export Certificate Bundle

For device installation, create a bundle:

```bash
# Create device certificate bundle
cat device_${DEVICE_ID}.crt certs/ca.crt > device_${DEVICE_ID}_bundle.pem

# Or create PKCS#12 format (optional, for some systems)
openssl pkcs12 -export \
    -out device_${DEVICE_ID}.p12 \
    -inkey device_${DEVICE_ID}.key \
    -in device_${DEVICE_ID}.crt \
    -certfile certs/ca.crt \
    -name "Z-Monitor-${DEVICE_ID}" \
    -passout pass:${DEVICE_PASSWORD}
```

**Files for Device:**
- `device_${DEVICE_ID}.crt`: Device certificate
- `device_${DEVICE_ID}.key`: Device private key (encrypted)
- `ca.crt`: CA certificate (for server verification)
- `device_${DEVICE_ID}_bundle.pem`: Certificate bundle (optional)

## Certificate Installation on Device

### Step 1: Prepare Device Filesystem

```bash
# On device, create secure certificate directory
mkdir -p /opt/z-monitor/certs
chmod 700 /opt/z-monitor/certs
```

**Security Requirements:**
- Directory permissions: 700 (owner read/write/execute only)
- Files should be owned by device application user
- Private keys: 600 permissions
- Certificates: 644 permissions

### Step 2: Transfer Certificates to Device

**Secure Transfer Methods:**

**Option A: Secure Copy (SCP)**
```bash
scp device_${DEVICE_ID}.crt device_${DEVICE_ID}.key ca.crt user@device-ip:/opt/z-monitor/certs/
```

**Option B: USB/Secure Media**
- Copy files to encrypted USB drive
- Physically transfer to device
- Copy files to device filesystem
- Securely erase USB drive

**Option C: Provisioning Service**
- Use secure provisioning API
- Certificates encrypted in transit
- Authenticated device registration

### Step 3: Install Certificates

```bash
# On device
cd /opt/z-monitor/certs

# Set proper permissions
chmod 644 device_${DEVICE_ID}.crt
chmod 600 device_${DEVICE_ID}.key
chmod 644 ca.crt

# Verify file integrity
sha256sum device_${DEVICE_ID}.crt device_${DEVICE_ID}.key ca.crt > certs.sha256
```

### Step 4: Update Device Configuration

Update device settings to reference certificates:

```bash
# In device configuration or settings database
deviceId = "${DEVICE_ID}"
certificatePath = "/opt/z-monitor/certs/device_${DEVICE_ID}.crt"
privateKeyPath = "/opt/z-monitor/certs/device_${DEVICE_ID}.key"
caCertificatePath = "/opt/z-monitor/certs/ca.crt"
```

### Step 5: Register Certificate in Database

The device should register its certificate in the local database:

```sql
INSERT INTO certificates (
    device_id,
    cert_serial,
    cert_subject,
    cert_issuer,
    issued_at,
    expires_at,
    status,
    cert_fingerprint
) VALUES (
    '${DEVICE_ID}',
    '$(openssl x509 -in device_${DEVICE_ID}.crt -noout -serial | cut -d= -f2)',
    '$(openssl x509 -in device_${DEVICE_ID}.crt -noout -subject)',
    '$(openssl x509 -in device_${DEVICE_ID}.crt -noout -issuer)',
    $(date +%s),
    $(openssl x509 -in device_${DEVICE_ID}.crt -noout -enddate | cut -d= -f2 | xargs -I {} date -d {} +%s),
    'active',
    '$(openssl x509 -in device_${DEVICE_ID}.crt -noout -fingerprint -sha256 | cut -d= -f2 | tr -d :)'
);
```

## Certificate Validation and Verification

### Step 1: On-Device Validation

The device should validate its certificate on startup:

```cpp
// Pseudo-code for certificate validation
bool ValidateDeviceCertificate() {
    // 1. Load certificate
    QSslCertificate cert = LoadCertificate("device_${DEVICE_ID}.crt");
    
    // 2. Check expiration
    if (cert.expiryDate() < QDateTime::currentDateTime()) {
        LogError("Certificate expired");
        return false;
    }
    
    // 3. Verify CA signature
    if (!cert.verify({caCertificate})) {
        LogError("Certificate not signed by trusted CA");
        return false;
    }
    
    // 4. Extract and verify device ID
    QString deviceIdFromCert = ExtractDeviceIdFromCertificate(cert);
    QString deviceIdFromSettings = settingsManager->GetValue("deviceId");
    if (deviceIdFromCert != deviceIdFromSettings) {
        LogError("Certificate device ID mismatch");
        return false;
    }
    
    // 5. Check revocation (if CRL available)
    if (IsCertificateRevoked(cert.serialNumber())) {
        LogError("Certificate revoked");
        return false;
    }
    
    // 6. Update last validation timestamp
    databaseManager->UpdateCertificateValidation(cert.serialNumber());
    
    return true;
}
```

### Step 2: Server-Side Validation

The server validates device certificates during mTLS handshake:

1. **TLS Handshake**: Server requests client certificate
2. **Certificate Chain Verification**: Server verifies certificate is signed by trusted CA
3. **Device ID Extraction**: Server extracts device ID from certificate SAN
4. **Device Registration Check**: Server verifies device is registered
5. **Revocation Check**: Server checks CRL if available
6. **Connection Authorization**: Server authorizes connection if all checks pass

## Certificate Renewal Process

Certificates should be renewed before expiration (recommended: 30 days before).

### Step 1: Generate New Certificate

Follow [Device Certificate Generation](#device-certificate-generation) steps to create a new certificate.

### Step 2: Install New Certificate (Parallel Installation)

```bash
# Install new certificate alongside old one
cp device_${DEVICE_ID}_new.crt /opt/z-monitor/certs/device_${DEVICE_ID}_new.crt
cp device_${DEVICE_ID}_new.key /opt/z-monitor/certs/device_${DEVICE_ID}_new.key

# Update configuration to use new certificate
# Old certificate remains valid until expiration
```

### Step 3: Switch to New Certificate

```bash
# After verifying new certificate works
mv device_${DEVICE_ID}_new.crt device_${DEVICE_ID}.crt
mv device_${DEVICE_ID}_new.key device_${DEVICE_ID}.key

# Restart device service to load new certificate
systemctl restart z-monitor
```

### Step 4: Update Database

```sql
-- Mark old certificate as expired
UPDATE certificates 
SET status = 'expired' 
WHERE device_id = '${DEVICE_ID}' AND status = 'active';

-- Register new certificate
INSERT INTO certificates (...) VALUES (...);
```

## Certificate Revocation Process

If a device is compromised or needs to be decommissioned:

### Step 1: Revoke Certificate

```bash
# On CA server
openssl ca -revoke newcerts/$(openssl x509 -in device_${DEVICE_ID}.crt -noout -serial | cut -d= -f2).pem \
    -crl_reason superseded \
    -config ca.conf
```

**Revocation Reasons:**
- `unspecified`: General revocation
- `keyCompromise`: Private key compromised
- `superseded`: Replaced by new certificate
- `cessationOfOperation`: Device decommissioned

### Step 2: Generate Updated CRL

```bash
openssl ca -gencrl \
    -out crl/crl.pem \
    -config ca.conf
```

### Step 3: Distribute CRL

```bash
# Copy CRL to devices and server
scp crl/crl.pem user@device:/opt/z-monitor/certs/
scp crl/crl.pem user@server:/etc/ssl/certs/
```

### Step 4: Update Device Status

```sql
UPDATE certificates 
SET status = 'revoked', 
    revoked_at = $(date +%s),
    revocation_reason = 'keyCompromise'
WHERE device_id = '${DEVICE_ID}';
```

## Workflow Diagrams

Detailed sequence diagrams are available for each workflow. Both Mermaid source (`.mmd`) and rendered SVG (`.svg`) files are provided:

1. **[Initial Certificate Provisioning](./15_CERTIFICATE_PROVISIONING_INITIAL.svg)** ([Mermaid source](./15_CERTIFICATE_PROVISIONING_INITIAL.mmd)): Complete flow from CA setup to device installation
   - Phase 1: CA Setup (steps 1-4)
   - Phase 2: Device Certificate Generation (steps 5-11)
   - Phase 3: Certificate Installation (steps 12-17)
   - Phase 4: Certificate Validation (steps 18-34)

2. **[Certificate Renewal Workflow](./15_CERTIFICATE_PROVISIONING_RENEWAL.svg)** ([Mermaid source](./15_CERTIFICATE_PROVISIONING_RENEWAL.mmd)): Zero-downtime renewal process
   - Phase 1: Renewal Preparation (monitoring, alerting, new cert generation)
   - Phase 2: Parallel Installation (install new cert alongside old)
   - Phase 3: Seamless Switchover (activate new cert, deactivate old)

3. **[Certificate Revocation Workflow](./15_CERTIFICATE_PROVISIONING_REVOCATION.svg)** ([Mermaid source](./15_CERTIFICATE_PROVISIONING_REVOCATION.mmd)): Revocation and CRL distribution
   - Phase 1: Revocation Decision (identify device, revoke cert, generate CRL)
   - Phase 2: CRL Distribution (distribute to server and devices)
   - Phase 3: Revocation Enforcement (server rejects revoked certificates)

4. **[Certificate Validation on Device Startup](./15_CERTIFICATE_PROVISIONING_VALIDATION.svg)** ([Mermaid source](./15_CERTIFICATE_PROVISIONING_VALIDATION.mmd)): Detailed validation process
   - Certificate loading and expiration checking
   - CA signature verification
   - Device ID extraction and matching
   - CRL checking
   - Database updates

5. **[Secure Data Transmission](./15_CERTIFICATE_PROVISIONING_TRANSMISSION.svg)** ([Mermaid source](./15_CERTIFICATE_PROVISIONING_TRANSMISSION.mmd)): End-to-end secure transmission flow
   - Certificate validation before transmission
   - Digital signature creation
   - mTLS handshake with server
   - Server-side certificate validation
   - Payload signature verification
   - Replay prevention

**Note:** SVG files are auto-generated from Mermaid source files. To regenerate SVGs after editing MMD files:
```bash
npx @mermaid-js/mermaid-cli -i <path/to/diagram>.mmd -o <path/to/diagram>.svg
```

## Troubleshooting

### Common Issues

**Issue: Certificate validation fails**
- Check certificate expiration: `openssl x509 -in cert.crt -noout -dates`
- Verify CA signature: `openssl verify -CAfile ca.crt device.crt`
- Check device ID match: Extract device ID from certificate SAN

**Issue: mTLS handshake fails**
- Verify client certificate is loaded: Check `QSslConfiguration`
- Check server requires client certificate: Server must request client cert
- Verify CA certificate on both sides: Device and server must trust same CA

**Issue: Certificate not found**
- Check file paths in configuration
- Verify file permissions (600 for keys, 644 for certs)
- Check file ownership matches application user

**Issue: Private key password incorrect**
- Verify key encryption password
- Consider using unencrypted keys in secure storage (HSM) for production

### Security Best Practices

1. **CA Private Key Protection**:
   - Store in hardware security module (HSM) if available
   - Use strong encryption (AES-256)
   - Limit access to authorized personnel only
   - Regular backups in secure location

2. **Device Private Key Protection**:
   - Encrypt at rest (AES-256)
   - Use secure storage (HSM, TPM, or encrypted filesystem)
   - Clear from memory after use
   - Never transmit unencrypted

3. **Certificate Lifecycle Management**:
   - Monitor expiration dates (alert 30 days before)
   - Automate renewal where possible
   - Maintain certificate inventory
   - Regular CRL updates

4. **Audit and Compliance**:
   - Log all certificate operations
   - Track certificate lifecycle in database
   - Maintain security audit logs
   - Regular security reviews

## Appendix: OpenSSL Configuration File

For production use, create a proper OpenSSL configuration file:

```ini
[ca]
default_ca = CA_default

[CA_default]
dir = ./ca
certs = $dir/certs
new_certs_dir = $dir/newcerts
database = $dir/index.txt
serial = $dir/serial
RANDFILE = $dir/private/.rand
private_key = $dir/private/ca.key
certificate = $dir/certs/ca.crt
default_days = 365
default_md = sha256
policy = policy_match
x509_extensions = v3_device

[policy_match]
countryName = match
stateOrProvinceName = optional
localityName = optional
organizationName = match
organizationalUnitName = optional
commonName = supplied

[v3_device]
basicConstraints = CA:FALSE
keyUsage = digitalSignature,keyEncipherment
extendedKeyUsage = clientAuth
subjectAltName = @alt_names
authorityKeyIdentifier = keyid:always,issuer:always

[alt_names]
# Device-specific SAN will be added during certificate generation
```

## References

- [OpenSSL Documentation](https://www.openssl.org/docs/)
- [RFC 5280: X.509 Certificate Profile](https://tools.ietf.org/html/rfc5280)
- [TLS 1.3 Specification](https://tools.ietf.org/html/rfc8446)
- [Z Monitor Security Design](./06_SECURITY.md)

