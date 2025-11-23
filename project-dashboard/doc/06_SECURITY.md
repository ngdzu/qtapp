# Security Design

This document details the security architecture for the Medical Device Dashboard, covering data in transit, data at rest, and authentication.

## 1. Guiding Principles

-   **Defense in Depth:** Employ multiple layers of security controls.
-   **Principle of Least Privilege:** Components and users should only have the access required to perform their functions.
-   **Secure by Default:** The default configuration of the system must be secure.

## 2. Encryption in Transit

-   **Requirement:** All communication between the device and the central server must be encrypted.
-   **Implementation:**
    -   **Protocol:** TLS 1.2 or higher.
    -   **Endpoint:** All API endpoints must be accessed via `https://`.
    -   **C++ Client:** The `NetworkManager` class will use `QNetworkAccessManager` and a properly configured `QSslConfiguration` to enforce TLS.
    -   **Server:** The simulated central server (Python/Flask) will be configured to only serve content over HTTPS.

## 3. Authentication

### 3.1. Device-to-Server Authentication

-   **Requirement:** The device and server must authenticate each other to prevent unauthorized devices from connecting and to ensure the device is connecting to a legitimate server.
-   **Implementation: Mutual TLS (mTLS)**
    -   **Device Side:** Each device will be provisioned with a unique client certificate (`.pem`) and a corresponding private key (`.key`). The `NetworkManager` will load these into its `QSslConfiguration`. The device will also have a copy of the CA certificate to verify the server's identity.
    -   **Server Side:** The server will have its own server certificate and private key. It will also be configured with the CA certificate and will be set to require and verify client certificates from incoming connections. Any connection without a valid, trusted client certificate will be rejected at the TLS handshake level.

### 3.2. User-to-Device Authentication

-   **Requirement:** Only authorized clinicians and technicians can access the device's functions.
-   **Implementation: PIN-based Login & Roles**
    -   **Login Screen:** A PIN entry screen is presented on startup.
    -   **Roles:**
        -   **Clinician:** Can access monitoring, alarm management, and basic settings.
        -   **Technician:** Has all Clinician privileges, plus access to advanced settings like device calibration and diagnostics.
    -   **Backend:** The `AuthenticationService` in C++ is responsible for verifying the PIN and managing the active user's role and session.

## 4. Encryption at Rest

-   **Requirement:** All sensitive patient data stored locally on the device must be encrypted.
-   **Implementation: Encrypted SQLite Database**
    -   **Technology:** The project will use **SQLCipher**, a widely-used open-source extension to SQLite that provides transparent 256-bit AES encryption of database files.
    -   **Backend:** The `DatabaseManager` class will be responsible for managing the connection to the encrypted database, including handling the encryption key.
    -   **Key Management:** The encryption key for the database must be stored securely, for instance, in a hardware-backed keystore if available on the target device. For simulation, it can be a hardcoded value or stored in a configuration file with restricted permissions.

## 5. Certificate & Key Provisioning

-   **Requirement:** A secure process for installing and managing cryptographic materials on the device.
-   **Process (Simulated):**
    1.  A Certificate Authority (CA) is created (self-signed for this project).
    2.  The CA is used to sign a unique certificate for the server and a unique certificate for each device.
    3.  **Device:** The device's client certificate, its private key, and the CA certificate are securely installed in a protected area of the device's filesystem during "manufacturing".
    4.  **Server:** The server's certificate, its private key, and the CA certificate are installed on the server.
