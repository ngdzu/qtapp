/**
 * @file PatientIdentity.h
 * @brief Value object representing patient demographic information.
 * 
 * This file contains the PatientIdentity value object which represents patient
 * demographic information (MRN, name, DOB, sex, allergies). Value objects are
 * immutable and defined by their attributes.
 * 
 * @author Z Monitor Team
 * @date 2025-01-15
 */

#pragma once

#include <chrono>
#include <string>
#include <vector>

namespace zmon {
/**
 * @class PatientIdentity
 * @brief Immutable value object representing patient demographic information.
 * 
 * This value object encapsulates patient identity information including MRN,
 * name, date of birth, sex, and allergies. It is immutable and can be safely
 * shared across threads.
 * 
 * @note This is a value object - it has no identity and is defined by its attributes.
 * @note All members are const to enforce immutability.
 */
struct PatientIdentity {
    /**
     * @brief Medical Record Number (MRN).
     * 
     * Primary patient identifier in hospital systems.
     */
    const std::string mrn;
    
    /**
     * @brief Patient full name.
     */
    const std::string name;
    
    /**
     * @brief Date of birth.
     * 
     * Unix timestamp in milliseconds (epoch milliseconds).
     */
    const int64_t dateOfBirthMs;
    
    /**
     * @brief Patient sex.
     * 
     * Examples: "M", "F", "Other", "Unknown".
     */
    const std::string sex;
    
    /**
     * @brief List of known allergies.
     * 
     * Vector of allergy descriptions (e.g., ["Penicillin", "Latex"]).
     */
    const std::vector<std::string> allergies;
    
    /**
     * @brief Default constructor.
     * 
     * Creates an empty patient identity with default values.
     */
    PatientIdentity()
        : mrn("")
        , name("")
        , dateOfBirthMs(0)
        , sex("")
        , allergies()
    {}
    
    /**
     * @brief Constructor with all parameters.
     * 
     * @param patientMrn Medical Record Number
     * @param patientName Patient full name
     * @param dobMs Date of birth in milliseconds (epoch milliseconds)
     * @param patientSex Patient sex ("M", "F", "Other", "Unknown")
     * @param patientAllergies List of allergies (default: empty)
     */
    PatientIdentity(const std::string& patientMrn, 
                    const std::string& patientName,
                    int64_t dobMs,
                    const std::string& patientSex,
                    const std::vector<std::string>& patientAllergies = {})
        : mrn(patientMrn)
        , name(patientName)
        , dateOfBirthMs(dobMs)
        , sex(patientSex)
        , allergies(patientAllergies)
    {}
    
    /**
     * @brief Copy constructor.
     * 
     * @param other Source patient identity
     */
    PatientIdentity(const PatientIdentity& other) = default;
    
    /**
     * @brief Assignment operator (deleted - value objects are immutable).
     */
    PatientIdentity& operator=(const PatientIdentity&) = delete;
    
    /**
     * @brief Equality comparison.
     * 
     * Two patient identities are equal if all their attributes match.
     * 
     * @param other Other patient identity to compare
     * @return true if all attributes are equal, false otherwise
     */
    bool operator==(const PatientIdentity& other) const {
        return mrn == other.mrn &&
               name == other.name &&
               dateOfBirthMs == other.dateOfBirthMs &&
               sex == other.sex &&
               allergies == other.allergies;
    }
    
    /**
     * @brief Inequality comparison.
     * 
     * @param other Other patient identity to compare
     * @return true if any attribute differs, false otherwise
     */
    bool operator!=(const PatientIdentity& other) const {
        return !(*this == other);
    }
    
    /**
     * @brief Check if patient identity is valid (has MRN).
     * 
     * @return true if MRN is not empty, false otherwise
     */
    bool isValid() const {
        return !mrn.empty();
    }
};

} // namespace zmon
} // namespace zmon