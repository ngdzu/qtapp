/**
 * @file PermissionRegistry.cpp
 * @brief Implementation of PermissionRegistry.
 * 
 * @author Z Monitor Team
 * @date 2025-01-15
 */

#include "PermissionRegistry.h"
#include <cassert>
#include <cctype>
#include <algorithm>

namespace zmon {

PermissionRegistry::PermissionRegistry() {
    initializeRoleMatrix();
    initializePermissionStrings();
}

const PermissionRegistry& PermissionRegistry::instance() {
    static const PermissionRegistry s_instance;
    return s_instance;
}

PermissionSet PermissionRegistry::permissionsForRole(UserRole role) const {
    size_t roleIndex = static_cast<size_t>(role);
    if (roleIndex >= m_roleMatrix.size()) {
        return 0; // Invalid role, return empty permission set
    }
    return m_roleMatrix[roleIndex];
}

std::string PermissionRegistry::toString(Permission permission) const {
    size_t permIndex = static_cast<size_t>(permission);
    if (permIndex >= m_permissionStrings.size()) {
        return std::string();
    }
    return m_permissionStrings[permIndex];
}

std::string PermissionRegistry::toDisplayName(Permission permission) const {
    size_t permIndex = static_cast<size_t>(permission);
    if (permIndex >= m_permissionDisplayNames.size()) {
        return std::string();
    }
    return m_permissionDisplayNames[permIndex];
}

Permission PermissionRegistry::fromString(const std::string& permissionStr) const {
    // Convert to uppercase for case-insensitive comparison
    std::string upper;
    for (char c : permissionStr) {
        upper += static_cast<char>(std::toupper(static_cast<unsigned char>(c)));
    }
    
    // Linear search through permission strings (small array, acceptable)
    for (size_t i = 0; i < static_cast<size_t>(Permission::Count); ++i) {
        std::string permUpper;
        for (char c : m_permissionStrings[i]) {
            permUpper += static_cast<char>(std::toupper(static_cast<unsigned char>(c)));
        }
        if (permUpper == upper) {
            return static_cast<Permission>(i);
        }
    }
    return Permission::Count; // Not found
}

void PermissionRegistry::initializeRoleMatrix() {
    // Initialize all roles to empty permission set
    m_roleMatrix.fill(0);

    // Helper lambda to build permission sets
    auto buildSet = [](std::initializer_list<Permission> perms) {
        PermissionSet set = 0;
        for (Permission p : perms) {
            set = addPermission(set, p);
        }
        return set;
    };

    // Observer: Read-only access (no permissions by default)
    // Note: Observer role may be added later if needed
    m_roleMatrix[static_cast<size_t>(UserRole::Observer)] = 0;

    // Technician: Device configuration, diagnostics, provisioning
    m_roleMatrix[static_cast<size_t>(UserRole::Technician)] = buildSet({
        Permission::AccessSystemSettings,
        Permission::ConfigureDevice,
        Permission::EnterProvisioningMode,
        Permission::ViewDiagnostics,
        Permission::ViewLogs,
        Permission::ExportLogs,
        Permission::CalibrateDevice
    });

    // Nurse: Basic clinical operations
    m_roleMatrix[static_cast<size_t>(UserRole::Nurse)] = buildSet({
        // Monitoring
        Permission::ViewVitals,
        Permission::ViewWaveforms,
        Permission::ViewTrends,
        // Alarms
        Permission::ViewAlarms,
        Permission::AcknowledgeAlarm,
        Permission::SilenceAlarmShort,
        // Patient Management
        Permission::ViewPatientData,
        Permission::AdmitPatient,
        Permission::DischargePatient,
        Permission::TransferPatient
    });

    // Physician: Clinical operations + advanced settings
    m_roleMatrix[static_cast<size_t>(UserRole::Physician)] = buildSet({
        // All Nurse permissions
        Permission::ViewVitals,
        Permission::ViewWaveforms,
        Permission::ViewTrends,
        Permission::ViewAlarms,
        Permission::AcknowledgeAlarm,
        Permission::SilenceAlarmShort,
        Permission::ViewPatientData,
        Permission::AdmitPatient,
        Permission::DischargePatient,
        Permission::TransferPatient,
        // Additional Physician permissions
        Permission::SilenceAlarmExtended,
        Permission::AdjustAlarmThresholds,
        Permission::OverrideAlarm,
        Permission::ExportVitals,
        Permission::ExportTrends
    });

    // Administrator: Full access
    m_roleMatrix[static_cast<size_t>(UserRole::Administrator)] = buildSet({
        // All permissions
        Permission::ViewVitals,
        Permission::ViewWaveforms,
        Permission::ViewTrends,
        Permission::ViewAlarms,
        Permission::AcknowledgeAlarm,
        Permission::SilenceAlarmShort,
        Permission::SilenceAlarmExtended,
        Permission::AdjustAlarmThresholds,
        Permission::OverrideAlarm,
        Permission::ViewPatientData,
        Permission::AdmitPatient,
        Permission::DischargePatient,
        Permission::TransferPatient,
        Permission::ExportVitals,
        Permission::ExportTrends,
        Permission::AccessSystemSettings,
        Permission::ConfigureDevice,
        Permission::EnterProvisioningMode,
        Permission::ViewDiagnostics,
        Permission::ViewLogs,
        Permission::ExportLogs,
        Permission::CalibrateDevice,
        Permission::ManageUsers,
        Permission::ViewAuditLogs,
        Permission::ManageSettings,
        Permission::ResetDevice,
        Permission::UpdateFirmware
    });
}

void PermissionRegistry::initializePermissionStrings() {
    // Initialize permission string mappings
    m_permissionStrings[static_cast<size_t>(Permission::ViewVitals)] = std::string("VIEW_VITALS");
    m_permissionStrings[static_cast<size_t>(Permission::ViewWaveforms)] = std::string("VIEW_WAVEFORMS");
    m_permissionStrings[static_cast<size_t>(Permission::ViewTrends)] = std::string("VIEW_TRENDS");
    m_permissionStrings[static_cast<size_t>(Permission::ViewAlarms)] = std::string("VIEW_ALARMS");
    m_permissionStrings[static_cast<size_t>(Permission::AcknowledgeAlarm)] = std::string("ACKNOWLEDGE_ALARM");
    m_permissionStrings[static_cast<size_t>(Permission::SilenceAlarmShort)] = std::string("SILENCE_ALARM_SHORT");
    m_permissionStrings[static_cast<size_t>(Permission::SilenceAlarmExtended)] = std::string("SILENCE_ALARM_EXTENDED");
    m_permissionStrings[static_cast<size_t>(Permission::AdjustAlarmThresholds)] = std::string("ADJUST_ALARM_THRESHOLDS");
    m_permissionStrings[static_cast<size_t>(Permission::OverrideAlarm)] = std::string("OVERRIDE_ALARM");
    m_permissionStrings[static_cast<size_t>(Permission::ViewPatientData)] = std::string("VIEW_PATIENT_DATA");
    m_permissionStrings[static_cast<size_t>(Permission::AdmitPatient)] = std::string("ADMIT_PATIENT");
    m_permissionStrings[static_cast<size_t>(Permission::DischargePatient)] = std::string("DISCHARGE_PATIENT");
    m_permissionStrings[static_cast<size_t>(Permission::TransferPatient)] = std::string("TRANSFER_PATIENT");
    m_permissionStrings[static_cast<size_t>(Permission::ExportVitals)] = std::string("EXPORT_VITALS");
    m_permissionStrings[static_cast<size_t>(Permission::ExportTrends)] = std::string("EXPORT_TRENDS");
    m_permissionStrings[static_cast<size_t>(Permission::AccessSystemSettings)] = std::string("ACCESS_SYSTEM_SETTINGS");
    m_permissionStrings[static_cast<size_t>(Permission::ConfigureDevice)] = std::string("CONFIGURE_DEVICE");
    m_permissionStrings[static_cast<size_t>(Permission::EnterProvisioningMode)] = std::string("ENTER_PROVISIONING_MODE");
    m_permissionStrings[static_cast<size_t>(Permission::ViewDiagnostics)] = std::string("VIEW_DIAGNOSTICS");
    m_permissionStrings[static_cast<size_t>(Permission::ViewLogs)] = std::string("VIEW_LOGS");
    m_permissionStrings[static_cast<size_t>(Permission::ExportLogs)] = std::string("EXPORT_LOGS");
    m_permissionStrings[static_cast<size_t>(Permission::CalibrateDevice)] = std::string("CALIBRATE_DEVICE");
    m_permissionStrings[static_cast<size_t>(Permission::ManageUsers)] = std::string("MANAGE_USERS");
    m_permissionStrings[static_cast<size_t>(Permission::ViewAuditLogs)] = std::string("VIEW_AUDIT_LOGS");
    m_permissionStrings[static_cast<size_t>(Permission::ManageSettings)] = std::string("MANAGE_SETTINGS");
    m_permissionStrings[static_cast<size_t>(Permission::ResetDevice)] = std::string("RESET_DEVICE");
    m_permissionStrings[static_cast<size_t>(Permission::UpdateFirmware)] = std::string("UPDATE_FIRMWARE");

    // Initialize permission display names
    m_permissionDisplayNames[static_cast<size_t>(Permission::ViewVitals)] = std::string("View Vitals");
    m_permissionDisplayNames[static_cast<size_t>(Permission::ViewWaveforms)] = std::string("View Waveforms");
    m_permissionDisplayNames[static_cast<size_t>(Permission::ViewTrends)] = std::string("View Trends");
    m_permissionDisplayNames[static_cast<size_t>(Permission::ViewAlarms)] = std::string("View Alarms");
    m_permissionDisplayNames[static_cast<size_t>(Permission::AcknowledgeAlarm)] = std::string("Acknowledge Alarm");
    m_permissionDisplayNames[static_cast<size_t>(Permission::SilenceAlarmShort)] = std::string("Silence Alarm (Short)");
    m_permissionDisplayNames[static_cast<size_t>(Permission::SilenceAlarmExtended)] = std::string("Silence Alarm (Extended)");
    m_permissionDisplayNames[static_cast<size_t>(Permission::AdjustAlarmThresholds)] = std::string("Adjust Alarm Thresholds");
    m_permissionDisplayNames[static_cast<size_t>(Permission::OverrideAlarm)] = std::string("Override Alarm");
    m_permissionDisplayNames[static_cast<size_t>(Permission::ViewPatientData)] = std::string("View Patient Data");
    m_permissionDisplayNames[static_cast<size_t>(Permission::AdmitPatient)] = std::string("Admit Patient");
    m_permissionDisplayNames[static_cast<size_t>(Permission::DischargePatient)] = std::string("Discharge Patient");
    m_permissionDisplayNames[static_cast<size_t>(Permission::TransferPatient)] = std::string("Transfer Patient");
    m_permissionDisplayNames[static_cast<size_t>(Permission::ExportVitals)] = std::string("Export Vitals");
    m_permissionDisplayNames[static_cast<size_t>(Permission::ExportTrends)] = std::string("Export Trends");
    m_permissionDisplayNames[static_cast<size_t>(Permission::AccessSystemSettings)] = std::string("Access System Settings");
    m_permissionDisplayNames[static_cast<size_t>(Permission::ConfigureDevice)] = std::string("Configure Device");
    m_permissionDisplayNames[static_cast<size_t>(Permission::EnterProvisioningMode)] = std::string("Enter Provisioning Mode");
    m_permissionDisplayNames[static_cast<size_t>(Permission::ViewDiagnostics)] = std::string("View Diagnostics");
    m_permissionDisplayNames[static_cast<size_t>(Permission::ViewLogs)] = std::string("View Logs");
    m_permissionDisplayNames[static_cast<size_t>(Permission::ExportLogs)] = std::string("Export Logs");
    m_permissionDisplayNames[static_cast<size_t>(Permission::CalibrateDevice)] = std::string("Calibrate Device");
    m_permissionDisplayNames[static_cast<size_t>(Permission::ManageUsers)] = std::string("Manage Users");
    m_permissionDisplayNames[static_cast<size_t>(Permission::ViewAuditLogs)] = std::string("View Audit Logs");
    m_permissionDisplayNames[static_cast<size_t>(Permission::ManageSettings)] = std::string("Manage Settings");
    m_permissionDisplayNames[static_cast<size_t>(Permission::ResetDevice)] = std::string("Reset Device");
    m_permissionDisplayNames[static_cast<size_t>(Permission::UpdateFirmware)] = std::string("Update Firmware");
}

} // namespace zmon

