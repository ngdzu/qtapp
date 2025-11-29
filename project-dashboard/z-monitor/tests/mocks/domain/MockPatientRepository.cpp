/**
 * @file MockPatientRepository.cpp
 * @brief Implementation of MockPatientRepository.
 *
 * @author Z Monitor Team
 * @date 2025-01-15
 */

#include "MockPatientRepository.h"
#include "domain/common/Result.h"
#include "domain/monitoring/PatientAggregate.h"
#include <QMutexLocker>
#include <QString>

namespace zmon {

MockPatientRepository::MockPatientRepository()
{
}

std::shared_ptr<PatientAggregate> MockPatientRepository::findByMrn(const std::string& mrn)
{
    QMutexLocker locker(&m_mutex);
    QString key = QString::fromStdString(mrn);
    if (m_patients.contains(key)) {
        return m_patients[key];
    }
    return nullptr;
}

Result<void> MockPatientRepository::save(const PatientAggregate& patient)
{
    QMutexLocker locker(&m_mutex);
    
    if (m_simulateFailures) {
        return Result<void>::error(Error::create(
            ErrorCode::DatabaseError,
            m_failureError
        ));
    }
    
    QString mrn = QString::fromStdString(patient.mrn());
    m_patients[mrn] = std::make_shared<PatientAggregate>(patient);
    
    return Result<void>::ok();
}

std::vector<std::string> MockPatientRepository::getAdmissionHistory(const std::string& mrn)
{
    QMutexLocker locker(&m_mutex);
    QString key = QString::fromStdString(mrn);
    if (m_admissionHistory.contains(key)) {
        return m_admissionHistory[key];
    }
    return std::vector<std::string>();
}

std::vector<std::shared_ptr<PatientAggregate>> MockPatientRepository::findAll()
{
    QMutexLocker locker(&m_mutex);
    std::vector<std::shared_ptr<PatientAggregate>> result;
    for (auto it = m_patients.begin(); it != m_patients.end(); ++it) {
        result.push_back(it.value());
    }
    return result;
}

Result<void> MockPatientRepository::remove(const std::string& mrn)
{
    QMutexLocker locker(&m_mutex);
    
    if (m_simulateFailures) {
        return Result<void>::error(Error::create(
            ErrorCode::DatabaseError,
            m_failureError
        ));
    }
    
    QString key = QString::fromStdString(mrn);
    if (m_patients.contains(key)) {
        m_patients.remove(key);
        m_admissionHistory.remove(key);
        return Result<void>::ok();
    }
    
    return Result<void>::error(Error::create(
        ErrorCode::NotFound,
        "Patient not found: " + mrn
    ));
}

void MockPatientRepository::clear()
{
    QMutexLocker locker(&m_mutex);
    m_patients.clear();
    m_admissionHistory.clear();
}

size_t MockPatientRepository::patientCount() const
{
    QMutexLocker locker(&m_mutex);
    return m_patients.size();
}

void MockPatientRepository::setSimulateFailures(bool enabled)
{
    QMutexLocker locker(&m_mutex);
    m_simulateFailures = enabled;
}

bool MockPatientRepository::isSimulatingFailures() const
{
    QMutexLocker locker(&m_mutex);
    return m_simulateFailures;
}

void MockPatientRepository::setFailureError(const std::string& error)
{
    QMutexLocker locker(&m_mutex);
    m_failureError = error;
}

} // namespace zmon

