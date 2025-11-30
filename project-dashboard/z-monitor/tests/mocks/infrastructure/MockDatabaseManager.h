/**
 * @file MockDatabaseManager.h
 * @brief Mock implementation of DatabaseManager for testing.
 *
 * This mock provides an in-memory SQLite database for testing repository
 * implementations without requiring a real database file or Qt SQL plugins.
 *
 * @author Z Monitor Team
 * @date 2025-11-29
 */

#pragma once

#include "infrastructure/persistence/IDatabaseManager.h"
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include <QMap>
#include <QString>
#include <memory>

namespace zmon
{

    /**
     * @class MockDatabaseManager
     * @brief Mock implementation of IDatabaseManager for testing.
     *
     * This mock implementation:
     * - Uses in-memory SQLite database (":memory:")
     * - Supports basic transaction operations
     * - Provides prepared query caching
     * - Automatically creates schema on construction
     * - No encryption support (not needed for testing)
     *
     * @note Thread-safe for single-threaded tests.
     */
    class MockDatabaseManager : public QObject, public IDatabaseManager
    {
        Q_OBJECT

    public:
        /**
         * @brief Constructor.
         *
         * Creates an in-memory SQLite database and initializes the schema.
         *
         * @param parent Parent QObject (for Qt parent-child ownership)
         */
        explicit MockDatabaseManager(QObject *parent = nullptr);

        /**
         * @brief Destructor.
         */
        ~MockDatabaseManager() override;

        /**
         * @brief Open database connection (no-op for mock - auto-opens).
         *
         * @param dbPath Ignored (always uses ":memory:")
         * @param encryptionKey Ignored (no encryption in mock)
         * @return Result<void> Always returns success
         */
        Result<void> open(const QString &dbPath = QString(), const QString &encryptionKey = QString()) override;

        /**
         * @brief Close database connection.
         */
        void close() override;

        /**
         * @brief Check if database is open.
         *
         * @return true if database is open, false otherwise
         */
        bool isOpen() const override;

        /**
         * @brief Get main database connection.
         *
         * @return Reference to QSqlDatabase connection
         */
        QSqlDatabase &getConnection() override;

        /**
         * @brief Begin database transaction.
         *
         * @return Result<void> Success or error details
         */
        Result<void> beginTransaction() override;

        /**
         * @brief Commit current transaction.
         *
         * @return Result<void> Success or error details
         */
        Result<void> commit() override;

        /**
         * @brief Rollback current transaction.
         *
         * @return Result<void> Success or error details
         */
        Result<void> rollback() override;

        /**
         * @brief Register a prepared query.
         *
         * @param queryId Unique query identifier
         * @param sql SQL statement with named parameters
         * @return Result<void> Success or error details
         */
        Result<void> registerPreparedQuery(const QString &queryId, const QString &sql) override;

        /**
         * @brief Get a prepared query by ID.
         *
         * @param queryId Query ID constant
         * @return Prepared QSqlQuery ready for parameter binding
         */
        QSqlQuery getPreparedQuery(const QString &queryId) override;

        /**
         * @brief Check if query is registered.
         *
         * @param queryId Query ID to check
         * @return true if registered, false otherwise
         */
        bool hasQuery(const QString &queryId) const override;

        /**
         * @brief Initialize schema for testing.
         *
         * Creates all necessary tables for testing (vitals, patients, etc.).
         *
         * @return Result<void> Success or error details
         */
        Result<void> initializeTestSchema();

    private:
        QSqlDatabase m_db;                        ///< In-memory database connection
        QMap<QString, QString> m_preparedQueries; ///< Map of query IDs to SQL statements
        bool m_inTransaction;                     ///< Transaction state flag
        static int s_connectionCounter;           ///< Counter for unique connection names
    };

} // namespace zmon
