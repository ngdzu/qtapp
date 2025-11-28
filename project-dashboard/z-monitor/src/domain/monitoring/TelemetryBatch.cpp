/**
 * @file TelemetryBatch.cpp
 * @brief Implementation of TelemetryBatch domain aggregate.
 * 
 * @author Z Monitor Team
 * @date 2025-01-15
 */

#include "TelemetryBatch.h"
#include <random>
#include <sstream>
#include <iomanip>
#include <chrono>

namespace ZMonitor {
namespace Domain {
namespace Monitoring {

TelemetryBatch::TelemetryBatch()
    : m_batchId(generateBatchId())
    , m_deviceId("")
    , m_patientMrn("")
    , m_createdAtMs(getCurrentTimestampMs())
    , m_signedAtMs(0)
    , m_signature("")
    , m_nonce(generateNonce())
    , m_vitals()
    , m_alarms()
{
    m_vitals.reserve(MAX_VITALS_PER_BATCH);
    m_alarms.reserve(MAX_ALARMS_PER_BATCH);
}

TelemetryBatch::~TelemetryBatch() = default;

bool TelemetryBatch::addVital(const VitalRecord& vital) {
    // Business rule: Cannot add vitals after batch is signed
    if (isSigned()) {
        return false;
    }
    
    // Business rule: Check batch size limits
    if (m_vitals.size() >= MAX_VITALS_PER_BATCH) {
        return false;
    }
    
    // Business rule: All vitals must be associated with same patient (if patient admitted)
    if (!m_patientMrn.empty() && vital.patientMrn != m_patientMrn) {
        return false;
    }
    
    m_vitals.push_back(vital);
    return true;
}

bool TelemetryBatch::addAlarm(const AlarmSnapshot& alarm) {
    // Business rule: Cannot add alarms after batch is signed
    if (isSigned()) {
        return false;
    }
    
    // Business rule: Check batch size limits
    if (m_alarms.size() >= MAX_ALARMS_PER_BATCH) {
        return false;
    }
    
    m_alarms.push_back(alarm);
    return true;
}

bool TelemetryBatch::sign(const std::string& signature) {
    // Business rule: Cannot sign empty batch
    if (m_vitals.empty() && m_alarms.empty()) {
        return false;
    }
    
    // Business rule: Cannot sign twice
    if (isSigned()) {
        return false;
    }
    
    m_signature = signature;
    m_signedAtMs = getCurrentTimestampMs();
    return true;
}

bool TelemetryBatch::validate() const {
    // Batch must be signed
    if (!isSigned()) {
        return false;
    }
    
    // Batch must have valid timestamp
    if (m_createdAtMs <= 0) {
        return false;
    }
    
    // Batch must have nonce for replay prevention
    if (m_nonce.empty()) {
        return false;
    }
    
    // Batch must contain data
    if (m_vitals.empty() && m_alarms.empty()) {
        return false;
    }
    
    return true;
}

size_t TelemetryBatch::getEstimatedSizeBytes() const {
    size_t size = 0;
    
    // Batch metadata
    size += m_batchId.size();
    size += m_deviceId.size();
    size += m_patientMrn.size();
    size += m_signature.size();
    size += m_nonce.size();
    size += sizeof(m_createdAtMs);
    size += sizeof(m_signedAtMs);
    
    // Vital records (estimated)
    for (const auto& vital : m_vitals) {
        size += vital.vitalType.size();
        size += vital.patientMrn.size();
        size += vital.deviceId.size();
        size += sizeof(vital.value);
        size += sizeof(vital.timestampMs);
        size += sizeof(vital.signalQuality);
    }
    
    // Alarm snapshots (estimated)
    for (const auto& alarm : m_alarms) {
        size += alarm.alarmId.size();
        size += alarm.alarmType.size();
        size += alarm.patientMrn.size();
        size += alarm.deviceId.size();
        size += alarm.acknowledgedBy.size();
        size += sizeof(alarm.value);
        size += sizeof(alarm.thresholdValue);
        size += sizeof(alarm.timestampMs);
        size += sizeof(alarm.acknowledgedAtMs);
    }
    
    return size;
}

std::string TelemetryBatch::generateBatchId() const {
    // Simple UUID v4 generation (for domain layer, no external dependencies)
    // In production, use a proper UUID library
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(0, 15);
    std::uniform_int_distribution<> dis2(8, 11);
    
    std::stringstream ss;
    ss << std::hex;
    
    // Generate 32 hex digits
    for (int i = 0; i < 32; i++) {
        if (i == 8 || i == 12 || i == 16 || i == 20) {
            ss << "-";
        }
        int n = (i == 12) ? dis2(gen) : dis(gen);
        ss << n;
    }
    
    return ss.str();
}

std::string TelemetryBatch::generateNonce() const {
    // Generate a random nonce for replay prevention
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(0, 255);
    
    std::stringstream ss;
    ss << std::hex;
    
    // Generate 16-byte nonce (32 hex characters)
    for (int i = 0; i < 16; i++) {
        ss << std::setw(2) << std::setfill('0') << dis(gen);
    }
    
    return ss.str();
}

int64_t TelemetryBatch::getCurrentTimestampMs() const {
    auto now = std::chrono::system_clock::now();
    auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(
        now.time_since_epoch());
    return ms.count();
}

} // namespace Monitoring
} // namespace Domain
} // namespace ZMonitor

