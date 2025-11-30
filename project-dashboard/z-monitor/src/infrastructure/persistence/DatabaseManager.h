/**
 * @file DatabaseManager.h
 * @brief Database connection manager with support for QxOrm and manual SQL.
 *
 * This file contains the DatabaseManager class which manages database connections,
 * transactions, migrations, and provides support for both QxOrm (ORM) and manual
 * SQL operations. It supports SQLCipher encryption and connection pooling.
 *
 * @author Z Monitor Team
 * @date 2025-01-15
 */

#pragma once

#include "domain/common/Result.h"
#include "IDatabaseManager.h"
#include <QObject>
#include <QString>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QMap>
#include <memory>
#include <QSqlError>

#ifdef USE_QXORM
#include <QxOrm.h>
#endif

namespace zmon
{

    /**
     * @class DatabaseManager
     * @brief Database connection manager with support for QxOrm and manual SQL.
     *
     * This class manages database connections, transactions, migrations, and provides
     * support for both QxOrm (ORM) and manual SQL operations. It supports:
     * - SQLCipher encryption
     * - Connection pooling (read/write connections)
     * - Transaction management
     * - Migration execution
     * - QxOrm connection management (when USE_QXORM is enabled)
     *
     * @note Runs on Database I/O Thread for non-blocking operations.
     * @note Supports hybrid approach: ORM for simple CRUD, manual SQL for complex queries.
     *
     * @thread Database I/O Thread
     * @ingroup Infrastructure
     */
    class DatabaseManager : public QObject, public IDatabaseManager
    {
        Q_OBJECT

    public:
        /**
         * @brief Constructor.
         *
         * @param parent Parent QObject (for Qt parent-child ownership)
         */
        explicit DatabaseManager(QObject *parent = nullptr);

        /**
         * @brief Destructor.
         */
        ~DatabaseManager() override;

        /**
         * @brief Open database connection.
         *
         * Opens a connection to the SQLite database. If encryption key is provided,
         * enables SQLCipher encryption. Initializes QxOrm connection if USE_QXORM
         * is enabled.
         *
         * @param dbPath Path to SQLite database file
         * @param encryptionKey Optional encryption key for SQLCipher (empty = no encryption)
         * @return Result<void> Success or error details if connection fails
         */
        Result<void> open(const QString &dbPath, const QString &encryptionKey = QString()) override;

        /**
         * @brief Close database connection.
         *
         * Closes all database connections and cleans up resources.
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
         * Returns the main database connection for general use.
         *
         * @return Reference to QSqlDatabase connection
         * @note Connection must be open before calling this method.
         */
        QSqlDatabase &getConnection() override;

        /**
         * @brief Get write connection (dedicated for writes).
         *
         * Returns a dedicated write connection for database writes.
         * This connection is optimized for write operations and runs
         * on the Database I/O Thread.
         *
         * @return Reference to QSqlDatabase write connection
         * @note Connection must be open before calling this method.
         */
        QSqlDatabase &getWriteConnection();

        /**
         * @brief Get read connection (can be shared).
         *
         * Returns a read-only connection for database reads.
         * This connection can be shared across multiple readers.
         *
         * @return Reference to QSqlDatabase read connection
         * @note Connection must be open before calling this method.
         */
        QSqlDatabase &getReadConnection();

        /**
         * @brief Begin database transaction.
         *
         * Starts a new database transaction. All subsequent operations
         * will be part of this transaction until commit() or rollback()
         * is called.
         *
         * @return Result<void> Success or error details if transaction fails
         */
        Result<void> beginTransaction() override;

        /**
         * @brief Commit current transaction.
         *
         * Commits the current database transaction, making all changes
         * permanent.
         *
         * @return Result<void> Success or error details if commit fails
         */
        Result<void> commit() override;

        /**
         * @brief Rollback current transaction.
         *
         * Rolls back the current database transaction, discarding all
         * changes made since beginTransaction().
         *
         * @return Result<void> Success or error details if rollback fails
         */
        Result<void> rollback() override;

        /**
         * @brief Execute database migrations.
         *
         * Executes all pending database migrations from the migrations
         * directory in order.
         *
         * @return Result<void> Success or error details if migration fails
         */
        Result<void> executeMigrations();

        /**
         * @brief Register a prepared query.
         *
         * Registers a SQL query with a unique query ID. The query is prepared
         * and cached for efficient reuse. All queries should use Schema:: constants
         * for table and column names.
         *
         * @param queryId Unique query identifier (use QueryId constants from QueryRegistry.h)
         * @param sql SQL statement with named parameters (uses Schema:: constants)
         * @return Result<void> Success or error details if query preparation fails
         *
         * @note Queries are prepared on the write connection by default.
         * @note Use QueryCatalog::initializeQueries() to register all queries at startup.
         *
         * @see QueryRegistry.h
         * @see QueryCatalog::initializeQueries()
         */
        Result<void> registerPreparedQuery(const QString &queryId, const QString &sql) override;

