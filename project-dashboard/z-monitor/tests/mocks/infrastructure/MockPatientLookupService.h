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

#include "infrastructure/interfaces/IPatientLookupService.h"
#include <QMutex>
#include <QMap>
#include <QString>

namespace zmon {

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
class MockPatientLookupService : public IPatientLookupService {
    Q_OBJECT

public:
    /**
     * @brief Constructor.
     */
    explicit MockPatientLookupService(QObject* parent = nullptr);

    /**
     * @brief Destructor.
     */
    ~MockPatientLookupService() override = default;

    // IPatientLookupService interface implementation
    std::optional<PatientInfo> lookupPatient(const QString& patientId) override;
    void lookupPatientAsync(
        const QString& patientId,
        std::function<void(const std::optional<PatientInfo>&)> callback = nullptr) override;
    bool isAvailable() const override;
    QString getLastError() const override;

    // Test helper methods

    /**
     * @brief Add a patient to the mock database.
     *
     * @param patientId Patient identifier (MRN or patient ID)
     * @param info Patient information
     */
    void addPatient(const QString& patientId, const PatientInfo& info);

    /**
     * @brief Remove a patient from the mock database.
     *
     * @param patientId Patient identifier
     */
    void removePatient(const QString& patientId);

    /**
     * @brief Clear all patients from the mock database.
     */
    void clear();

    /**
     * @brief Get all patient IDs that were looked up.
     *
     * @return List of patient IDs
     */
    QList<QString> lookupHistory() const;

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
    void setFailureError(const QString& error);

    /**
     * @brief Set service availability.
     *
     * @param available true to simulate available service, false for unavailable
     */
    void setAvailable(bool available);

private:
    mutable QMutex m_mutex;
    QMap<QString, PatientInfo> m_patients;
    QList<QString> m_lookupHistory;
    bool m_available{true};
    bool m_simulateFailures{false};
    QString m_failureError{"Simulated lookup failure"};
    QString m_lastError;
    
    void initializeDefaultPatients();
};

} // namespace zmon

