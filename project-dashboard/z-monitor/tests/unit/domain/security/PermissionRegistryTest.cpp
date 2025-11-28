/**
 * @file PermissionRegistryTest.cpp
 * @brief Unit tests for PermissionRegistry.
 * 
 * @author Z Monitor Team
 * @date 2025-01-15
 */

#include <gtest/gtest.h>
#include "domain/security/PermissionRegistry.h"
#include "domain/security/Permission.h"
#include "domain/security/UserRole.h"

using namespace zmon;

class PermissionRegistryTest : public ::testing::Test {
protected:
    void SetUp() override {
        registry = &PermissionRegistry::instance();
    }

    const PermissionRegistry* registry;
};

// Test: Observer role has no permissions
TEST_F(PermissionRegistryTest, ObserverRole_HasNoPermissions) {
    PermissionSet perms = registry->permissionsForRole(UserRole::Observer);
    EXPECT_EQ(perms, 0u);
}

// Test: Technician role has device configuration permissions
TEST_F(PermissionRegistryTest, TechnicianRole_HasDevicePermissions) {
    PermissionSet perms = registry->permissionsForRole(UserRole::Technician);
    
    // Should have device configuration permissions
    EXPECT_TRUE(hasPermission(perms, Permission::AccessSystemSettings));
    EXPECT_TRUE(hasPermission(perms, Permission::ConfigureDevice));
    EXPECT_TRUE(hasPermission(perms, Permission::EnterProvisioningMode));
    EXPECT_TRUE(hasPermission(perms, Permission::ViewDiagnostics));
    EXPECT_TRUE(hasPermission(perms, Permission::ViewLogs));
    EXPECT_TRUE(hasPermission(perms, Permission::ExportLogs));
    EXPECT_TRUE(hasPermission(perms, Permission::CalibrateDevice));
    
    // Should NOT have clinical permissions
    EXPECT_FALSE(hasPermission(perms, Permission::ViewVitals));
    EXPECT_FALSE(hasPermission(perms, Permission::AdmitPatient));
    EXPECT_FALSE(hasPermission(perms, Permission::ManageUsers));
}

// Test: Nurse role has basic clinical permissions
TEST_F(PermissionRegistryTest, NurseRole_HasBasicClinicalPermissions) {
    PermissionSet perms = registry->permissionsForRole(UserRole::Nurse);
    
    // Should have monitoring permissions
    EXPECT_TRUE(hasPermission(perms, Permission::ViewVitals));
    EXPECT_TRUE(hasPermission(perms, Permission::ViewWaveforms));
    EXPECT_TRUE(hasPermission(perms, Permission::ViewTrends));
    
    // Should have alarm permissions (short silence only)
    EXPECT_TRUE(hasPermission(perms, Permission::ViewAlarms));
    EXPECT_TRUE(hasPermission(perms, Permission::AcknowledgeAlarm));
    EXPECT_TRUE(hasPermission(perms, Permission::SilenceAlarmShort));
    EXPECT_FALSE(hasPermission(perms, Permission::SilenceAlarmExtended));
    EXPECT_FALSE(hasPermission(perms, Permission::AdjustAlarmThresholds));
    
    // Should have patient management permissions
    EXPECT_TRUE(hasPermission(perms, Permission::ViewPatientData));
    EXPECT_TRUE(hasPermission(perms, Permission::AdmitPatient));
    EXPECT_TRUE(hasPermission(perms, Permission::DischargePatient));
    EXPECT_TRUE(hasPermission(perms, Permission::TransferPatient));
    
    // Should NOT have export or admin permissions
    EXPECT_FALSE(hasPermission(perms, Permission::ExportVitals));
    EXPECT_FALSE(hasPermission(perms, Permission::ManageUsers));
}

