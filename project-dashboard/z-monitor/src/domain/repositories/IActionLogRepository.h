/**
 * @file IActionLogRepository.h
 * @brief Repository interface for action log persistence.
 * 
 * This file contains the IActionLogRepository interface which defines the contract
 * for persisting and retrieving user action log entries. Repository interfaces are
 * defined in the domain layer to ensure business logic is independent of
 * infrastructure implementations.
 * 
 * @note This interface uses Qt types (QObject, QString, etc.) for asynchronous
 * communication via signals/slots. This is an exception to the domain layer's
 * no-Qt rule, as action logging requires Qt for async operations and QML integration.
 * 
 * @author Z Monitor Team
 * @date 2025-01-15
 */

#pragma once

#include <QObject>
#include <QString>
#include <QStringList>
#include <QJsonObject>
#include <QList>

namespace zmon {

/**
 * @struct ActionLogEntry
 * @brief Represents a single action log entry.
 */
struct ActionLogEntry {
    QString userId;              ///< User who performed action (empty if no login)
    QString userRole;            ///< User role (NURSE, PHYSICIAN, etc.)
    QString actionType;          ///< Action type (LOGIN, ADMIT_PATIENT, etc.)
    QString targetType;          ///< Type of target (PATIENT, SETTING, etc.)
    QString targetId;            ///< Target identifier (MRN, setting name, etc.)
    QJsonObject details;         ///< Additional context (JSON object)
    QString result;              ///< SUCCESS, FAILURE, PARTIAL
    QString errorCode;           ///< Error code if result is FAILURE
    QString errorMessage;       ///< Error message if result is FAILURE
    QString sessionTokenHash;    ///< SHA-256 hash of session token
    QString ipAddress;           ///< IP address (if available)
    QString deviceId;            ///< Device identifier
};

/**
 * @struct ActionLogFilter
 * @brief Filter criteria for querying action log entries.
 */
struct ActionLogFilter {
    QString userId;             ///< Filter by user ID (empty = all users)
    QString actionType;         ///< Filter by action type (empty = all actions)
    QString targetType;          ///< Filter by target type (empty = all targets)
    QString targetId;            ///< Filter by target ID (empty = all targets)
    qint64 startTimeMs;          ///< Start time in milliseconds (epoch milliseconds, 0 = no limit)
    qint64 endTimeMs;            ///< End time in milliseconds (epoch milliseconds, 0 = no limit)
    int limit;                   ///< Maximum number of entries to return (0 = no limit)
};

/**
 * @class IActionLogRepository
 * @brief Repository interface for persisting user actions to action_log table.
 * 
 * Provides abstraction for logging user actions (login, logout, configuration changes, etc.)
 * to the action_log table for audit and compliance purposes.
 * 
 * @note All methods are asynchronous and non-blocking
 * @note Runs on Database I/O Thread
 * 
 * @ingroup RepositoryInterfaces
 */
class IActionLogRepository : public QObject {
    Q_OBJECT

public:
    /**
     * @brief Virtual destructor.
     */
    virtual ~IActionLogRepository() = default;
    
    /**
     * @brief Log a user action to action_log table.
     * 
     * Persists action log entry to storage. Entry is queued and written
     * to database on background thread for non-blocking operation.
     * 
     * @param entry Action log entry to persist
     * 
     * @note This method is asynchronous and non-blocking
     * @note Entry is queued and written to database on background thread
     */
    virtual void logAction(const ActionLogEntry& entry) = 0;
    
    /**
     * @brief Log multiple actions in a batch (for performance).
     * 
     * Persists multiple action log entries in a single transaction.
     * More efficient than calling logAction() multiple times.
     * 
     * @param entries List of action log entries
     * 
     * @note This method is asynchronous and non-blocking
     * @note Entries are queued and written to database on background thread
     */
    virtual void logActions(const QList<ActionLogEntry>& entries) = 0;
    
    /**
     * @brief Query action log entries.
     * 
     * Retrieves action log entries matching the specified filter criteria.
     * Results are returned asynchronously via actionsQueried() signal.
     * 
     * @param filter Filter criteria (user_id, action_type, date range, etc.)
     * 
     * @note This method is asynchronous and non-blocking
     * @note Query is executed on background thread
     */
    virtual void queryActions(const ActionLogFilter& filter) = 0;

signals:
    /**
     * @brief Emitted when action is successfully logged.
     * 
     * @param entry The action log entry that was logged
     */
    void actionLogged(const ActionLogEntry& entry);
    
    /**
     * @brief Emitted when action logging fails.
     * 
     * @param entry The action log entry that failed to log
     * @param errorMessage Error message describing the failure
     */
    void actionLogFailed(const ActionLogEntry& entry, const QString& errorMessage);
    
    /**
     * @brief Emitted when action log query completes.
     * 
     * @param entries List of action log entries matching the filter
     */
    void actionsQueried(const QList<ActionLogEntry>& entries);
    
    /**
     * @brief Emitted when action log query fails.
     * 
     * @param filter The filter that was used for the query
     * @param errorMessage Error message describing the failure
     */
    void queryFailed(const ActionLogFilter& filter, const QString& errorMessage);
};

} // namespace zmon

