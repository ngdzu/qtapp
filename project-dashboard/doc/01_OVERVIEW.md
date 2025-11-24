# Project Overview: Z Monitor

## 1. Introduction

This document provides a high-level overview of the Z Monitor project. The Z Monitor is a modern, real-time patient monitoring system designed for an embedded touch-screen device.

The core purpose is to create a sophisticated, near-realistic application that demonstrates best practices in software architecture, UI/UX design, security, and system-level integration using Qt/C++ and QML.

## 2. Target Hardware

- **Device Type:** Embedded Medical Monitor
- **Screen:** 8-inch Touch Screen
- **Resolution:** 1280x800 pixels (fixed)

## 3. Core Features

### Patient Monitoring
- **Real-time Vitals:** Continuous display of critical patient data, including:
  - **Hemodynamics:** ECG waveform, Heart Rate (BPM), ST-Segment, PVCs.
  - **Respiratory:** Plethysmograph waveform, SpO2, Respiration Rate.
  - **Infusion:** TCI pump status, flow rate, volume, and drug name.
- **Historical Trends:** A dedicated view to plot vital signs over various time windows (1h, 8h, 24h), allowing for clinical analysis of patient history.

### Alarms & Notifications
- **Prioritized Alarms:** A multi-level alarm system (Critical, High, Medium, Low) to alert clinicians to adverse events.
- **Visual & Audible Alerts:** Includes screen-wide flashes for critical alarms, localized indicators, and distinct audible beep patterns.
- **Notification Center:** A non-intrusive system for informational messages and warnings, accessible via a bell icon.

### System & Security
- **Central Server Communication:** Securely transmits telemetry data to a simulated central monitoring station. The UI provides real-time feedback on the connection status (Online, Offline, Connecting).
- **User Authentication:** PIN-based login system with role-based access control (`Clinician`, `Technician`) to protect sensitive operations.
- **Data Security:** Implements end-to-end security with encrypted communication (mTLS) and encrypted local data storage (SQLCipher).

### Advanced Features
- **Predictive Analytics (Simulated):** A forward-looking feature that calculates and displays a risk score for conditions like Sepsis or Arrhythmia, providing suggestions for clinical intervention.
- **Internationalization:** The UI is designed to be fully translatable to support global use cases.

## 4. Technology Stack

- **UI Framework:** Qt 6 with QML for the front-end.
- **Backend Logic:** C++17 for core services, data management, and hardware simulation.
- **Build System:** CMake.
- **Database:** Encrypted SQLite via SQLCipher for local data persistence.
- **Networking:** Qt Networking (`QNetworkAccessManager`) for secure HTTPS communication.
- **Simulated Server:** A standalone Python (Flask/FastAPI) application.