        /**
         * @brief Get a prepared query by ID.
         *
         * Returns a prepared QSqlQuery ready for parameter binding. The query
         * is cloned from the cached prepared query, so bindings are reset.
         *
         * @param queryId Query ID constant (from QueryId namespace, e.g., QueryId::Patient::FIND_BY_MRN)
         * @return Prepared QSqlQuery ready for parameter binding, or invalid query if not found
         *
         * @note Returns query bound to write connection by default.
         * @note Use getPreparedQueryForRead() for read-only queries.
         *
         * Example:
         * @code
         * QSqlQuery query = dbManager->getPreparedQuery(QueryId::Patient::FIND_BY_MRN);
         * query.bindValue(":mrn", "MRN-12345");
         * query.exec();
         * @endcode
         */
        QSqlQuery getPreparedQuery(const QString &queryId) override;

        /**
         * @brief Get a prepared query for read operations.
         *
         * Returns a prepared QSqlQuery bound to the read connection for read-only
         * operations. This allows read queries to use the read connection pool.
         *
         * @param queryId Query ID constant (from QueryId namespace)
         * @return Prepared QSqlQuery bound to read connection, or invalid query if not found
         */
        QSqlQuery getPreparedQueryForRead(const QString &queryId);

        /**
         * @brief Check if query is registered.
         *
         * @param queryId Query ID to check
         * @return true if registered, false otherwise
         */
        bool hasQuery(const QString &queryId) const override;

        /**
         * @brief Get all registered query IDs.
         *
         * @return List of all registered query IDs
         */
        QStringList getRegisteredQueries() const;

#ifdef USE_QXORM
        /**
         * @brief Get QxOrm database connection.
         *
         * Returns the QxOrm database connection singleton for ORM operations.
         * This connection is initialized when USE_QXORM is enabled.
         *
         * @return Pointer to QxOrm database connection singleton
         * @note Connection must be open before calling this method.
         * @note Only available when USE_QXORM is enabled.
         */
        qx::QxSqlDatabase *getQxOrmConnection();

        /**
         * @brief Check if QxOrm is enabled.
         *
         * @return true if USE_QXORM is enabled, false otherwise
         */
        static bool isQxOrmEnabled();
#endif // USE_QXORM

    signals:
        /**
         * @brief Emitted when database connection is opened.
         */
        void connectionOpened();

        /**
         * @brief Emitted when database connection is closed.
         */
        void connectionClosed();

        /**
         * @brief Emitted when a transaction fails.
         *
         * @param error Error message describing the failure
         */
        void transactionFailed(const QString &error);

        /**
         * @brief Emitted when a migration fails.
         *
         * @param error Error message describing the failure
         */
        void migrationFailed(const QString &error);

        /**
         * @brief Emitted when QxOrm initialization fails.
         *
         * This signal is emitted when QxOrm initialization fails during database open.
         * The database connection is still usable with manual SQL, but ORM operations
         * will not work. Callers should log this error.
         *
         * @param error Error message describing the failure
         */
        void qxOrmInitializationFailed(const QString &error);

    private:
        QString m_databasePath;
        QString m_encryptionKey;
        QSqlDatabase m_mainDb;                  ///< Main database connection
        QSqlDatabase m_writeDb;                 ///< Dedicated write connection
        QSqlDatabase m_readDb;                  ///< Read-only connection
        QMap<QString, QString> m_querySqlCache; ///< Map of query ID to prepared SQL statement (for caching)

#ifdef USE_QXORM
        // Note: QxOrm uses singleton pattern, so we don't need to store the connection
        // The getQxOrmConnection() method returns the singleton reference
#endif // USE_QXORM

        bool m_isOpen; ///< Whether database is open

        /**
         * @brief Initialize QxOrm connection.
         *
         * Initializes QxOrm database connection using the main database
         * connection. This must be called after opening the database.
         *
         * @return Result<void> Success or error details if initialization fails
         */
        Result<void> initializeQxOrm();

        /**
         * @brief Setup SQLCipher encryption.
         *
         * Configures SQLCipher encryption on the database connection.
         *
         * @param db Database connection to configure
         * @return Result<void> Success or error details if encryption setup fails
         */
        Result<void> setupEncryption(QSqlDatabase &db);

        /**
         * @brief Create database connection.
         *
         * Creates a new QSqlDatabase connection with the specified name.
         *
         * @param connectionName Name for the connection
         * @return QSqlDatabase connection
         */
        QSqlDatabase createConnection(const QString &connectionName);
    };

} // namespace zmon
