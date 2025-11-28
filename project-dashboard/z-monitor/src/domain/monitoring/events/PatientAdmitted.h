/**
 * @file PatientAdmitted.h
 * @brief Domain event representing patient admission.
 * 
 * This file contains the PatientAdmitted domain event which is raised when
 * a patient is admitted to the device. Domain events are plain structs that
 * represent something that happened in the domain.
 * 
 * @author Z Monitor Team
 * @date 2025-01-15
 */

#pragma once

#include "domain/admission/PatientIdentity.h"
#include "domain/admission/BedLocation.h"
#include <string>
#include <cstdint>

namespace zmon {
namespace Events {

/**
 * @struct PatientAdmitted
 * @brief Domain event raised when a patient is admitted to the device.
 * 
 * This event is raised by PatientAggregate when a patient is successfully
 * admitted. It is consumed by application services (e.g., AdmissionService)
 * for logging, UI updates, and telemetry service coordination.
 * 
 * @note Domain events are plain structs (POD) with no business logic.
 */
struct PatientAdmitted {
    /**
     * @brief Patient identity.
     */
    PatientIdentity patientIdentity;
    
    /**
     * @brief Bed location assignment.
     */
    BedLocation bedLocation;
    
    /**
     * @brief Admission source.
     * 
     * Examples: "manual", "barcode", "central_station".
     */
    std::string admissionSource;
    
    /**
     * @brief Timestamp when admission occurred.
     * 
     * Unix timestamp in milliseconds (epoch milliseconds).
     */
    int64_t timestampMs;
    
    /**
     * @brief Device identifier.
     */
    std::string deviceId;
    
    /**
     * @brief Default constructor.
     */
    PatientAdmitted()
        : patientIdentity()
        , bedLocation()
        , admissionSource("")
        , timestampMs(0)
        , deviceId("")
    {}
    
    /**
     * @brief Constructor with all parameters.
     * 
     * @param identity Patient identity
     * @param location Bed location
     * @param source Admission source
     * @param ts Timestamp in milliseconds
     * @param devId Device identifier
     */
    PatientAdmitted(const PatientIdentity& identity, const BedLocation& location,
                    const std::string& source, int64_t ts, const std::string& devId)
        : patientIdentity(identity)
        , bedLocation(location)
        , admissionSource(source)
        , timestampMs(ts)
        , deviceId(devId)
    {}
};

} // namespace zmon
} // namespace zmon