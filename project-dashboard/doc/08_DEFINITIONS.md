# Definitions and Terminologies

This document provides a glossary of key terms and terminologies used within the Z Monitor project.

## Z

*   **Z Monitor:** The primary medical device application that reads data from sensors and attached devices, displays real-time patient vital signs, manages alarms, and securely transmits telemetry data to a central server. The Z Monitor is a patient monitoring system designed for embedded touch-screen devices, providing continuous monitoring of hemodynamic, respiratory, and infusion pump data. It features a modern QML-based UI, encrypted local data storage, secure mTLS communication, and comprehensive alarm management.

## A

*   **AlarmManager:** A C++ backend class responsible for monitoring physiological and technical data, detecting alarm conditions, and managing the state and history of alarms.
*   **AuthenticationService:** A C++ backend class handling user login, session management, and role-based access control for the device.

## A

*   **ADT (Admission, Discharge, Transfer):** Hospital workflow for managing patient assignments to devices. The Z Monitor implements ADT workflow where patients are admitted to devices, creating an association between device and patient. See `doc/19_ADT_WORKFLOW.md` for complete workflow documentation.
*   **Admission Modal:** A specialized UI component for admitting patients to the device. Supports three admission methods: Manual Entry (type MRN/name), Barcode Scan (scan patient wristband), and Central Station Push (automatic admission from HIS/EHR).

## B

*   **Bed Location:** Current bed/room assignment for a patient (e.g., "ICU-4B"). Part of the Patient object and managed through the ADT workflow. Previously called "Bed ID" but now separated from device configuration.

## C

*   **CA (Certificate Authority):** A trusted entity that issues digital certificates. In mTLS, the CA signs both server and client certificates.
*   **CMake:** An open-source, cross-platform family of tools designed to build, test and package software. Used as the build system for the Qt application.
*   **Critical Alarm (P1):** The highest priority alarm, indicating an immediate threat to patient life or device integrity, requiring urgent intervention.

## D

*   **DashboardController:** A C++ QObject-based controller that exposes real-time vital signs and device status data to the QML frontend for display on the main dashboard view.
*   **DatabaseManager:** A C++ backend class responsible for managing the local encrypted SQLite database, including data storage, retrieval, and encryption.
*   **DataArchiver:** A C++ backend class responsible for handling the archival of historical data from the local database, typically data older than a defined retention policy.
*   **Device ID:** A unique identifier assigned to each Z Monitor device, used for device identification in telemetry transmission and tracking in the central monitoring system. Configurable through the Settings View.
*   **Device Label:** A static device identifier/asset tag (e.g., "ICU-MON-04") that serves as the fixed technical identifier for the device itself. Separate from patient assignment and rarely changes. Displayed in Settings View (read-only for most users, editable by Technician role).
*   **DeviceSimulator:** A C++ backend class that generates realistic, simulated patient vital signs and device operational data for testing and demonstration purposes.
*   **Diagnostics View:** A QML view displaying system logs and providing controls for simulation parameters or remote commands.
*   **Docker:** A platform that uses OS-level virtualization to deliver software in packages called containers. Used for consistent development and deployment environments.

## E

