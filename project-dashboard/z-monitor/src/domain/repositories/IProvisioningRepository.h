/**
 * @file IProvisioningRepository.h
 * @brief Repository interface for provisioning data persistence.
 * 
 * This file contains the IProvisioningRepository interface which defines the contract
 * for persisting and retrieving provisioning aggregate data. Repository interfaces are
 * defined in the domain layer to ensure business logic is independent of
 * infrastructure implementations.
 * 
 * @author Z Monitor Team
 * @date 2025-01-15
 */

#pragma once

#include <string>
#include <memory>

namespace zmon {
    class DeviceAggregate;
    class ProvisioningSession;
}
/**
 * @class IProvisioningRepository
 * @brief Repository interface for provisioning aggregate persistence.
 * 
 * This interface defines the contract for persisting and retrieving provisioning
 * aggregate data. Implementations (e.g., SQLiteProvisioningRepository) live in the
 * infrastructure layer and provide database persistence.
 * 
 * @note Repository interfaces are defined in domain layer (no infrastructure dependencies).
 * @note Implementations are in infrastructure layer (SQLite, in-memory, etc.).
 */
class IProvisioningRepository {
public:
    /**
     * @brief Virtual destructor.
     */
    virtual ~IProvisioningRepository() = default;
    
    /**
     * @brief Save device aggregate.
     * 
     * Persists device aggregate to storage.
     * 
     * @param device Device aggregate to save
     * @return true if save succeeded, false otherwise
     */
    virtual bool saveDevice(const Provisioning::DeviceAggregate& device) = 0;
    
    /**
     * @brief Find device by device ID.
     * 
     * Retrieves device aggregate by device identifier.
     * 
     * @param deviceId Device identifier
     * @return Shared pointer to device aggregate, or nullptr if not found
     */
    virtual std::shared_ptr<Provisioning::DeviceAggregate> findDeviceById(const std::string& deviceId) = 0;
    
    /**
     * @brief Save provisioning session.
     * 
     * Persists provisioning session to storage.
     * 
     * @param session Provisioning session to save
     * @return true if save succeeded, false otherwise
     */
    virtual bool saveSession(const Provisioning::ProvisioningSession& session) = 0;
    
    /**
     * @brief Find provisioning session by pairing code.
     * 
     * Retrieves provisioning session by pairing code.
     * 
     * @param pairingCode Pairing code
     * @return Shared pointer to provisioning session, or nullptr if not found
     */
    virtual std::shared_ptr<Provisioning::ProvisioningSession> findSessionByCode(const std::string& pairingCode) = 0;
    
    /**
     * @brief Delete expired provisioning sessions.
     * 
     * Removes provisioning sessions that have expired.
     * 
     * @param currentTimeMs Current time in milliseconds (epoch milliseconds)
     * @return Number of sessions deleted
     */
    virtual size_t deleteExpiredSessions(int64_t currentTimeMs) = 0;
};

} // namespace zmon
} // namespace zmon