/**
 * @file SQLiteActionLogRepository.h
 * @brief SQLite implementation of IActionLogRepository.
 * 
 * This file contains the SQLiteActionLogRepository class which persists action
 * log entries to the action_log table in SQLite database.
 * 
 * @author Z Monitor Team
 * @date 2025-01-15
 */

#pragma once

#include "domain/common/Result.h"
#include "domain/repositories/IActionLogRepository.h"
#include <QSqlDatabase>
#include <QQueue>
#include <QTimer>
#include <QMutex>
#include <QString>

namespace zmon {

/**
 * @class SQLiteActionLogRepository
 * @brief SQLite implementation of IActionLogRepository.
 * 
 * Persists action log entries to action_log table in SQLite database.
 * Runs on Database I/O Thread for non-blocking writes.
 * 
 * Features:
 * - Batch writes for performance (queues entries and flushes periodically)
 * - Hash chain for tamper detection (previous_hash column)
 * - Asynchronous operation (non-blocking)
 * 
 * @thread Database I/O Thread
 * @ingroup Infrastructure
 */
class SQLiteActionLogRepository : public IActionLogRepository {
    Q_OBJECT

public:
    /**
     * @brief Constructor.
     * 
     * @param databasePath Path to SQLite database file
     * @param parent Parent QObject (for Qt parent-child ownership)
     */
    explicit SQLiteActionLogRepository(const QString& databasePath, QObject* parent = nullptr);
    
    /**
     * @brief Destructor.
     */
    ~SQLiteActionLogRepository() override;

    /**
     * @brief Initialize the repository.
     * 
     * Creates the action_log table if it doesn't exist and sets up
     * the batch write timer.
     * 
     * @return Result<void> Success or error details if initialization fails
     */
    Result<void> initialize();

    /**
     * @brief Log a user action to action_log table.
     * 
     * Queues the entry for batch write. Entry is written to database
     * when the batch timer fires or when flushPendingEntries() is called.
     * 
     * @param entry Action log entry to persist
     */
    void logAction(const ActionLogEntry& entry) override;
    
    /**
     * @brief Log multiple actions in a batch (for performance).
     * 
     * Queues all entries for batch write.
     * 
     * @param entries List of action log entries
     */
    void logActions(const QList<ActionLogEntry>& entries) override;
    
    /**
     * @brief Query action log entries.
     * 
     * Executes query on background thread and emits actionsQueried()
     * signal with results.
     * 
     * @param filter Filter criteria (user_id, action_type, date range, etc.)
     */
    void queryActions(const ActionLogFilter& filter) override;

private slots:
    /**
     * @brief Flush pending entries to database.
     * 
     * Writes all queued entries to database in a single transaction.
     */
    void flushPendingEntries();

private:
    QString m_databasePath;
    QSqlDatabase m_database;
    QQueue<ActionLogEntry> m_pendingEntries;  ///< Queue for batch writes
    QTimer* m_flushTimer;                     ///< Timer for periodic batch writes
    QMutex m_queueMutex;                      ///< Mutex for thread-safe queue access
    
    /**
     * @brief Compute hash of previous entry for hash chain.
     * 
     * @param previousId ID of previous entry
     * @return SHA-256 hash of previous entry, or empty string if no previous entry
     */
    QString computePreviousHash(qint64 previousId);
    
    /**
     * @brief Create action_log table if it doesn't exist.
     * 
     * @return Result<void> Success or error details if table creation fails
     */
    Result<void> createTableIfNotExists();
    
    /**
     * @brief Get the last entry ID from database.
     * 
     * @return Last entry ID, or 0 if no entries exist
     */
    qint64 getLastEntryId();
    
    /**
     * @brief Write entries to database in a transaction.
     * 
     * @param entries List of entries to write
     * @return Result<void> Success or error details if write fails
     */
    Result<void> writeEntriesToDatabase(const QList<ActionLogEntry>& entries);
};

} // namespace zmon