*   **ECG (Electrocardiogram):** A measurement of the electrical activity of the heart. Displayed as a waveform on the dashboard.
*   **Encryption at Rest:** The practice of encrypting data when it is stored on a physical medium (e.g., the device's local storage).
*   **Encryption in Transit:** The practice of encrypting data as it is being transmitted over a network.

## H

*   **High Alarm (P2):** A high priority alarm indicating a potential threat to patient life or device integrity if not addressed promptly.
*   **HTTPS:** Hypertext Transfer Protocol Secure. An extension of HTTP that uses TLS to encrypt communication over a computer network.

## I

*   **i18n (Internationalization):** The process of designing a software application so that it can be adapted to various languages and regions without engineering changes.
*   **Infusion Pump (TCI):** Target Controlled Infusion pump. A medical device that delivers fluids, medication, or nutrients into a patient's circulatory system.
*   **IoT (Internet of Things):** A network of physical objects embedded with sensors, software, and other technologies for the purpose of connecting and exchanging data with other devices and systems over the internet.

## L

*   **LogService:** A C++ backend class providing a centralized mechanism for generating and managing application log messages.
*   **Low Alarm (P4):** The lowest priority alarm, typically informational or advisory, requiring no direct intervention.

## M

*   **Measurement Unit:** System preference for displaying measurements in either metric or imperial units. Affects the display of vital signs, infusion rates, and other measured values throughout the application. Configurable through the Settings View. Options include "metric" (e.g., Celsius, milliliters) or "imperial" (e.g., Fahrenheit, fluid ounces).
*   **Medium Alarm (P3):** A medium priority alarm requiring awareness and follow-up, but not immediate intervention.
*   **mTLS (Mutual TLS):** Mutual Transport Layer Security. A two-way authentication process where both the client (device) and the server authenticate each other using digital certificates.

## N

*   **NetworkManager:** A C++ backend class responsible for managing network connectivity, secure data transmission (mTLS) to the central server, and handling connection status. Integrates with `ITelemetryServer` interface for server communication and supports configurable server URLs. Implements comprehensive security measures including certificate validation, digital signatures, rate limiting, and security audit logging.
*   **ITelemetryServer:** An interface for sending telemetry data and sensor data to a central monitoring server. Supports multiple implementations: `NetworkTelemetryServer` (production), `MockTelemetryServer` (testing/development), and `FileTelemetryServer` (offline testing).
*   **MockTelemetryServer:** A test implementation of `ITelemetryServer` that swallows all data without sending to a real server. Used for unit testing and development when server infrastructure is not available.
*   **NotificationController:** A C++ QObject-based controller that exposes non-critical informational and warning messages to the QML notification system.
*   **Notification System:** A UI component (bell icon with badge) for displaying non-critical informational messages and warnings.

## P

*   **Pairing Code:** A time-limited, one-time-use code (format: XXX-XXX-XXX) used for device provisioning. Generated cryptographically and expires after 10 minutes. See `doc/17_DEVICE_PROVISIONING.md` for details.
*   **Patient:** An object representing a patient being monitored, containing MRN, name, DOB, sex, allergies, and bed location. Managed through the ADT workflow.
*   **Patient Admission:** The process of associating a patient with a device, creating a monitoring session. Supports three methods: Manual Entry, Barcode Scan, and Central Station Push. See `doc/19_ADT_WORKFLOW.md` for details.
*   **Patient Banner:** A persistent UI element in the header displaying critical patient identification and safety information. When patient is admitted, shows patient name prominently, MRN, bed location, and allergies. When no patient is admitted, shows "DISCHARGED / STANDBY" status. Tappable to open Admission Modal when no patient is assigned.
*   **PatientController:** A C++ QObject-based controller that exposes current patient data to the QML frontend and provides patient admission/discharge functionality via `admitPatient()` and `dischargePatient()`.
*   **PatientManager:** A C++ backend class managing patient-specific data, profiles, and ADT workflow (admission, discharge, transfer). Integrates with `IPatientLookupService` to retrieve patient information from external systems.
*   **IPatientLookupService:** An interface for looking up patient information from external systems (HIS/EHR) by patient ID or MRN. Supports both synchronous and asynchronous lookup patterns.
*   **PI (Perfusion Index):** A numerical value that indicates the pulsatile blood flow in peripheral tissues.
*   **Predictive Analytics:** Simulated feature that generates a risk score for potential adverse events based on current and historical patient data.
*   **Provisioning:** The process of securely configuring a device with server URL, certificates, and mTLS keys through a QR code-based pairing workflow. Replaces manual network configuration. See `doc/17_DEVICE_PROVISIONING.md` for complete workflow.
*   **ProvisioningController:** A C++ QObject-based controller that exposes device provisioning state, QR code, pairing code, and provisioning actions to the Network Settings View.
*   **ProvisioningService:** A C++ backend class that manages device provisioning workflow, including pairing code generation, QR code generation, configuration validation, and connection testing.
*   **PVC (Premature Ventricular Contraction):** An extra, abnormal heartbeat that begins in the ventricles.

## Q

*   **QML (Qt Modeling Language):** A declarative language for building highly dynamic, custom user interfaces. Used for the frontend of the application.
*   **Qt:** A cross-platform application development framework for C++ and QML.
*   **QObject:** The base class of all Qt objects, providing the signal/slot mechanism for inter-object communication.
*   **QSslConfiguration:** A Qt class used to configure SSL/TLS settings for network connections, including client certificates for mTLS.

## R

*   **RR (Respiration Rate):** The number of breaths a person takes per minute.

## S

*   **Security Audit Log:** Immutable audit trail stored in `security_audit_log` table for all security-relevant events including authentication, connections, certificate operations, and data transmission events. Used for compliance, forensics, and security monitoring.
*   **SettingsController:** A C++ QObject-based controller that exposes configurable device settings to the QML frontend and handles updates.
*   **SettingsManager:** A C++ backend class managing device configuration settings and user preferences.
*   **Sidebar:** A collapsible navigation panel on the left side of the UI.
*   **SpO2 (Peripheral Oxygen Saturation):** A measure of the oxygen level in the blood.
*   **SQLCipher:** An open-source extension to SQLite that provides transparent 256-bit AES encryption for database files.
*   **SQLite:** A C-language library that implements a small, fast, self-contained, high-reliability, full-featured, SQL database engine. Used for local data persistence.
*   **Stat Card:** A reusable QML component on the dashboard displaying a primary metric, secondary metrics, and a visualization for a specific vital sign or device status.
*   **SystemController:** A C++ QObject-based controller handling system-wide states like navigation, global alerts, and connection status.

## T

*   **Telemetry Data:** Data collected from the Z Monitor (vitals, alarms, status) and transmitted to a central server.
*   **TLS (Transport Layer Security):** A cryptographic protocol designed to provide communications security over a computer network.
*   **TopBar:** The header section of the UI, displaying connection status, notifications, system version, and clock.
*   **TrendsController:** A C++ QObject-based controller that provides historical data for plotting in the Trends View.
*   **Trends View:** A QML view dedicated to displaying historical vital sign data over selectable time ranges.

## U

*   **User Session:** A temporary authenticated session for a logged-in user, with configurable timeout (default: 30 minutes of inactivity).

## V

*   **Vitals Data:** Time-series physiological measurements (heart rate, SpO2, respiration rate, etc.) stored in the `vitals` table with configurable retention policy (default: 7 days).

## W

*   **Watchdog:** A lightweight monitoring component that detects system failures and triggers recovery procedures.
