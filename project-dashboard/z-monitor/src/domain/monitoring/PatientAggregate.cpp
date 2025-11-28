/**
 * @file PatientAggregate.cpp
 * @brief Implementation of PatientAggregate domain aggregate.
 * 
 * @author Z Monitor Team
 * @date 2025-01-15
 */

#include "PatientAggregate.h"
#include "../admission/PatientIdentity.h"
#include "../admission/BedLocation.h"
#include <chrono>
#include <algorithm>

namespace ZMonitor {
namespace Domain {
namespace Monitoring {

PatientAggregate::PatientAggregate()
    : m_admissionState(AdmissionState::NotAdmitted)
    , m_patientIdentity()
    , m_bedLocation()
    , m_admittedAtMs(0)
    , m_dischargedAtMs(0)
    , m_admissionSource("")
    , m_transferTargetDevice("")
    , m_recentVitals()
{
    m_recentVitals.reserve(MAX_RECENT_VITALS);
}

PatientAggregate::~PatientAggregate() = default;

bool PatientAggregate::admit(const PatientIdentity& identity, 
                              const BedLocation& bedLocation,
                              const std::string& admissionSource) {
    // Business rule: Only one patient can be admitted at a time
    if (m_admissionState == AdmissionState::Admitted) {
        return false;
    }
    
    // Validate patient identity
    if (!identity.isValid()) {
        return false;
    }
    
    // Update state
    m_admissionState = AdmissionState::Admitted;
    m_patientIdentity = identity;
    m_bedLocation = bedLocation;
    m_admittedAtMs = getCurrentTimestampMs();
    m_dischargedAtMs = 0;
    m_admissionSource = admissionSource;
    m_transferTargetDevice = "";
    
    // Clear previous vitals history
    m_recentVitals.clear();
    
    // Note: Domain event PatientAdmitted would be raised here
    // (event publishing handled by application service)
    
    return true;
}

bool PatientAggregate::discharge() {
    if (m_admissionState != AdmissionState::Admitted) {
        return false;
    }
    
    // Update state
    m_admissionState = AdmissionState::Discharged;
    m_dischargedAtMs = getCurrentTimestampMs();
    
    // Note: Domain event PatientDischarged would be raised here
    // (event publishing handled by application service)
    
    return true;
}

bool PatientAggregate::transfer(const std::string& targetDevice) {
    if (m_admissionState != AdmissionState::Admitted) {
        return false;
    }
    
    if (targetDevice.empty()) {
        return false;
    }
    
    // Update state
    m_admissionState = AdmissionState::Discharged;
    m_dischargedAtMs = getCurrentTimestampMs();
    m_transferTargetDevice = targetDevice;
    
    // Note: Domain event PatientTransferred would be raised here
    // (event publishing handled by application service)
    
    return true;
}

bool PatientAggregate::updateVitals(const VitalRecord& vital) {
    // Business rule: Vitals can only be recorded if patient is admitted
    if (m_admissionState != AdmissionState::Admitted) {
        return false;
    }
    
    // Business rule: Vital must be associated with current patient MRN
    if (vital.patientMrn != m_patientIdentity.mrn) {
        return false;
    }
    
    // Add to recent vitals (maintain size limit)
    m_recentVitals.push_back(vital);
    
    // Keep only most recent N vitals
    if (m_recentVitals.size() > MAX_RECENT_VITALS) {
        // Remove oldest (first) element
        m_recentVitals.erase(m_recentVitals.begin());
    }
    
    return true;
}

std::vector<VitalRecord> PatientAggregate::getRecentVitals(size_t count) const {
    size_t returnCount = std::min(count, m_recentVitals.size());
    
    // Return most recent vitals (last N elements)
    if (returnCount == 0) {
        return {};
    }
    
    auto startIt = m_recentVitals.end() - returnCount;
    return std::vector<VitalRecord>(startIt, m_recentVitals.end());
}

int64_t PatientAggregate::getCurrentTimestampMs() const {
    auto now = std::chrono::system_clock::now();
    auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(
        now.time_since_epoch());
    return ms.count();
}

} // namespace Monitoring
} // namespace Domain
} // namespace ZMonitor

