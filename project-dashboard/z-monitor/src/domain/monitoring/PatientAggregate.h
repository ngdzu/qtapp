/**
 * @file PatientAggregate.h
 * @brief Domain aggregate representing patient admission lifecycle and vitals state.
 * 
 * This file contains the PatientAggregate class which manages patient admission
 * lifecycle, vitals history, and bed assignment. It enforces business invariants
 * and raises domain events (PatientAdmitted, PatientDischarged).
 * 
 * @author Z Monitor Team
 * @date 2025-01-15
 */

#pragma once

#include "domain/admission/PatientIdentity.h"
#include "domain/admission/BedLocation.h"
#include "domain/common/Result.h"
#include "domain/monitoring/VitalRecord.h"
#include <chrono>
#include <string>
#include <vector>
#include <memory>

namespace zmon {

// Forward declarations for domain events
class PatientAdmitted;
class PatientDischarged;
class PatientTransferred;

/**
 * @enum AdmissionState
 * @brief Patient admission state.
 */
enum class AdmissionState {
    NotAdmitted,  ///< No patient currently admitted
    Admitted,     ///< Patient is currently admitted
    Discharged    ///< Patient has been discharged
};

/**
 * @class PatientAggregate
 * @brief Domain aggregate managing patient admission lifecycle and vitals state.
 * 
 * This aggregate encapsulates patient admission state, vitals history, and bed
 * assignment. It enforces business rules such as:
 * - Only one patient can be admitted at a time
 * - Vitals can only be recorded for an admitted patient
 * - Discharge clears admission state but preserves history
 * 
 * The aggregate raises domain events (PatientAdmitted, PatientDischarged,
 * PatientTransferred) which are consumed by application services and UI controllers.
 * 
 * @note This is a domain aggregate - it has identity (patient MRN) and enforces invariants.
 * @note Domain layer has no Qt dependencies - uses standard C++ only.
 */
class PatientAggregate {
public:
    /**
     * @brief Constructor.
     * 
     * Creates a new patient aggregate in NotAdmitted state.
     */
    PatientAggregate();
    
    /**
     * @brief Destructor.
     */
    ~PatientAggregate();
    
    /**
     * @brief Admit a patient to the device.
     * 
     * Transitions the aggregate from NotAdmitted to Admitted state. Associates
     * the patient identity and bed location with the device. Raises PatientAdmitted
     * domain event.
     * 
     * @param identity Patient identity (MRN, name, DOB, etc.)
     * @param bedLocation Bed/room location assignment
     * @param admissionSource Source of admission ("manual", "barcode", "central_station")
     * @return Result<void> - Success if admission succeeded, Error if patient already admitted or invalid identity
     * 
     * @note Business rule: Only one patient can be admitted at a time.
     */
    Result<void> admit(const PatientIdentity& identity, const BedLocation& bedLocation,
                       const std::string& admissionSource = "manual");
    
    /**
     * @brief Discharge the current patient.
     * 
     * Transitions the aggregate from Admitted to Discharged state. Preserves
     * patient identity and history but clears current admission. Raises
     * PatientDischarged domain event.
     * 
     * @return Result<void> - Success if discharge succeeded, Error if no patient admitted
     */
    Result<void> discharge();
    
    /**
     * @brief Transfer patient to another device.
     * 
     * Transitions the aggregate to Discharged state and records transfer target.
     * Raises PatientTransferred domain event.
     * 
     * @param targetDevice Device identifier to transfer to
     * @return Result<void> - Success if transfer succeeded, Error if no patient admitted or invalid target device
     */
    Result<void> transfer(const std::string& targetDevice);
    
    /**
     * @brief Update vitals for the current patient.
     * 
     * Records a vital sign measurement. Business rule: Vitals can only be
     * recorded if a patient is currently admitted.
     * 
     * @param vital Vital record to add
     * @return Result<void> - Success if vital was recorded, Error if no patient admitted or MRN mismatch
     * 
     * @note Business rule: Vitals require an admitted patient.
     */
    Result<void> updateVitals(const VitalRecord& vital);
    
    /**
     * @brief Get current admission state.
     * 
     * @return Current admission state
     */
    AdmissionState getAdmissionState() const { return m_admissionState; }
    
    /**
     * @brief Check if a patient is currently admitted.
     * 
     * @return true if patient is admitted, false otherwise
     */
    bool isAdmitted() const { return m_admissionState == AdmissionState::Admitted; }
    
    /**
     * @brief Get current patient identity.
     * 
     * @return Patient identity (empty if not admitted)
     */
    const PatientIdentity& getPatientIdentity() const { return m_patientIdentity; }
    
    /**
     * @brief Get current bed location.
     * 
     * @return Bed location (empty if not admitted)
     */
    const BedLocation& getBedLocation() const { return m_bedLocation; }
    
    /**
     * @brief Get admission timestamp.
     * 
     * @return Timestamp when patient was admitted (0 if not admitted)
     */
    int64_t getAdmittedAt() const { return m_admittedAtMs; }
    
    /**
     * @brief Get discharge timestamp.
     * 
     * @return Timestamp when patient was discharged (0 if not discharged)
     */
    int64_t getDischargedAt() const { return m_dischargedAtMs; }
    
    /**
     * @brief Get recent vitals history.
     * 
     * @param count Maximum number of vitals to return
     * @return Vector of recent vital records (most recent first)
     */
    std::vector<VitalRecord> getRecentVitals(size_t count = 100) const;
    
    /**
     * @brief Get patient MRN.
     * 
     * @return Patient MRN (empty string if not admitted)
     */
    std::string getPatientMrn() const { return m_patientIdentity.mrn; }

private:
    AdmissionState m_admissionState;
    PatientIdentity m_patientIdentity;
    BedLocation m_bedLocation;
    int64_t m_admittedAtMs;
    int64_t m_dischargedAtMs;
    std::string m_admissionSource;
    std::string m_transferTargetDevice;
    
    // In-memory vitals history (last N records)
    // Note: Full history is persisted via repository, this is for quick access
    std::vector<VitalRecord> m_recentVitals;
    static constexpr size_t MAX_RECENT_VITALS = 1000;
    
    /**
     * @brief Get current timestamp in milliseconds.
     * 
     * @return Current Unix timestamp in milliseconds
     */
    int64_t getCurrentTimestampMs() const;
};

} // namespace zmon