// Test: Physician role has all nurse permissions plus advanced
TEST_F(PermissionRegistryTest, PhysicianRole_HasAdvancedClinicalPermissions) {
    PermissionSet perms = registry->permissionsForRole(UserRole::Physician);
    
    // Should have all nurse permissions
    EXPECT_TRUE(hasPermission(perms, Permission::ViewVitals));
    EXPECT_TRUE(hasPermission(perms, Permission::ViewWaveforms));
    EXPECT_TRUE(hasPermission(perms, Permission::ViewTrends));
    EXPECT_TRUE(hasPermission(perms, Permission::ViewAlarms));
    EXPECT_TRUE(hasPermission(perms, Permission::AcknowledgeAlarm));
    EXPECT_TRUE(hasPermission(perms, Permission::ViewPatientData));
    EXPECT_TRUE(hasPermission(perms, Permission::AdmitPatient));
    
    // Should have extended alarm permissions
    EXPECT_TRUE(hasPermission(perms, Permission::SilenceAlarmExtended));
    EXPECT_TRUE(hasPermission(perms, Permission::AdjustAlarmThresholds));
    EXPECT_TRUE(hasPermission(perms, Permission::OverrideAlarm));
    
    // Should have export permissions
    EXPECT_TRUE(hasPermission(perms, Permission::ExportVitals));
    EXPECT_TRUE(hasPermission(perms, Permission::ExportTrends));
    
    // Should NOT have admin permissions
    EXPECT_FALSE(hasPermission(perms, Permission::ManageUsers));
    EXPECT_FALSE(hasPermission(perms, Permission::ViewAuditLogs));
}

// Test: Administrator role has all permissions
TEST_F(PermissionRegistryTest, AdministratorRole_HasAllPermissions) {
    PermissionSet perms = registry->permissionsForRole(UserRole::Administrator);
    
    // Should have all permissions
    EXPECT_TRUE(hasPermission(perms, Permission::ViewVitals));
    EXPECT_TRUE(hasPermission(perms, Permission::ViewWaveforms));
    EXPECT_TRUE(hasPermission(perms, Permission::ViewTrends));
    EXPECT_TRUE(hasPermission(perms, Permission::ViewAlarms));
    EXPECT_TRUE(hasPermission(perms, Permission::AcknowledgeAlarm));
    EXPECT_TRUE(hasPermission(perms, Permission::SilenceAlarmShort));
    EXPECT_TRUE(hasPermission(perms, Permission::SilenceAlarmExtended));
    EXPECT_TRUE(hasPermission(perms, Permission::AdjustAlarmThresholds));
    EXPECT_TRUE(hasPermission(perms, Permission::OverrideAlarm));
    EXPECT_TRUE(hasPermission(perms, Permission::ViewPatientData));
    EXPECT_TRUE(hasPermission(perms, Permission::AdmitPatient));
    EXPECT_TRUE(hasPermission(perms, Permission::DischargePatient));
    EXPECT_TRUE(hasPermission(perms, Permission::TransferPatient));
    EXPECT_TRUE(hasPermission(perms, Permission::ExportVitals));
    EXPECT_TRUE(hasPermission(perms, Permission::ExportTrends));
    EXPECT_TRUE(hasPermission(perms, Permission::AccessSystemSettings));
    EXPECT_TRUE(hasPermission(perms, Permission::ConfigureDevice));
    EXPECT_TRUE(hasPermission(perms, Permission::EnterProvisioningMode));
    EXPECT_TRUE(hasPermission(perms, Permission::ViewDiagnostics));
    EXPECT_TRUE(hasPermission(perms, Permission::ViewLogs));
    EXPECT_TRUE(hasPermission(perms, Permission::ExportLogs));
    EXPECT_TRUE(hasPermission(perms, Permission::CalibrateDevice));
    EXPECT_TRUE(hasPermission(perms, Permission::ManageUsers));
    EXPECT_TRUE(hasPermission(perms, Permission::ViewAuditLogs));
    EXPECT_TRUE(hasPermission(perms, Permission::ManageSettings));
    EXPECT_TRUE(hasPermission(perms, Permission::ResetDevice));
    EXPECT_TRUE(hasPermission(perms, Permission::UpdateFirmware));
}

// Test: Permission string serialization
TEST_F(PermissionRegistryTest, PermissionToString_ReturnsCorrectString) {
    EXPECT_EQ(registry->toString(Permission::ViewVitals), std::string("VIEW_VITALS"));
    EXPECT_EQ(registry->toString(Permission::AcknowledgeAlarm), std::string("ACKNOWLEDGE_ALARM"));
    EXPECT_EQ(registry->toString(Permission::AdmitPatient), std::string("ADMIT_PATIENT"));
    EXPECT_EQ(registry->toString(Permission::ManageUsers), std::string("MANAGE_USERS"));
}

