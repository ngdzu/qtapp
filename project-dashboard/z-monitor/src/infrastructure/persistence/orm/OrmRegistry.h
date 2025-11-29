/**
 * @file OrmRegistry.h
 * @brief Registry for initializing all QxOrm mappings.
 * 
 * This file contains the OrmRegistry class which initializes all QxOrm
 * object-relational mappings at application startup. It ensures all ORM
 * mappings are registered before any database operations.
 * 
 * @author Z Monitor Team
 * @date 2025-01-15
 */

#pragma once

#ifdef USE_QXORM
#include <QxOrm.h>
#endif

namespace zmon {
namespace persistence {

/**
 * @class OrmRegistry
 * @brief Registry for initializing all QxOrm mappings.
 * 
 * This class provides a static method to initialize all ORM mappings.
 * It should be called once at application startup before any database
 * operations that use ORM.
 */
class OrmRegistry {
public:
    /**
     * @brief Initialize all ORM mappings.
     * 
     * Registers all domain aggregates with QxOrm. This must be called
     * before any ORM operations.
     * 
     * @note This method is a no-op if USE_QXORM is not defined.
     */
    static void initialize();
    
    /**
     * @brief Check if ORM is enabled.
     * 
     * @return true if USE_QXORM is defined, false otherwise
     */
    static bool isEnabled();
};

} // namespace persistence
} // namespace zmon

