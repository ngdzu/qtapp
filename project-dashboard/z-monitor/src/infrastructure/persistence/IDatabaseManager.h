/**
 * @file IDatabaseManager.h
 * @brief Database manager interface.
 *
 * Defines the contract for database operations including connections,
 * transactions, and prepared queries.
 *
 * @author Z Monitor Team
 * @date 2025-11-29
 */

#pragma once

#include "domain/common/Result.h"
#include <QString>
#include <QSqlDatabase>
#include <QSqlQuery>

namespace zmon
{

    /**
     * @interface IDatabaseManager
     * @brief Interface for database management operations.
     *
     * This interface defines the contract for database operations:
     * - Connection management (open, close, isOpen, getConnection)
     * - Transaction management (beginTransaction, commit, rollback)
     * - Prepared query management (registerPreparedQuery, getPreparedQuery, hasQuery)
     *
     * Implementations can provide:
     * - Real database connections (DatabaseManager)
     * - In-memory test databases (MockDatabaseManager)
     * - Other database backends
     */
    class IDatabaseManager
    {
    public:
        virtual ~IDatabaseManager() = default;

        /**
         * @brief Open database connection.
         *
         * @param dbPath Path to database file (or ":memory:" for in-memory)
         * @param encryptionKey Optional encryption key for SQLCipher
         * @return Result<void> Success or error details
         */
        virtual Result<void> open(const QString &dbPath = QString(),
                                  const QString &encryptionKey = QString()) = 0;

        /**
         * @brief Close database connection.
         */
        virtual void close() = 0;

        /**
         * @brief Check if database is open.
         *
         * @return true if database is open, false otherwise
         */
        virtual bool isOpen() const = 0;

        /**
         * @brief Get main database connection.
         *
         * @return Reference to QSqlDatabase connection
         */
        virtual QSqlDatabase &getConnection() = 0;

        /**
         * @brief Begin database transaction.
         *
         * @return Result<void> Success or error details
         */
        virtual Result<void> beginTransaction() = 0;

        /**
         * @brief Commit current transaction.
         *
         * @return Result<void> Success or error details
         */
        virtual Result<void> commit() = 0;

        /**
         * @brief Rollback current transaction.
         *
         * @return Result<void> Success or error details
         */
        virtual Result<void> rollback() = 0;

        /**
         * @brief Register a prepared query.
         *
         * @param queryId Unique query identifier
         * @param sql SQL statement with named parameters
         * @return Result<void> Success or error details
         */
        virtual Result<void> registerPreparedQuery(const QString &queryId, const QString &sql) = 0;

        /**
         * @brief Get a prepared query by ID.
         *
         * @param queryId Query ID constant
         * @return Prepared QSqlQuery ready for parameter binding
         */
        virtual QSqlQuery getPreparedQuery(const QString &queryId) = 0;

        /**
         * @brief Check if query is registered.
         *
         * @param queryId Query ID to check
         * @return true if registered, false otherwise
         */
        virtual bool hasQuery(const QString &queryId) const = 0;
    };

} // namespace zmon