// Test: Permission display name
TEST_F(PermissionRegistryTest, PermissionToDisplayName_ReturnsCorrectName) {
    EXPECT_EQ(registry->toDisplayName(Permission::ViewVitals), std::string("View Vitals"));
    EXPECT_EQ(registry->toDisplayName(Permission::AcknowledgeAlarm), std::string("Acknowledge Alarm"));
    EXPECT_EQ(registry->toDisplayName(Permission::AdmitPatient), std::string("Admit Patient"));
    EXPECT_EQ(registry->toDisplayName(Permission::ManageUsers), std::string("Manage Users"));
}

// Test: String to permission parsing
TEST_F(PermissionRegistryTest, PermissionFromString_ParsesCorrectly) {
    EXPECT_EQ(registry->fromString("VIEW_VITALS"), Permission::ViewVitals);
    EXPECT_EQ(registry->fromString("ACKNOWLEDGE_ALARM"), Permission::AcknowledgeAlarm);
    EXPECT_EQ(registry->fromString("ADMIT_PATIENT"), Permission::AdmitPatient);
    EXPECT_EQ(registry->fromString("MANAGE_USERS"), Permission::ManageUsers);
    
    // Case-insensitive
    EXPECT_EQ(registry->fromString("view_vitals"), Permission::ViewVitals);
    EXPECT_EQ(registry->fromString("View_Vitals"), Permission::ViewVitals);
    
    // Invalid string returns Count
    EXPECT_EQ(registry->fromString("INVALID_PERMISSION"), Permission::Count);
    EXPECT_EQ(registry->fromString(""), Permission::Count);
}

// Test: Permission helper functions
TEST_F(PermissionRegistryTest, PermissionHelpers_WorkCorrectly) {
    PermissionSet perms = 0;
    
    // Add permissions
    perms = addPermission(perms, Permission::ViewVitals);
    perms = addPermission(perms, Permission::AcknowledgeAlarm);
    
    // Check permissions
    EXPECT_TRUE(hasPermission(perms, Permission::ViewVitals));
    EXPECT_TRUE(hasPermission(perms, Permission::AcknowledgeAlarm));
    EXPECT_FALSE(hasPermission(perms, Permission::AdmitPatient));
    
    // Remove permission
    perms = removePermission(perms, Permission::ViewVitals);
    EXPECT_FALSE(hasPermission(perms, Permission::ViewVitals));
    EXPECT_TRUE(hasPermission(perms, Permission::AcknowledgeAlarm));
}

// Test: Registry is singleton
TEST_F(PermissionRegistryTest, Registry_IsSingleton) {
    const PermissionRegistry& reg1 = PermissionRegistry::instance();
    const PermissionRegistry& reg2 = PermissionRegistry::instance();
    
    // Should be the same instance
    EXPECT_EQ(&reg1, &reg2);
}

// Test: Role hierarchy (Physician has all Nurse permissions)
TEST_F(PermissionRegistryTest, RoleHierarchy_PhysicianHasAllNursePermissions) {
    PermissionSet nursePerms = registry->permissionsForRole(UserRole::Nurse);
    PermissionSet physicianPerms = registry->permissionsForRole(UserRole::Physician);
    
    // Physician should have all nurse permissions
    EXPECT_EQ(physicianPerms & nursePerms, nursePerms);
    
    // Physician should have additional permissions
    EXPECT_TRUE(hasPermission(physicianPerms, Permission::SilenceAlarmExtended));
    EXPECT_TRUE(hasPermission(physicianPerms, Permission::AdjustAlarmThresholds));
    EXPECT_TRUE(hasPermission(physicianPerms, Permission::ExportVitals));
}

// Test: Administrator has all permissions from all roles
TEST_F(PermissionRegistryTest, RoleHierarchy_AdministratorHasAllPermissions) {
    PermissionSet adminPerms = registry->permissionsForRole(UserRole::Administrator);
    PermissionSet nursePerms = registry->permissionsForRole(UserRole::Nurse);
    PermissionSet physicianPerms = registry->permissionsForRole(UserRole::Physician);
    PermissionSet techPerms = registry->permissionsForRole(UserRole::Technician);
    
    // Administrator should have all permissions from all roles
    EXPECT_EQ(adminPerms & nursePerms, nursePerms);
    EXPECT_EQ(adminPerms & physicianPerms, physicianPerms);
    EXPECT_EQ(adminPerms & techPerms, techPerms);
}

