# Developer Setup Guide

This document provides instructions for setting up the development environment, building the Medical Device Dashboard application, and running its components.

## 1. Prerequisites

Before you begin, ensure you have the following installed:

-   **Docker & Docker Compose:** For containerized development and running the simulated server.
-   **CMake (3.16+):** Build system for the C++ Qt application.
-   **Qt 6 (6.2+):** Development libraries for C++ and QML.
-   **Python 3.8+:** For the simulated central server.
-   **Git:** Version control.

## 2. Project Structure

```
.
├── project-dashboard/
│   ├── CMakeLists.txt       # Build configuration for Qt app
│   ├── Dockerfile           # Multi-stage build for Qt app
│   ├── README.md            # Project overview
│   ├── src/                 # C++ source code and QML controllers
│   ├── resources/           # QML files, assets, i18n, certs
│   └── tests/               # Unit tests
└── central-server-simulator/
    ├── app.py               # Simple Flask/FastAPI server
    ├── README.md            # Server specific instructions
    └── certs/               # Server certificate and trusted CA
```

## 3. Setting up the Development Environment

### 3.1. Clone the Repository

```bash
git clone <repository_url>
cd qtapp/project-dashboard
```

### 3.2. Docker-based Development (Recommended)

The project is designed for containerized development using `devcontainer.json` (if available) or by manually building the Docker images.

1.  **Build Development Environment Image:**
    ```bash
    docker build -t qtapp-qt-dev-env -f Dockerfile.dev .
    ```
    (Note: The actual `Dockerfile.dev` might be in the root `qtapp` directory or specified in `.devcontainer/devcontainer.json`. Adjust path as necessary.)

2.  **Run Development Container:**
    If using VS Code with Remote-Containers extension, simply "Reopen in Container".
    Otherwise, you can run it manually:
    ```bash
    docker run -it --rm -v $(pwd):/app -w /app qtapp-qt-dev-env bash
    ```
    This will give you a shell inside the container with all Qt dependencies installed.

## 4. Building the Medical Device Dashboard Application

These steps are performed *inside* the development container or a system with Qt 6 installed.

1.  **Create Build Directory:**
    ```bash
    mkdir build
    cd build
    ```

2.  **Configure CMake:**
    ```bash
    cmake ..
    ```
    (Ensure `qmake` or `qtchooser` points to Qt 6)

3.  **Build the Application:**
    ```bash
    cmake --build .
    ```

4.  **Run the Application:**
    ```bash
    ./project-dashboard
    ```
    (The executable name might vary based on `CMakeLists.txt`)

## 5. Setting up the Simulated Central Server

The simulated server is a Python application.

1.  **Navigate to Server Directory:**
    ```bash
    cd ../central-server-simulator
    ```

2.  **Create Virtual Environment (Optional but Recommended):**
    ```bash
    python3 -m venv venv
    source venv/bin/activate
    ```

3.  **Install Dependencies:**
    ```bash
    pip install -r requirements.txt # (You might need to create this file first, e.g., Flask, Gunicorn, cryptography)
    ```

4.  **Generate Certificates (for mTLS):**
    For development, you'll need to generate self-signed certificates. A script for this should be provided (e.g., `generate_certs.sh` in `central-server-simulator/certs/`).
    ```bash
    ./certs/generate_certs.sh
    ```
    This script should generate:
    -   `ca.crt` (Certificate Authority)
    -   `server.crt`, `server.key` (for the server)
    -   `client.crt`, `client.key` (for the device)

5.  **Run the Server:**
    ```bash
    python3 app.py
    ```
    (Ensure `app.py` is configured to use the generated certificates for HTTPS and mTLS.)

## 6. Running the Complete System

1.  Start the simulated central server (as described in Section 5).
2.  Run the Medical Device Dashboard application (as described in Section 4).
3.  Ensure the device application is configured to connect to the correct server address and port, and that it can access its client certificates.

## 7. Database Encryption (SQLCipher)

-   **Integration:** Ensure your Qt build includes SQLCipher support. This might require building Qt from source with specific flags or using a pre-built Qt version that includes it.
-   **Key Management:** The `DatabaseManager` in `src/core/DatabaseManager.cpp` will need to be initialized with the encryption key. For development, this key can be hardcoded or read from a secure local configuration.
