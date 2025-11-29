/**
 * @file VitalRecord.h
 * @brief Value object representing a single vital sign measurement.
 * 
 * This file contains the VitalRecord value object which represents a single
 * vital sign measurement (heart rate, SpO2, respiration rate, etc.) with
 * timestamp and metadata. Value objects are immutable and defined by their
 * attributes.
 * 
 * @author Z Monitor Team
 * @date 2025-01-15
 */

#pragma once

#include <chrono>
#include <cstdint>
#include <string>

namespace zmon {
/**
 * @class VitalRecord
 * @brief Immutable value object representing a single vital sign measurement.
 * 
 * This value object encapsulates a vital sign measurement with its timestamp,
 * value, and metadata. It is immutable and can be safely shared across threads.
 * 
 * @note This is a value object - it has no identity and is defined by its attributes.
 * @note All members are const to enforce immutability.
 */
struct VitalRecord {
    /**
     * @brief Vital sign type identifier.
     * 
     * Examples: "HR" (heart rate), "SPO2" (oxygen saturation), "RR" (respiration rate).
     */
    const std::string vitalType;
    
    /**
     * @brief Measured value.
     * 
     * The numeric value of the vital sign (e.g., 72 for heart rate in BPM).
     */
    const double value;
    
    /**
     * @brief Timestamp when the measurement was taken.
     * 
     * Unix timestamp in milliseconds (epoch milliseconds).
     */
    const int64_t timestampMs;
    
    /**
     * @brief Signal quality indicator (0-100).
     * 
     * Optional signal quality metric. Higher values indicate better signal quality.
     * 0 = no signal, 100 = excellent signal quality.
     */
    const int signalQuality;
    
    /**
     * @brief Patient MRN (Medical Record Number).
     * 
     * The patient identifier associated with this vital sign measurement.
     * Empty string if no patient is admitted.
     */
    const std::string patientMrn;
    
    /**
     * @brief Device identifier.
     * 
     * The device that captured this measurement.
     */
    const std::string deviceId;
    
    /**
     * @brief Default constructor.
     * 
     * Creates an empty vital record with default values.
     */
    VitalRecord()
        : vitalType("")
        , value(0.0)
        , timestampMs(0)
        , signalQuality(0)
        , patientMrn("")
        , deviceId("")
    {}
    
    /**
     * @brief Constructor with all parameters.
     * 
     * @param type Vital sign type (e.g., "HR", "SPO2", "RR")
     * @param val Measured value
     * @param ts Timestamp in milliseconds (epoch milliseconds)
     * @param quality Signal quality (0-100, default: 100)
     * @param mrn Patient MRN (default: empty string)
     * @param devId Device identifier (default: empty string)
     */
    VitalRecord(const std::string& type, double val, int64_t ts, 
                int quality = 100, const std::string& mrn = "", 
                const std::string& devId = "")
        : vitalType(type)
        , value(val)
        , timestampMs(ts)
        , signalQuality(quality)
        , patientMrn(mrn)
        , deviceId(devId)
    {}
    
    /**
     * @brief Copy constructor.
     * 
     * @param other Source vital record
     */
    VitalRecord(const VitalRecord& other) = default;
    
    /**
     * @brief Assignment operator (deleted - value objects are immutable).
     */
    VitalRecord& operator=(const VitalRecord&) = delete;
    
    /**
     * @brief Equality comparison.
     * 
     * Two vital records are equal if all their attributes match.
     * 
     * @param other Other vital record to compare
     * @return true if all attributes are equal, false otherwise
     */
    bool operator==(const VitalRecord& other) const {
        return vitalType == other.vitalType &&
               value == other.value &&
               timestampMs == other.timestampMs &&
               signalQuality == other.signalQuality &&
               patientMrn == other.patientMrn &&
               deviceId == other.deviceId;
    }
    
    /**
     * @brief Inequality comparison.
     * 
     * @param other Other vital record to compare
     * @return true if any attribute differs, false otherwise
     */
    bool operator!=(const VitalRecord& other) const {
        return !(*this == other);
    }
};

} // namespace zmon