/**
 * @file OrmRegistry.cpp
 * @brief Implementation of OrmRegistry for initializing QxOrm mappings.
 *
 * @author Z Monitor Team
 * @date 2025-01-15
 */

#include "infrastructure/persistence/orm/OrmRegistry.h"
#include "infrastructure/persistence/orm/PatientEntity.h"

#ifdef USE_QXORM
#include <QxOrm.h>
// Define QxOrm factory symbol for PatientEntity (at global scope)
// Use COMPLEX_CLASS_NAME variant with same identifier as HPP macro
QX_REGISTER_COMPLEX_CLASS_NAME_CPP(zmon::persistence::PatientEntity, zmon_persistence_PatientEntity)

// Define register_class specialization for PatientEntity
namespace qx
{
    template <>
    void register_class(QxClass<zmon::persistence::PatientEntity> &t)
    {
        using namespace Schema::Tables;
        using namespace Schema::Columns::Patients;
        t.setName(PATIENTS);
        t.id(&zmon::persistence::PatientEntity::mrn, MRN);
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
#endif

namespace zmon
{
    namespace persistence
    {

        void OrmRegistry::initialize()
        {
#ifdef USE_QXORM
            // Registration is handled by QX_REGISTER_CPP above.
            // Future entities: add their QX_REGISTER_CPP here.
#endif // USE_QXORM
        }

        bool OrmRegistry::isEnabled()
        {
#ifdef USE_QXORM
            return true;
#else
            return false;
#endif
        }

    } // namespace persistence
} // namespace zmon
