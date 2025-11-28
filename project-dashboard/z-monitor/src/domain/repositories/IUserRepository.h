/**
 * @file IUserRepository.h
 * @brief Repository interface for user data persistence.
 * 
 * This file contains the IUserRepository interface which defines the contract
 * for persisting and retrieving user data. Repository interfaces are
 * defined in the domain layer to ensure business logic is independent of
 * infrastructure implementations.
 * 
 * @author Z Monitor Team
 * @date 2025-01-15
 */

#pragma once

#include <string>
#include <vector>
#include <memory>

namespace ZMonitor {
namespace Domain {
namespace Security {
    class UserSession;
    class PinCredential;
}

namespace Repositories {

/**
 * @class IUserRepository
 * @brief Repository interface for user data persistence.
 * 
 * This interface defines the contract for persisting and retrieving user
 * data. Implementations (e.g., SQLiteUserRepository) live in the
 * infrastructure layer and provide database persistence.
 * 
 * @note Repository interfaces are defined in domain layer (no infrastructure dependencies).
 * @note Implementations are in infrastructure layer (SQLite, in-memory, etc.).
 */
class IUserRepository {
public:
    /**
     * @brief User information structure.
     */
    struct UserInfo {
        std::string userId;
        std::string username;
        std::string role;  // "NURSE", "PHYSICIAN", "TECHNICIAN", "ADMINISTRATOR"
        int64_t createdAtMs;
        int64_t lastLoginMs;
    };
    
    /**
     * @brief Virtual destructor.
     */
    virtual ~IUserRepository() = default;
    
    /**
     * @brief Find user by username.
     * 
     * Retrieves user information by username.
     * 
     * @param username Username
     * @return User information, or empty if not found
     */
    virtual UserInfo findByUsername(const std::string& username) = 0;
    
    /**
     * @brief Find user by user ID.
     * 
     * Retrieves user information by user identifier.
     * 
     * @param userId User identifier
     * @return User information, or empty if not found
     */
    virtual UserInfo findById(const std::string& userId) = 0;
    
    /**
     * @brief Save user.
     * 
     * Persists user information to storage.
     * 
     * @param user User information to save
     * @return true if save succeeded, false otherwise
     */
    virtual bool save(const UserInfo& user) = 0;
    
    /**
     * @brief Save user PIN credential.
     * 
     * Persists hashed PIN credential for a user.
     * 
     * @param userId User identifier
     * @param credential Hashed PIN credential
     * @return true if save succeeded, false otherwise
     */
    virtual bool saveCredential(const std::string& userId, const Security::PinCredential& credential) = 0;
    
    /**
     * @brief Verify user PIN.
     * 
     * Verifies a PIN against stored credential.
     * 
     * @param userId User identifier
     * @param pin Plain-text PIN to verify
     * @return true if PIN is correct, false otherwise
     */
    virtual bool verifyPin(const std::string& userId, const std::string& pin) = 0;
    
    /**
     * @brief Get all users.
     * 
     * Retrieves all user information from storage.
     * 
     * @return Vector of user information
     */
    virtual std::vector<UserInfo> findAll() = 0;
    
    /**
     * @brief Delete user by user ID.
     * 
     * Removes user from storage.
     * 
     * @param userId User identifier
     * @return true if deletion succeeded, false otherwise
     */
    virtual bool remove(const std::string& userId) = 0;
    
    /**
     * @brief Update last login timestamp.
     * 
     * Updates the last login timestamp for a user.
     * 
     * @param userId User identifier
     * @param loginTimeMs Login time in milliseconds (epoch milliseconds)
     * @return true if update succeeded, false otherwise
     */
    virtual bool updateLastLogin(const std::string& userId, int64_t loginTimeMs) = 0;
};

} // namespace Repositories
} // namespace Domain
} // namespace ZMonitor

