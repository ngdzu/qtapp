/**
 * @file MockPatientRepository.h
 * @brief Mock implementation of IPatientRepository for testing.
 *
 * This mock implementation stores patient aggregates in memory for testing.
 *
 * @author Z Monitor Team
 * @date 2025-01-15
 */

#pragma once

#include "domain/repositories/IPatientRepository.h"
#include <QMutex>
#include <QMap>
#include <QString>
#include <memory>
#include <vector>

namespace zmon
{
    class PatientAggregate;
}

namespace zmon
{

    /**
     * @class MockPatientRepository
     * @brief Mock implementation of IPatientRepository for testing.
     *
     * This mock implementation:
     * - Stores patient aggregates in memory
     * - Supports simulated failures
     * - Tracks all operations for verification
     *
     * @note Thread-safe: All methods are protected by mutex.
     */
    class MockPatientRepository : public IPatientRepository
    {
    public:
        /**
         * @brief Constructor.
         */
        MockPatientRepository();

        /**
         * @brief Destructor.
         */
        ~MockPatientRepository() override = default;

        // IPatientRepository interface implementation
        Result<std::shared_ptr<PatientAggregate>> findByMrn(const std::string &mrn) override;
        Result<void> save(const PatientAggregate &patient) override;
        Result<std::vector<std::string>> getAdmissionHistory(const std::string &mrn) override;
        Result<std::vector<std::shared_ptr<PatientAggregate>>> findAll() override;
        Result<void> remove(const std::string &mrn) override;

        // Test helper methods

        /**
         * @brief Clear all stored patients.
         */
        void clear();

        /**
         * @brief Get the number of patients stored.
         *
         * @return Number of patients
         */
        size_t patientCount() const;

        /**
         * @brief Enable simulated failures.
         *
         * When enabled, all save/remove operations will fail.
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
        mutable QMutex m_mutex;
        QMap<QString, std::shared_ptr<PatientAggregate>> m_patients;
        QMap<QString, std::vector<std::string>> m_admissionHistory;
        bool m_simulateFailures{false};
        std::string m_failureError{"Simulated repository failure"};
    };

} // namespace zmon
