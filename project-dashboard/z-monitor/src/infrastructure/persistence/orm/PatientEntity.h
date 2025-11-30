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
#include "infrastructure/persistence/generated/SchemaInfo.h" // Schema constants

#ifdef USE_QXORM
#include <QxOrm.h>
#endif

namespace zmon
{
    namespace persistence
    {

        /**
         * @class PatientEntity
         * @brief DTO for patient persistence using QxOrm.
         *
         * This class represents a patient row in the patients table. It is used by
         * QxOrm for object-relational mapping. The repository layer converts between
         * PatientEntity (ORM) and PatientAggregate (domain).
         */
        class PatientEntity
        {
        public:
            // QxOrm integration: declare primary key value type (always defined for template resolution)
            typedef QString type_primary_key;

#ifdef USE_QXORM
            QX_REGISTER_FRIEND_CLASS(PatientEntity)
#endif

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
                : mrn(""), name(""), dob(""), sex(""), allergies(""), bedLocation(""), admissionStatus(""), admittedAt(0), dischargedAt(0), admissionSource(""), deviceLabel(""), createdAt(0), lastLookupAt(0), lookupSource(""), room("")
            {
            }

            /**
             * @brief Copy constructor.
             */
            PatientEntity(const PatientEntity &other) = default;

            /**
             * @brief Assignment operator.
             */
            PatientEntity &operator=(const PatientEntity &other) = default;
        };

    } // namespace persistence
} // namespace zmon

#ifdef USE_QXORM
// Register primary key type for QxOrm (at global scope, after namespace closes)
QX_REGISTER_PRIMARY_KEY(zmon::persistence::PatientEntity, QString)

// Register DTO with QxOrm (at global scope with namespace-qualified name)
// Use COMPLEX_CLASS_NAME variant to provide a valid factory identifier
QX_REGISTER_COMPLEX_CLASS_NAME_HPP(zmon::persistence::PatientEntity, qx::trait::no_base_class_defined, 1, zmon_persistence_PatientEntity)

namespace qx
{
    // Forward declaration of register_class specialization
    // Implementation is in OrmRegistry.cpp to avoid duplicate symbols
    template <>
    void register_class(QxClass<zmon::persistence::PatientEntity> &t);
}
#endif // USE_QXORM
