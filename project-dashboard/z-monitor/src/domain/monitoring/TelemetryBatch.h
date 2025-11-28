/**
 * @file TelemetryBatch.h
 * @brief Domain aggregate representing a telemetry batch for transmission.
 * 
 * This file contains the TelemetryBatch class which aggregates VitalRecord and
 * AlarmSnapshot collections, enforces signing/timestamping rules, and validates
 * batch integrity before transmission.
 * 
 * @author Z Monitor Team
 * @date 2025-01-15
 */

#pragma once

#include "monitoring/VitalRecord.h"
#include "monitoring/AlarmSnapshot.h"
#include <chrono>
#include <string>
#include <vector>
#include <cstdint>

namespace ZMonitor {
namespace Domain {
namespace Monitoring {

/**
 * @class TelemetryBatch
 * @brief Domain aggregate managing telemetry data collection and batch preparation.
 * 
 * This aggregate collects vital records and alarm snapshots into a batch for
 * transmission to the central server. It enforces business rules such as:
 * - All vitals in batch must be associated with the same patient (if patient admitted)
 * - Batch must be signed before transmission
 * - Batch must include timestamp and nonce for replay prevention
 * - Batch size limits for efficient transmission
 * 
 * @note This is a domain aggregate - it has identity (batch ID) and enforces invariants.
 * @note Domain layer has no Qt dependencies - uses standard C++ only.
 */
class TelemetryBatch {
public:
    /**
     * @brief Constructor.
     * 
     * Creates a new telemetry batch with a unique batch ID and current timestamp.
     */
    TelemetryBatch();
    
    /**
     * @brief Destructor.
     */
    ~TelemetryBatch();
    
    /**
     * @brief Add a vital record to the batch.
     * 
     * @param vital Vital record to add
     * @return true if vital was added, false if batch is full or signed
     * 
     * @note Business rule: Cannot add vitals after batch is signed.
     */
    bool addVital(const VitalRecord& vital);
    
    /**
     * @brief Add an alarm snapshot to the batch.
     * 
     * @param alarm Alarm snapshot to add
     * @return true if alarm was added, false if batch is full or signed
     * 
     * @note Business rule: Cannot add alarms after batch is signed.
     */
    bool addAlarm(const AlarmSnapshot& alarm);
    
    /**
     * @brief Sign the batch.
     * 
     * Marks the batch as signed and ready for transmission. After signing,
     * no more vitals or alarms can be added.
     * 
     * @param signature Digital signature (HMAC-SHA256 or ECDSA)
     * @return true if signing succeeded, false if batch is empty or already signed
     */
    bool sign(const std::string& signature);
    
    /**
     * @brief Validate batch integrity.
     * 
     * Checks that batch is properly signed, has valid timestamp, and contains data.
     * 
     * @return true if batch is valid, false otherwise
     */
    bool validate() const;
    
    /**
     * @brief Get batch ID.
     * 
     * @return Unique batch identifier (UUID)
     */
    const std::string& getBatchId() const { return m_batchId; }
    
    /**
     * @brief Get device identifier.
     * 
     * @return Device ID associated with this batch
     */
    const std::string& getDeviceId() const { return m_deviceId; }
    
    /**
     * @brief Set device identifier.
     * 
     * @param deviceId Device ID
     */
    void setDeviceId(const std::string& deviceId) { m_deviceId = deviceId; }
    
    /**
     * @brief Get patient MRN.
     * 
     * @return Patient MRN (empty if no patient admitted)
     */
    const std::string& getPatientMrn() const { return m_patientMrn; }
    
    /**
     * @brief Set patient MRN.
     * 
     * @param mrn Patient MRN
     */
    void setPatientMrn(const std::string& mrn) { m_patientMrn = mrn; }
    
    /**
     * @brief Get batch creation timestamp.
     * 
     * @return Timestamp when batch was created (epoch milliseconds)
     */
    int64_t getCreatedAt() const { return m_createdAtMs; }
    
    /**
     * @brief Get batch signing timestamp.
     * 
     * @return Timestamp when batch was signed (0 if not signed)
     */
    int64_t getSignedAt() const { return m_signedAtMs; }
    
    /**
     * @brief Get digital signature.
     * 
     * @return Digital signature (empty if not signed)
     */
    const std::string& getSignature() const { return m_signature; }
    
    /**
     * @brief Get nonce for replay prevention.
     * 
     * @return Nonce value
     */
    const std::string& getNonce() const { return m_nonce; }
    
    /**
     * @brief Get vital records in batch.
     * 
     * @return Vector of vital records
     */
    const std::vector<VitalRecord>& getVitals() const { return m_vitals; }
    
    /**
     * @brief Get alarm snapshots in batch.
     * 
     * @return Vector of alarm snapshots
     */
    const std::vector<AlarmSnapshot>& getAlarms() const { return m_alarms; }
    
    /**
     * @brief Check if batch is signed.
     * 
     * @return true if batch is signed, false otherwise
     */
    bool isSigned() const { return !m_signature.empty(); }
    
    /**
     * @brief Get batch size in bytes (estimated).
     * 
     * @return Estimated batch size in bytes
     */
    size_t getEstimatedSizeBytes() const;

private:
    std::string m_batchId;  // UUID
    std::string m_deviceId;
    std::string m_patientMrn;
    int64_t m_createdAtMs;
    int64_t m_signedAtMs;
    std::string m_signature;
    std::string m_nonce;  // For replay prevention
    
    std::vector<VitalRecord> m_vitals;
    std::vector<AlarmSnapshot> m_alarms;
    
    static constexpr size_t MAX_BATCH_SIZE_BYTES = 64 * 1024;  // 64 KB
    static constexpr size_t MAX_VITALS_PER_BATCH = 1000;
    static constexpr size_t MAX_ALARMS_PER_BATCH = 100;
    
    /**
     * @brief Generate a unique batch ID (UUID v4).
     * 
     * @return UUID string
     */
    std::string generateBatchId() const;
    
    /**
     * @brief Generate a nonce for replay prevention.
     * 
     * @return Nonce string
     */
    std::string generateNonce() const;
    
    /**
     * @brief Get current timestamp in milliseconds.
     * 
     * @return Current Unix timestamp in milliseconds
     */
    int64_t getCurrentTimestampMs() const;
};

} // namespace Monitoring
} // namespace Domain
} // namespace ZMonitor

