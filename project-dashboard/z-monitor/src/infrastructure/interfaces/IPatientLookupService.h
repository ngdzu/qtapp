/**
 * @file IPatientLookupService.h
 * @brief Interface for looking up patient information from external systems (HIS/EHR).
 *
 * This file defines the IPatientLookupService interface which provides a standardized
 * way to query external patient information systems (Hospital Information System,
 * Electronic Health Records) for patient demographics and safety information.
 *
 * @author Z Monitor Team
 * @date 2025-01-15
 */

#pragma once

#include <QObject>
#include <QString>
#include <QDate>
#include <QDateTime>
#include <QStringList>
#include <optional>
#include <functional>

namespace ZMonitor {
namespace Infrastructure {
namespace Interfaces {

/**
 * @struct PatientInfo
 * @brief Patient information structure returned by lookup service.
 */
struct PatientInfo {
    QString patientId;      ///< Primary identifier
    QString mrn;           ///< Medical Record Number
    QString name;          ///< Full name
    QDate dateOfBirth;     ///< Date of birth
    QString sex;           ///< "M", "F", or other
    QStringList allergies; ///< List of known allergies
    QString room;          ///< Current room/bed assignment
    QDateTime lastUpdated; ///< When this info was last refreshed
};

/**
 * @class IPatientLookupService
 * @brief Interface for looking up patient information from external systems.
 *
 * Provides a standardized interface for querying external patient information
 * systems (HIS/EHR) by patient ID or MRN. Supports both synchronous and
 * asynchronous lookup patterns.
 *
 * @note Thread-safe: Can be called from any thread.
 * @note Implementations should use Qt's async networking (QNetworkAccessManager).
 *
 * @ingroup Infrastructure
 */
class IPatientLookupService : public QObject {
    Q_OBJECT

public:
    /**
     * @brief Virtual destructor.
     */
    virtual ~IPatientLookupService() = default;

    /**
     * @brief Synchronous patient lookup (blocking).
     *
     * Performs a blocking lookup of patient information. Use with caution
     * as this will block the calling thread.
     *
     * @param patientId Patient identifier (MRN or patient ID)
     * @return Patient information if found, std::nullopt otherwise
     */
    virtual std::optional<PatientInfo> lookupPatient(const QString& patientId) = 0;

    /**
     * @brief Asynchronous patient lookup (preferred).
     *
     * Performs a non-blocking lookup of patient information. The callback
     * will be invoked when the lookup completes (success or failure).
     *
     * @param patientId Patient identifier (MRN or patient ID)
     * @param callback Callback function invoked with result (may be nullptr to use signals)
     */
    virtual void lookupPatientAsync(
        const QString& patientId,
        std::function<void(const std::optional<PatientInfo>&)> callback = nullptr) = 0;

    /**
     * @brief Check if service is available/configured.
     *
     * @return true if service is available and configured, false otherwise
     */
    virtual bool isAvailable() const = 0;

    /**
     * @brief Get last error message.
     *
     * Returns the error message from the most recent failed lookup operation.
     *
     * @return Error message string, or empty if no error
     */
    virtual QString getLastError() const = 0;

signals:
    /**
     * @brief Emitted when patient lookup completes successfully.
     *
     * @param patientId Patient identifier that was looked up
     * @param info Patient information
     */
    void patientLookupCompleted(const QString& patientId, const PatientInfo& info);

    /**
     * @brief Emitted when patient lookup fails.
     *
     * @param patientId Patient identifier that was looked up
     * @param error Error message describing the failure
     */
    void patientLookupFailed(const QString& patientId, const QString& error);
};

} // namespace Interfaces
} // namespace Infrastructure
} // namespace ZMonitor

