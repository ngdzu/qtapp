/**
 * @file IPatientRepository.h
 * @brief Repository interface for patient data persistence.
 * 
 * This file contains the IPatientRepository interface which defines the contract
 * for persisting and retrieving patient aggregate data. Repository interfaces are
 * defined in the domain layer to ensure business logic is independent of
 * infrastructure implementations.
 * 
 * @author Z Monitor Team
 * @date 2025-01-15
 */

#pragma once

#include "domain/admission/PatientIdentity.h"
#include "domain/admission/BedLocation.h"
#include <string>
#include <vector>
#include <memory>

namespace ZMonitor {
namespace Domain {
namespace Monitoring {
    class PatientAggregate;
}

namespace Repositories {

/**
 * @class IPatientRepository
 * @brief Repository interface for patient aggregate persistence.
 * 
 * This interface defines the contract for persisting and retrieving patient
 * aggregate data. Implementations (e.g., SQLitePatientRepository) live in the
 * infrastructure layer and provide database persistence.
 * 
 * @note Repository interfaces are defined in domain layer (no infrastructure dependencies).
 * @note Implementations are in infrastructure layer (SQLite, in-memory, etc.).
 */
class IPatientRepository {
public:
    /**
     * @brief Virtual destructor.
     */
    virtual ~IPatientRepository() = default;
    
    /**
     * @brief Find patient by MRN.
     * 
     * Retrieves patient aggregate by Medical Record Number.
     * 
     * @param mrn Medical Record Number
     * @return Shared pointer to patient aggregate, or nullptr if not found
     */
    virtual std::shared_ptr<Monitoring::PatientAggregate> findByMrn(const std::string& mrn) = 0;
    
    /**
     * @brief Save patient aggregate.
     * 
     * Persists patient aggregate to storage. Creates new record if patient
     * doesn't exist, updates existing record otherwise.
     * 
     * @param patient Patient aggregate to save
     * @return true if save succeeded, false otherwise
     */
    virtual bool save(const Monitoring::PatientAggregate& patient) = 0;
    
    /**
     * @brief Get admission history for a patient.
     * 
     * Retrieves admission/discharge/transfer history for a patient by MRN.
     * 
     * @param mrn Medical Record Number
     * @return Vector of admission event records (most recent first)
     */
    virtual std::vector<std::string> getAdmissionHistory(const std::string& mrn) = 0;
    
    /**
     * @brief Find all patients.
     * 
     * Retrieves all patient aggregates from storage.
     * 
     * @return Vector of patient aggregates
     */
    virtual std::vector<std::shared_ptr<Monitoring::PatientAggregate>> findAll() = 0;
    
    /**
     * @brief Delete patient by MRN.
     * 
     * Removes patient aggregate from storage.
     * 
     * @param mrn Medical Record Number
     * @return true if deletion succeeded, false otherwise
     */
    virtual bool remove(const std::string& mrn) = 0;
};

} // namespace Repositories
} // namespace Domain
} // namespace ZMonitor

