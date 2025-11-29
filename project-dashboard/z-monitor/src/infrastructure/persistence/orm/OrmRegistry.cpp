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
#endif

namespace zmon {
namespace persistence {

void OrmRegistry::initialize() {
#ifdef USE_QXORM
    // Register all ORM mappings
    // QxOrm automatically registers classes when their QX_REGISTER_HPP_EXPORT
    // macros are processed during compilation. This method ensures all
    // mapping files are included and their registrations are processed.
    
    // Include all entity headers to trigger QX_REGISTER_HPP_EXPORT macros
    // PatientEntity registration is triggered by including PatientEntity.h
    
    // Note: QxOrm uses template specialization for registration, so including
    // the headers is sufficient to trigger registration. This method serves
    // as a central point to ensure all mappings are initialized.
    
    // Future entities can be added here:
    // - UserEntity
    // - SettingsEntity
    // etc.
#endif // USE_QXORM
}

bool OrmRegistry::isEnabled() {
#ifdef USE_QXORM
    return true;
#else
    return false;
#endif
}

} // namespace persistence
} // namespace zmon

