/**
 * @file MockPatientLookupService.cpp
 * @brief Implementation of MockPatientLookupService.
 *
 * @author Z Monitor Team
 * @date 2025-01-15
 */

#include "MockPatientLookupService.h"
#include <QMutexLocker>
#include <QTimer>

namespace zmon {

MockPatientLookupService::MockPatientLookupService(QObject* parent)
    : IPatientLookupService(parent)
{
    initializeDefaultPatients();
}

void MockPatientLookupService::initializeDefaultPatients()
{
    // Add some default test patients
    PatientInfo patient1;
    patient1.patientId = "P001";
    patient1.mrn = "MRN-001";
    patient1.name = "John Doe";
    patient1.dateOfBirth = QDate(1980, 1, 15);
    patient1.sex = "M";
    patient1.allergies = QStringList() << "Penicillin" << "Latex";
    patient1.room = "ICU-101";
    patient1.lastUpdated = QDateTime::currentDateTime();
    m_patients["MRN-001"] = patient1;
    m_patients["P001"] = patient1;
    
    PatientInfo patient2;
    patient2.patientId = "P002";
    patient2.mrn = "MRN-002";
    patient2.name = "Jane Smith";
    patient2.dateOfBirth = QDate(1975, 5, 20);
    patient2.sex = "F";
    patient2.allergies = QStringList() << "Aspirin";
    patient2.room = "ICU-102";
    patient2.lastUpdated = QDateTime::currentDateTime();
    m_patients["MRN-002"] = patient2;
    m_patients["P002"] = patient2;
}

std::optional<PatientInfo> MockPatientLookupService::lookupPatient(const QString& patientId)
{
    QMutexLocker locker(&m_mutex);
    m_lookupHistory.append(patientId);
    
    if (m_simulateFailures) {
        m_lastError = m_failureError;
        return std::nullopt;
    }
    
    if (!m_available) {
        m_lastError = "Service unavailable";
        return std::nullopt;
    }
    
    if (m_patients.contains(patientId)) {
        m_lastError.clear();
        return m_patients[patientId];
    }
    
    m_lastError = "Patient not found";
    return std::nullopt;
}

void MockPatientLookupService::lookupPatientAsync(
    const QString& patientId,
    std::function<void(const std::optional<PatientInfo>&)> callback)
{
    std::optional<PatientInfo> result;
    QString error;
    
    {
        QMutexLocker locker(&m_mutex);
        m_lookupHistory.append(patientId);
        
        if (m_simulateFailures) {
            m_lastError = m_failureError;
            error = m_failureError;
        } else if (!m_available) {
            m_lastError = "Service unavailable";
            error = "Service unavailable";
        } else if (m_patients.contains(patientId)) {
            m_lastError.clear();
            result = m_patients[patientId];
        } else {
            m_lastError = "Patient not found";
            error = "Patient not found";
        }
    }
    
    // Emit signal
    if (result.has_value()) {
        emit patientLookupCompleted(patientId, result.value());
    } else {
        emit patientLookupFailed(patientId, error);
    }
    
    // Call callback if provided
    if (callback) {
        callback(result);
    }
}

bool MockPatientLookupService::isAvailable() const
{
    QMutexLocker locker(&m_mutex);
    return m_available;
}

QString MockPatientLookupService::getLastError() const
{
    QMutexLocker locker(&m_mutex);
    return m_lastError;
}

void MockPatientLookupService::addPatient(const QString& patientId, const PatientInfo& info)
{
    QMutexLocker locker(&m_mutex);
    m_patients[patientId] = info;
}

void MockPatientLookupService::removePatient(const QString& patientId)
{
    QMutexLocker locker(&m_mutex);
    m_patients.remove(patientId);
}

void MockPatientLookupService::clear()
{
    QMutexLocker locker(&m_mutex);
    m_patients.clear();
    m_lookupHistory.clear();
    m_lastError.clear();
    initializeDefaultPatients();
}

QList<QString> MockPatientLookupService::lookupHistory() const
{
    QMutexLocker locker(&m_mutex);
    return m_lookupHistory;
}

int MockPatientLookupService::lookupCount() const
{
    QMutexLocker locker(&m_mutex);
    return m_lookupHistory.size();
}

void MockPatientLookupService::setSimulateFailures(bool enabled)
{
    QMutexLocker locker(&m_mutex);
    m_simulateFailures = enabled;
}

bool MockPatientLookupService::isSimulatingFailures() const
{
    QMutexLocker locker(&m_mutex);
    return m_simulateFailures;
}

void MockPatientLookupService::setFailureError(const QString& error)
{
    QMutexLocker locker(&m_mutex);
    m_failureError = error;
}

void MockPatientLookupService::setAvailable(bool available)
{
    QMutexLocker locker(&m_mutex);
    m_available = available;
}

} // namespace zmon

