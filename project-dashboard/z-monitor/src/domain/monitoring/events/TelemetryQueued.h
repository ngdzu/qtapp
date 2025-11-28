/**
 * @file TelemetryQueued.h
 * @brief Domain event representing telemetry batch queued for transmission.
 * 
 * This file contains the TelemetryQueued domain event which is raised when
 * a telemetry batch is ready for transmission. Domain events are plain structs
 * that represent something that happened in the domain.
 * 
 * @author Z Monitor Team
 * @date 2025-01-15
 */

#pragma once

#include <string>
#include <cstdint>

namespace zmon {
namespace Events {

/**
 * @struct TelemetryQueued
 * @brief Domain event raised when a telemetry batch is queued for transmission.
 * 
 * This event is raised by MonitoringService when a telemetry batch is ready
 * for transmission. It is consumed by NetworkManager for sending the batch
 * to the central server.
 * 
 * @note Domain events are plain structs (POD) with no business logic.
 */
struct TelemetryQueued {
    /**
     * @brief Batch identifier (UUID).
     */
    std::string batchId;
    
    /**
     * @brief Device identifier.
     */
    std::string deviceId;
    
    /**
     * @brief Patient MRN (empty if no patient admitted).
     */
    std::string patientMrn;
    
    /**
     * @brief Timestamp when batch was queued.
     * 
     * Unix timestamp in milliseconds (epoch milliseconds).
     */
    int64_t timestampMs;
    
    /**
     * @brief Number of vital records in batch.
     */
    size_t vitalCount;
    
    /**
     * @brief Number of alarm snapshots in batch.
     */
    size_t alarmCount;
    
    /**
     * @brief Default constructor.
     */
    TelemetryQueued()
        : batchId("")
        , deviceId("")
        , patientMrn("")
        , timestampMs(0)
        , vitalCount(0)
        , alarmCount(0)
    {}
    
    /**
     * @brief Constructor with all parameters.
     * 
     * @param id Batch identifier
     * @param devId Device identifier
     * @param mrn Patient MRN
     * @param ts Timestamp in milliseconds
     * @param vitals Number of vital records
     * @param alarms Number of alarm snapshots
     */
    TelemetryQueued(const std::string& id, const std::string& devId,
                    const std::string& mrn, int64_t ts, size_t vitals, size_t alarms)
        : batchId(id)
        , deviceId(devId)
        , patientMrn(mrn)
        , timestampMs(ts)
        , vitalCount(vitals)
        , alarmCount(alarms)
    {}
};

} // namespace zmon
} // namespace zmon