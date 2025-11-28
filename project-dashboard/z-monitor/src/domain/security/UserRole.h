/**
 * @file UserRole.h
 * @brief User role enumeration for role-based access control (RBAC).
 * 
 * This file defines all user roles available in the Z Monitor system.
 * Roles are used to determine default permissions via PermissionRegistry.
 * 
 * @author Z Monitor Team
 * @date 2025-01-15
 */

#pragma once

#include <string>
#include <cctype>

namespace zmon {

/**
 * @enum UserRole
 * @brief Enumeration of user roles in the system.
 * 
 * Roles are hierarchical:
 * - Observer: Read-only access
 * - Technician: Device configuration and diagnostics
 * - Nurse: Basic clinical operations
 * - Physician: Clinical operations + advanced settings
 * - Administrator: Full access including user management
 * 
 * @note Default permissions for each role are defined in PermissionRegistry.
 * @see PermissionRegistry
 * @ingroup Domain
 */
enum class UserRole {
    Observer,      ///< Read-only access (medical students, observers)
    Technician,    ///< Device configuration, diagnostics, provisioning
    Nurse,         ///< Basic clinical operations (view vitals, acknowledge alarms, admit/discharge)
    Physician,     ///< Clinical operations + advanced settings (adjust thresholds, export data)
    Administrator, ///< Full access including user management, audit logs, firmware updates
    Count          ///< Total number of roles (sentinel value)
};

/**
 * @brief Convert UserRole to string representation.
 * 
 * @param role User role to convert
 * @return String representation (e.g., "NURSE", "PHYSICIAN")
 */
inline std::string roleToString(UserRole role) {
    switch (role) {
        case UserRole::Observer:
            return "OBSERVER";
        case UserRole::Technician:
            return "TECHNICIAN";
        case UserRole::Nurse:
            return "NURSE";
        case UserRole::Physician:
            return "PHYSICIAN";
        case UserRole::Administrator:
            return "ADMINISTRATOR";
        case UserRole::Count:
            return "UNKNOWN";
    }
    return "UNKNOWN";
}

/**
 * @brief Convert string to UserRole.
 * 
 * @param roleStr String representation of role (case-insensitive)
 * @return UserRole enum value, or UserRole::Count if invalid
 */
inline UserRole roleFromString(const std::string& roleStr) {
    std::string upper;
    for (char c : roleStr) {
        upper += static_cast<char>(std::toupper(static_cast<unsigned char>(c)));
    }
    
    if (upper == "OBSERVER") {
        return UserRole::Observer;
    } else if (upper == "TECHNICIAN" || upper == "TECH") {
        return UserRole::Technician;
    } else if (upper == "NURSE") {
        return UserRole::Nurse;
    } else if (upper == "PHYSICIAN" || upper == "PHYS") {
        return UserRole::Physician;
    } else if (upper == "ADMINISTRATOR" || upper == "ADMIN") {
        return UserRole::Administrator;
    }
    return UserRole::Count;
}

/**
 * @brief Get display name for a user role.
 * 
 * @param role User role to get display name for
 * @return Human-readable display name (e.g., "Nurse", "Physician")
 */
inline std::string roleDisplayName(UserRole role) {
    switch (role) {
        case UserRole::Observer:
            return "Observer";
        case UserRole::Technician:
            return "Technician";
        case UserRole::Nurse:
            return "Nurse";
        case UserRole::Physician:
            return "Physician";
        case UserRole::Administrator:
            return "Administrator";
        case UserRole::Count:
            return "Unknown";
    }
    return "Unknown";
}

} // namespace zmon

