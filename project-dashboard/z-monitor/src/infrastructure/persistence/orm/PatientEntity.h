/**
 * @file PatientEntity.h
 * @brief Data Transfer Object (DTO) for patient persistence using QxOrm.
 * 
 * This file contains the PatientEntity class which is a simple DTO that maps
 * directly to the patients table in the database. It is used by QxOrm for
 * object-relational mapping. The repository layer converts between PatientEntity
 * (ORM/persistence) and PatientAggregate (domain).
 * 
 * @note This is a persistence DTO - it has no business logic.
 * @note All members are public for QxOrm mapping.
 * 
 * @author Z Monitor Team
 * @date 2025-01-15
 */

#pragma once

#include <QString>
#include <QDateTime>
#include <QStringList>
#include "infrastructure/persistence/generated/SchemaInfo.h"  // Schema constants

#ifdef USE_QXORM
#include <QxOrm.h>
#endif

namespace zmon {
namespace persistence {

/**
 * @class PatientEntity
 * @brief DTO for patient persistence using QxOrm.
 * 
 * This class represents a patient row in the patients table. It is used by
 * QxOrm for object-relational mapping. The repository layer converts between
 * PatientEntity (ORM) and PatientAggregate (domain).
 */
class PatientEntity {
public:
    /**
     * @brief Medical Record Number (primary key).
     */
    QString mrn;
    
    /**
     * @brief Patient full name.
     */
    QString name;
    
    /**
     * @brief Date of birth (ISO 8601: YYYY-MM-DD).
     */
    QString dob;
    
    /**
     * @brief Patient sex (M/F/O/U).
     */
    QString sex;
    
    /**
     * @brief Known allergies (comma-separated or JSON).
     */
    QString allergies;
    
    /**
     * @brief Current bed/room assignment.
     */
    QString bedLocation;
    
    /**
     * @brief Admission status (ADMITTED/DISCHARGED/TRANSFERRED).
     */
    QString admissionStatus;
    
    /**
     * @brief Timestamp when patient was admitted (Unix milliseconds).
     */
    qint64 admittedAt;
    
    /**
     * @brief Timestamp when patient was discharged (Unix milliseconds).
     */
    qint64 dischargedAt;
    
    /**
     * @brief Source of admission (Manual/Barcode/Central/Emergency).
     */
    QString admissionSource;
    
    /**
     * @brief Device label that admitted this patient.
     */
    QString deviceLabel;
    
    /**
     * @brief Timestamp when record was created (Unix milliseconds).
     */
    qint64 createdAt;
    
    /**
     * @brief Timestamp of last HIS lookup (Unix milliseconds).
     */
    qint64 lastLookupAt;
    
    /**
     * @brief Source of last lookup (HIS/Mock/Manual).
     */
    QString lookupSource;
    
    /**
     * @brief Legacy room/bed assignment (deprecated).
     */
    QString room;
    
    /**
     * @brief Default constructor.
     */
    PatientEntity()
        : mrn("")
        , name("")
        , dob("")
        , sex("")
        , allergies("")
        , bedLocation("")
        , admissionStatus("")
        , admittedAt(0)
        , dischargedAt(0)
        , admissionSource("")
        , deviceLabel("")
        , createdAt(0)
        , lastLookupAt(0)
        , lookupSource("")
        , room("")
    {}
    
    /**
     * @brief Copy constructor.
     */
    PatientEntity(const PatientEntity& other) = default;
    
    /**
     * @brief Assignment operator.
     */
    PatientEntity& operator=(const PatientEntity& other) = default;
};

} // namespace persistence
} // namespace zmon

#ifdef USE_QXORM
// QxOrm registration
QX_REGISTER_HPP_EXPORT(zmon::persistence::PatientEntity, qx::trait::no_base_class_defined, 0)

namespace qx {
    template<> void register_class(QxClass<zmon::persistence::PatientEntity>& t) {
        // Use schema constants (not hardcoded strings)
        using namespace Schema::Tables;
        using namespace Schema::Columns::Patients;
        
        // Table name
        t.setName(PATIENTS);
        
        // Primary key
        t.id(&zmon::persistence::PatientEntity::mrn, MRN);
        
        // Data fields - use schema constants
        t.data(&zmon::persistence::PatientEntity::name, NAME);
        t.data(&zmon::persistence::PatientEntity::dob, DOB);
        t.data(&zmon::persistence::PatientEntity::sex, SEX);
        t.data(&zmon::persistence::PatientEntity::allergies, ALLERGIES);
        t.data(&zmon::persistence::PatientEntity::bedLocation, BED_LOCATION);
        t.data(&zmon::persistence::PatientEntity::admissionStatus, ADMISSION_STATUS);
        t.data(&zmon::persistence::PatientEntity::admittedAt, ADMITTED_AT);
        t.data(&zmon::persistence::PatientEntity::dischargedAt, DISCHARGED_AT);
        t.data(&zmon::persistence::PatientEntity::admissionSource, ADMISSION_SOURCE);
        t.data(&zmon::persistence::PatientEntity::deviceLabel, DEVICE_LABEL);
        t.data(&zmon::persistence::PatientEntity::createdAt, CREATED_AT);
        t.data(&zmon::persistence::PatientEntity::lastLookupAt, LAST_LOOKUP_AT);
        t.data(&zmon::persistence::PatientEntity::lookupSource, LOOKUP_SOURCE);
        t.data(&zmon::persistence::PatientEntity::room, ROOM);
    }
}
#endif // USE_QXORM

