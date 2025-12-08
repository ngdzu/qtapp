/**
 * @file PatientAggregate.cpp
 * @brief Implementation of PatientAggregate domain aggregate.
 *
 * @author Z Monitor Team
 * @date 2025-01-15
 */

#include "domain/monitoring/PatientAggregate.h"
#include "domain/admission/PatientIdentity.h"
#include "domain/admission/BedLocation.h"
#include "domain/common/Result.h"
#include <chrono>
#include <algorithm>

namespace zmon
{
    PatientAggregate::PatientAggregate()
        : m_admissionState(AdmissionState::NotAdmitted), m_patientIdentity(), m_bedLocation(), m_admittedAtMs(0), m_dischargedAtMs(0), m_admissionSource(""), m_transferTargetDevice(""), m_recentVitals()
    {
        // Note: deque doesn't support reserve(), but it grows efficiently
    }

    PatientAggregate::~PatientAggregate() = default;

    Result<void> PatientAggregate::admit(const PatientIdentity &identity,
                                         const BedLocation &bedLocation,
                                         const std::string &admissionSource)
    {
        // Business rule: Only one patient can be admitted at a time
        if (m_admissionState == AdmissionState::Admitted)
        {
            return Result<void>::error(Error::create(
                ErrorCode::Conflict,
                "Patient already admitted",
                {{"currentMrn", m_patientIdentity.mrn}, {"newMrn", identity.mrn}}));
        }

        // Validate patient identity
        if (!identity.isValid())
        {
            return Result<void>::error(Error::create(
                ErrorCode::InvalidArgument,
                "Invalid patient identity",
                {{"mrn", identity.mrn}}));
        }

        // Update state
        m_admissionState = AdmissionState::Admitted;

        // Reconstruct value objects using placement new (since assignment is deleted due to const members)
        // CRITICAL: Must explicitly call destructor to free resources (std::string) before placement new
        m_patientIdentity.~PatientIdentity();
        new (&m_patientIdentity) PatientIdentity(identity);

        m_bedLocation.~BedLocation();
        new (&m_bedLocation) BedLocation(bedLocation);

        m_admittedAtMs = getCurrentTimestampMs();
        m_dischargedAtMs = 0;
        m_admissionSource = admissionSource;
        m_transferTargetDevice = "";

        // Clear previous vitals history
        m_recentVitals.clear();

        // Note: Domain event PatientAdmitted would be raised here
        // (event publishing handled by application service)

        return Result<void>::ok();
    }

    Result<void> PatientAggregate::discharge()
    {
        if (m_admissionState != AdmissionState::Admitted)
        {
            return Result<void>::error(Error::create(
                ErrorCode::NotFound,
                "No patient currently admitted",
                {{"currentState", std::to_string(static_cast<int>(m_admissionState))}}));
        }

        // Update state
        m_admissionState = AdmissionState::Discharged;
        m_dischargedAtMs = getCurrentTimestampMs();

        // Note: Domain event PatientDischarged would be raised here
        // (event publishing handled by application service)

        return Result<void>::ok();
    }

    Result<void> PatientAggregate::transfer(const std::string &targetDevice)
    {
        if (m_admissionState != AdmissionState::Admitted)
        {
            return Result<void>::error(Error::create(
                ErrorCode::NotFound,
                "No patient currently admitted",
                {{"currentState", std::to_string(static_cast<int>(m_admissionState))}}));
        }

        if (targetDevice.empty())
        {
            return Result<void>::error(Error::create(
                ErrorCode::InvalidArgument,
                "Target device cannot be empty",
                {{"targetDevice", targetDevice}}));
        }

        // Update state
        m_admissionState = AdmissionState::Discharged;
        m_dischargedAtMs = getCurrentTimestampMs();
        m_transferTargetDevice = targetDevice;

        // Note: Domain event PatientTransferred would be raised here
        // (event publishing handled by application service)

        return Result<void>::ok();
    }

    Result<void> PatientAggregate::updateVitals(const VitalRecord &vital)
    {
        // Business rule: Vitals can only be recorded if patient is admitted
        if (m_admissionState != AdmissionState::Admitted)
        {
            return Result<void>::error(Error::create(
                ErrorCode::NotFound,
                "No patient currently admitted",
                {{"currentState", std::to_string(static_cast<int>(m_admissionState))}, {"vitalMrn", vital.patientMrn}}));
        }

        // Business rule: Vital must be associated with current patient MRN
        if (vital.patientMrn != m_patientIdentity.mrn)
        {
            return Result<void>::error(Error::create(
                ErrorCode::Conflict,
                "Vital MRN does not match admitted patient",
                {{"admittedMrn", m_patientIdentity.mrn}, {"vitalMrn", vital.patientMrn}}));
        }

        // Add to recent vitals (maintain size limit)
        m_recentVitals.push_back(vital);

        // Keep only most recent N vitals
        if (m_recentVitals.size() > MAX_RECENT_VITALS)
        {
            // Remove oldest (first) element using pop_front (efficient for deque)
            m_recentVitals.pop_front();
        }

        return Result<void>::ok();
    }

    std::vector<VitalRecord> PatientAggregate::getRecentVitals(size_t count) const
    {
        size_t returnCount = std::min(count, m_recentVitals.size());

        // Return most recent vitals (last N elements)
        if (returnCount == 0)
        {
            return {};
        }

        auto startIt = m_recentVitals.end() - returnCount;
        return std::vector<VitalRecord>(startIt, m_recentVitals.end());
    }

    int64_t PatientAggregate::getCurrentTimestampMs() const
    {
        auto now = std::chrono::system_clock::now();
        auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(
            now.time_since_epoch());
        return ms.count();
    }

} // namespace zmon