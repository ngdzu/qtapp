/**
 * @file PermissionRegistry.h
 * @brief Compile-time registry mapping user roles to default permissions.
 * 
 * This class provides a single source of truth for role-based access control
 * (RBAC) permissions. It maps each UserRole to its default PermissionSet
 * according to the RBAC matrix defined in the authentication workflow.
 * 
 * The registry is thread-safe and provides helper methods for:
 * - Resolving role → permissions
 * - Permission → string serialization
 * - Permission → display name
 * 
 * @note This is a compile-time registry. Permission mappings are defined
 *       at compile time and cannot be modified at runtime.
 * 
 * @author Z Monitor Team
 * @date 2025-01-15
 */

#pragma once

#include "Permission.h"
#include "UserRole.h"
#include <string>
#include <array>
#include <vector>

namespace zmon {

/**
 * @class PermissionRegistry
 * @brief Singleton registry for role-to-permission mappings.
 * 
 * This class maintains the canonical mapping between UserRole values and
 * their default PermissionSet according to the RBAC matrix. It serves as
 * the single source of truth for both SecurityService and UI controllers
 * when they need to resolve permissions or display human-readable
 * permission descriptions.
 * 
 * @note Thread-safe: All methods are const and thread-safe.
 * @ingroup Domain
 */
class PermissionRegistry {
public:
    /**
     * @brief Get the singleton instance of PermissionRegistry.
     * 
     * @return Const reference to the singleton instance
     */
    static const PermissionRegistry& instance();

    /**
     * @brief Get permissions for a specific role.
     * 
     * Returns the default PermissionSet for the given role according to
     * the RBAC matrix. This is the compile-time mapping defined in the
     * registry initialization.
     * 
     * @param role User role to get permissions for
     * @return PermissionSet containing all permissions for the role
     */
    PermissionSet permissionsForRole(UserRole role) const;

    /**
     * @brief Convert a permission to its string representation.
     * 
     * Returns the canonical string representation of a permission (e.g.,
     * "VIEW_VITALS", "ACKNOWLEDGE_ALARM") for serialization in telemetry,
     * audit logs, or API responses.
     * 
     * @param permission Permission to convert
     * @return String representation (e.g., "VIEW_VITALS")
     */
    std::string toString(Permission permission) const;

    /**
     * @brief Convert a permission to its display name.
     * 
     * Returns a human-readable display name for a permission (e.g.,
     * "View Vitals", "Acknowledge Alarm") for use in UI.
     * 
     * @param permission Permission to convert
     * @return Human-readable display name (e.g., "View Vitals")
     */
    std::string toDisplayName(Permission permission) const;

    /**
     * @brief Convert a string to a Permission enum value.
     * 
     * Parses a string representation (e.g., "VIEW_VITALS") and returns
     * the corresponding Permission enum value.
     * 
     * @param permissionStr String representation of permission (case-insensitive)
     * @return Permission enum value, or Permission::Count if invalid
     */
    Permission fromString(const std::string& permissionStr) const;

    /**
     * @brief Get total number of permissions.
     * 
     * @return Number of permissions defined in the system
     */
    static constexpr size_t permissionCount() {
        return static_cast<size_t>(Permission::Count);
    }

    /**
     * @brief Get total number of roles.
     * 
     * @return Number of roles defined in the system
     */
    static constexpr size_t roleCount() {
        return static_cast<size_t>(UserRole::Count);
    }

private:
    /**
     * @brief Private constructor (singleton pattern).
     * 
     * Initializes the role-to-permission mapping according to the RBAC matrix.
     */
    PermissionRegistry();

    /**
     * @brief Deleted copy constructor (singleton pattern).
     */
    PermissionRegistry(const PermissionRegistry&) = delete;

    /**
     * @brief Deleted assignment operator (singleton pattern).
     */
    PermissionRegistry& operator=(const PermissionRegistry&) = delete;

    /**
     * @brief Initialize the role-to-permission mapping.
     * 
     * Sets up the compile-time mapping according to the RBAC matrix defined
     * in the authentication workflow document.
     */
    void initializeRoleMatrix();

    /**
     * @brief Initialize permission string mappings.
     * 
     * Sets up the mapping from Permission enum to string representation
     * and display names.
     */
    void initializePermissionStrings();

    /// Role-to-permission mapping matrix (compile-time)
    std::array<PermissionSet, static_cast<size_t>(UserRole::Count)> m_roleMatrix;

    /// Permission to string mapping
    std::array<std::string, static_cast<size_t>(Permission::Count)> m_permissionStrings;

    /// Permission to display name mapping
    std::array<std::string, static_cast<size_t>(Permission::Count)> m_permissionDisplayNames;
};

} // namespace zmon

