/**
 * @file QueryRegistry.h
 * @brief Centralized registry of all database query IDs and SQL statements.
 * 
 * This file is the single source of truth for all prepared SQL queries
 * in the Z Monitor application. Query IDs are compile-time constants
 * that provide type safety and enable autocomplete.
 * 
 * @note When adding a new query:
 *       1. Add constant to QueryId namespace
 *       2. Add SQL to QueryCatalog::initializeQueries()
 *       3. Add documentation to QueryCatalog section
 * 
 * @author Z Monitor Team
 * @date 2025-01-15
 */

#pragma once

#include <QString>
#include <QMap>
#include <QStringList>

namespace zmon {
namespace persistence {

// Forward declaration
class DatabaseManager;

/**
 * @namespace QueryId
 * @brief Compile-time constants for all database query IDs.
 * 
 * Use these constants instead of string literals when requesting
 * prepared queries from DatabaseManager.
 * 
 * Example:
 * @code
 * QSqlQuery query = dbManager->getPreparedQuery(QueryId::Patient::FIND_BY_MRN);
 * @endcode
 */
namespace QueryId {
    
    /**
     * @namespace Patient
     * @brief Query IDs for patient-related operations.
     */
    namespace Patient {
        constexpr const char* FIND_BY_MRN = "patient.find_by_mrn";           ///< Find patient by Medical Record Number
        constexpr const char* FIND_ALL = "patient.find_all";                 ///< Find all patients (ordered by created_at DESC)
        constexpr const char* INSERT = "patient.insert";                      ///< Insert new patient record
        constexpr const char* UPDATE = "patient.update";                      ///< Update existing patient record
        constexpr const char* DELETE = "patient.delete";                      ///< Delete patient by MRN
        constexpr const char* CHECK_EXISTS = "patient.check_exists";          ///< Check if patient exists by MRN
        constexpr const char* GET_ADMISSION_HISTORY = "patient.get_admission_history"; ///< Get admission history for patient
    }
    
    /**
     * @namespace ActionLog
     * @brief Query IDs for action log operations.
     */
    namespace ActionLog {
        constexpr const char* INSERT = "action_log.insert";                   ///< Insert action log entry
        constexpr const char* GET_LAST_ID = "action_log.get_last_id";         ///< Get last entry ID for hash chain
        constexpr const char* GET_PREVIOUS_ENTRY = "action_log.get_previous_entry"; ///< Get previous entry for hash chain
        constexpr const char* CREATE_TABLE = "action_log.create_table";       ///< Create action_log table if not exists
        constexpr const char* CREATE_INDEX_TIMESTAMP = "action_log.create_index_timestamp"; ///< Create timestamp index
        constexpr const char* CREATE_INDEX_USER = "action_log.create_index_user"; ///< Create user index
        constexpr const char* CREATE_INDEX_ACTION_TYPE = "action_log.create_index_action_type"; ///< Create action_type index
        constexpr const char* CREATE_INDEX_TARGET = "action_log.create_index_target"; ///< Create target index
        constexpr const char* CREATE_INDEX_DEVICE = "action_log.create_index_device"; ///< Create device index
    }
}

/**
 * @class QueryCatalog
 * @brief Registry of all SQL query statements and their metadata.
 * 
 * This class maintains the mapping between query IDs and their SQL
 * statements, parameters, and documentation. All SQL queries must
 * use Schema:: constants for table and column names.
 */
class QueryCatalog {
public:
    /**
     * @struct QueryDefinition
     * @brief Structure holding query metadata.
     */
    struct QueryDefinition {
        QString id;                     ///< Query ID (from QueryId namespace)
        QString sql;                    ///< SQL statement with named parameters (uses Schema:: constants)
        QString description;            ///< Human-readable description
        QStringList parameters;         ///< List of parameter names (for documentation)
        QString exampleUsage;           ///< Code example (optional)
        bool isReadOnly;                ///< true if SELECT, false if INSERT/UPDATE/DELETE
    };
    
    /**
     * @brief Get all registered queries.
     * 
     * Returns a map of all query definitions keyed by query ID.
     * 
     * @return Map of query ID to query definition
     */
    static QMap<QString, QueryDefinition> getAllQueries();
    
    /**
     * @brief Get query definition by ID.
     * 
     * @param queryId Query ID constant (e.g., QueryId::Patient::FIND_BY_MRN)
     * @return Query definition or empty if not found
     */
    static QueryDefinition getQuery(const QString& queryId);
    
    /**
     * @brief Initialize all queries in DatabaseManager.
     * 
     * Registers all queries from the catalog with the DatabaseManager,
     * preparing them for use. This should be called once at application
     * startup after DatabaseManager is opened.
     * 
     * @param dbManager DatabaseManager instance to populate
     */
    static void initializeQueries(DatabaseManager* dbManager);
    
    /**
     * @brief Generate documentation for all queries.
     * 
     * Generates Markdown documentation listing all queries with their
     * SQL, parameters, and examples. This can be used to auto-generate
     * QUERY_REFERENCE.md.
     * 
     * @return Markdown documentation of all queries
     */
    static QString generateDocumentation();
};

} // namespace persistence
} // namespace zmon

