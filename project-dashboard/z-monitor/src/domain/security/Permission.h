/**
 * @file Permission.h
 * @brief Permission enumeration for role-based access control (RBAC).
 * 
 * This file defines all permissions available in the Z Monitor system.
 * Permissions are used to control access to features and actions based on
 * user roles (Nurse, Physician, Technician, Administrator).
 * 
 * @author Z Monitor Team
 * @date 2025-01-15
 */

#pragma once

#include <cstdint>

namespace zmon {

/**
 * @enum Permission
 * @brief Enumeration of all system permissions.
 * 
 * Permissions are organized by category:
 * - Monitoring: View vitals, waveforms, trends
 * - Alarms: View, acknowledge, silence, adjust thresholds
 * - Patient Management: View, admit, discharge, transfer patients
 * - Data Export: Export vitals and trends
 * - Device Configuration: Access system settings, configure device, diagnostics
 * - Administration: Manage users, view audit logs, manage settings, reset device
 * 
 * @note Permissions are mapped to roles via PermissionRegistry.
 * @see PermissionRegistry
 * @ingroup Domain
 */
enum class Permission : uint32_t {
    // Monitoring
    ViewVitals = 1 << 0,           ///< View real-time vital signs
    ViewWaveforms = 1 << 1,       ///< View ECG and pleth waveforms
    ViewTrends = 1 << 2,           ///< View historical vital signs trends
    
    // Alarms
    ViewAlarms = 1 << 3,           ///< View active alarms
    AcknowledgeAlarm = 1 << 4,     ///< Acknowledge alarms
    SilenceAlarmShort = 1 << 5,    ///< Silence alarms for short duration (≤60s)
    SilenceAlarmExtended = 1 << 6, ///< Silence alarms for extended duration (>60s)
    AdjustAlarmThresholds = 1 << 7,///< Adjust alarm threshold values
    OverrideAlarm = 1 << 8,        ///< Override alarm conditions
    
    // Patient Management
    ViewPatientData = 1 << 9,      ///< View patient information
    AdmitPatient = 1 << 10,       ///< Admit a patient to the device
    DischargePatient = 1 << 11,   ///< Discharge a patient from the device
    TransferPatient = 1 << 12,    ///< Transfer a patient to another bed/device
    
    // Data Export
    ExportVitals = 1 << 13,       ///< Export vital signs data
    ExportTrends = 1 << 14,       ///< Export trend data
    
    // Device Configuration
    AccessSystemSettings = 1 << 15,///< Access system settings view
    ConfigureDevice = 1 << 16,    ///< Configure device settings
    EnterProvisioningMode = 1 << 17,///< Enter device provisioning mode
    ViewDiagnostics = 1 << 18,     ///< View system diagnostics
    ViewLogs = 1 << 19,            ///< View application logs
    ExportLogs = 1 << 20,          ///< Export application logs
    CalibrateDevice = 1 << 21,     ///< Calibrate device sensors
    
    // Administration
    ManageUsers = 1 << 22,         ///< Manage user accounts
    ViewAuditLogs = 1 << 23,       ///< View security audit logs
    ManageSettings = 1 << 24,      ///< Manage system settings
    ResetDevice = 1 << 25,         ///< Reset device to factory defaults
    UpdateFirmware = 1 << 26,      ///< Update device firmware
    
    // Sentinel value (must be last)
    Count = 27                     ///< Total number of permissions
};

/**
 * @brief Permission bitset type.
 * 
 * Uses uint32_t to store up to 32 permission flags.
 * Each bit represents a permission (Permission enum value).
 */
using PermissionSet = uint32_t;

/**
 * @brief Check if a permission is set in a permission set.
 * 
 * @param permissions Permission set to check
 * @param permission Permission to check for
 * @return true if permission is set, false otherwise
 */
inline bool hasPermission(PermissionSet permissions, Permission permission) {
    return (permissions & static_cast<uint32_t>(permission)) != 0;
}

/**
 * @brief Add a permission to a permission set.
 * 
 * @param permissions Permission set to modify
 * @param permission Permission to add
 * @return Modified permission set
 */
inline PermissionSet addPermission(PermissionSet permissions, Permission permission) {
    return permissions | static_cast<uint32_t>(permission);
}

/**
 * @brief Remove a permission from a permission set.
 * 
 * @param permissions Permission set to modify
 * @param permission Permission to remove
 * @return Modified permission set
 */
inline PermissionSet removePermission(PermissionSet permissions, Permission permission) {
    return permissions & ~static_cast<uint32_t>(permission);
}

} // namespace zmon

