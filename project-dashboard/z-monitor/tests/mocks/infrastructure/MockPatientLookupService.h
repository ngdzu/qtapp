/**
 * @file MockPatientLookupService.h
 * @brief Mock implementation of IPatientLookupService for testing.
 *
 * This mock implementation returns hardcoded patient data for testing and supports
 * simulated failures.
 *
 * @author Z Monitor Team
 * @date 2025-01-15
 */

#pragma once

#include "domain/interfaces/IPatientLookupService.h"
#include <map>
#include <mutex>
#include <string>
#include <vector>

namespace zmon
{

    /**
     * @class MockPatientLookupService
     * @brief Mock implementation of IPatientLookupService for testing.
     *
     * This mock implementation:
     * - Returns hardcoded patient data for testing
     * - Supports simulated failures
     * - Tracks all lookup requests for verification
     *
     * @note Thread-safe: All methods are protected by mutex.
     */
    class MockPatientLookupService : public IPatientLookupService
    {
    public:
        /**
         * @brief Constructor.
         */
        MockPatientLookupService();

        /**
         * @brief Destructor.
         */
        ~MockPatientLookupService() override = default;

        // IPatientLookupService interface implementation
        Result<PatientAggregate> getByMrn(const std::string &mrn) const override;
        Result<std::vector<PatientIdentity>> searchByName(const std::string &name) const override;

        // Test helper methods

        /**
         * @brief Add a patient to the mock database.
         *
         * @param mrn Medical Record Number
         * @param identity Patient identity (stores PatientIdentity, not full aggregate)
         */
        void addPatient(const std::string &mrn, const PatientIdentity &identity);

        /**
         * @brief Remove a patient from the mock database.
         *
         * @param mrn Medical Record Number
         */
        void removePatient(const std::string &mrn);

        /**
         * @brief Clear all patients from the mock database.
         */
        void clear();

        /**
         * @brief Get all MRNs that were looked up.
         *
         * @return List of MRNs
         */
        std::vector<std::string> lookupHistory() const;

        /**
         * @brief Get the number of lookups performed.
         *
         * @return Number of lookups
         */
        int lookupCount() const;

        /**
         * @brief Enable simulated failures.
         *
         * When enabled, all lookup operations will fail.
         *
         * @param enabled true to enable failures, false to disable
         */
        void setSimulateFailures(bool enabled);

        /**
         * @brief Check if failures are being simulated.
         *
         * @return true if failures are enabled, false otherwise
         */
        bool isSimulatingFailures() const;

        /**
         * @brief Set the error message for simulated failures.
         *
         * @param error Error message to return on failures
         */
        void setFailureError(const std::string &error);

    private:
        mutable std::mutex m_mutex;
        std::map<std::string, PatientIdentity> m_patients; // Store PatientIdentity, not PatientAggregate
        mutable std::vector<std::string> m_lookupHistory;
        bool m_simulateFailures{false};
        std::string m_failureError{"Simulated lookup failure"};

        void initializeDefaultPatients();
    };

} // namespace zmon
