/**
 * @file MockPatientLookupService.cpp
 * @brief Implementation of MockPatientLookupService.
 *
 * @author Z Monitor Team
 * @date 2025-01-15
 */

#include "MockPatientLookupService.h"
#include <algorithm>

namespace zmon
{

    MockPatientLookupService::MockPatientLookupService()
    {
        initializeDefaultPatients();
    }

    void MockPatientLookupService::initializeDefaultPatients()
    {
        // Add some default test patients with realistic data
        // Note: dateOfBirthMs is Unix timestamp in milliseconds
        m_patients.emplace("MRN-001", PatientIdentity("MRN-001", "John Doe", 315532800000, "M", {"Penicillin", "Latex"}));
        m_patients.emplace("MRN-002", PatientIdentity("MRN-002", "Jane Smith", 631152000000, "F", {}));
        m_patients.emplace("MRN-003", PatientIdentity("MRN-003", "Bob Johnson", 473385600000, "M", {"Peanuts"}));
    }

    Result<PatientAggregate> MockPatientLookupService::getByMrn(const std::string &mrn) const
    {
        std::lock_guard<std::mutex> lock(m_mutex);

        m_lookupHistory.push_back(mrn);

        if (m_simulateFailures)
        {
            return Result<PatientAggregate>::error(Error::create(ErrorCode::NotFound, "Simulated failure: " + m_failureError));
        }

        auto it = m_patients.find(mrn);
        if (it != m_patients.end())
        {
            // For mock purposes, return a default (not admitted) PatientAggregate
            // In a real lookup service, this would query external systems and construct
            // an aggregate from the retrieved data
            PatientAggregate aggregate;
            return Result<PatientAggregate>::ok(std::move(aggregate));
        }

        return Result<PatientAggregate>::error(Error::create(ErrorCode::NotFound, "Patient not found for MRN: " + mrn));
    }

    Result<std::vector<PatientIdentity>> MockPatientLookupService::searchByName(const std::string &name) const
    {
        std::lock_guard<std::mutex> lock(m_mutex);

        if (m_simulateFailures)
        {
            return Result<std::vector<PatientIdentity>>::error(Error::create(ErrorCode::NotFound, "Simulated failure: " + m_failureError));
        }

        std::vector<PatientIdentity> results;
        std::string lowerName = name;
        std::transform(lowerName.begin(), lowerName.end(), lowerName.begin(), ::tolower);

        for (const auto &[mrn, identity] : m_patients)
        {
            std::string patientName = identity.name;
            std::string lowerPatientName = patientName;
            std::transform(lowerPatientName.begin(), lowerPatientName.end(), lowerPatientName.begin(), ::tolower);

            if (lowerPatientName.find(lowerName) != std::string::npos)
            {
                results.push_back(identity);
            }
        }

        return Result<std::vector<PatientIdentity>>::ok(std::move(results));
    }

    void MockPatientLookupService::addPatient(const std::string &mrn, const PatientIdentity &identity)
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_patients.erase(mrn);             // Remove if exists
        m_patients.emplace(mrn, identity); // Copy-construct in-place
    }

    void MockPatientLookupService::removePatient(const std::string &mrn)
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_patients.erase(mrn);
    }

    void MockPatientLookupService::clear()
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_patients.clear();
        m_lookupHistory.clear();
    }

    std::vector<std::string> MockPatientLookupService::lookupHistory() const
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        return m_lookupHistory;
    }

    int MockPatientLookupService::lookupCount() const
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        return static_cast<int>(m_lookupHistory.size());
    }

    void MockPatientLookupService::setSimulateFailures(bool enabled)
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_simulateFailures = enabled;
    }

    bool MockPatientLookupService::isSimulatingFailures() const
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        return m_simulateFailures;
    }

    void MockPatientLookupService::setFailureError(const std::string &error)
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_failureError = error;
    }

} // namespace zmon
