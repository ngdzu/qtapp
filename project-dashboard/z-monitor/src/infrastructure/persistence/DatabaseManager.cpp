/**
 * @file DatabaseManager.cpp
 * @brief Implementation of DatabaseManager with QxOrm support.
 *
 * @author Z Monitor Team
 * @date 2025-01-15
 */

#include "infrastructure/persistence/DatabaseManager.h"
#include "infrastructure/persistence/QueryRegistry.h"
#include "infrastructure/persistence/orm/OrmRegistry.h"
#include "domain/common/Result.h"
#include <QSqlError>
#include <QSqlQuery>
#include <QRegularExpression>
#include <QFileInfo>
#include <QDir>

#ifdef USE_QXORM
#include <QxOrm.h>
#endif

namespace zmon
{

    DatabaseManager::DatabaseManager(QObject *parent)
        : QObject(parent), m_isOpen(false)
    {
    }

    DatabaseManager::~DatabaseManager()
    {
        close();
    }

    Result<void> DatabaseManager::open(const QString &dbPath, const QString &encryptionKey)
    {
        if (m_isOpen)
        {
            return Result<void>::error(
                Error::create(ErrorCode::InvalidArgument, "Database is already open"));
        }

        m_databasePath = dbPath;
        m_encryptionKey = encryptionKey;

        // Create database directory if it doesn't exist (skip for in-memory databases)
        if (dbPath != ":memory:" && !dbPath.startsWith("file::memory:"))
        {
            QFileInfo fileInfo(dbPath);
            QDir dir = fileInfo.absoluteDir();
            if (!dir.exists())
            {
                if (!dir.mkpath("."))
                {
                    return Result<void>::error(
                        Error::create(ErrorCode::DatabaseError,
                                      QString("Cannot create database directory: %1").arg(dir.absolutePath()).toStdString()));
                }
            }
        }

        // Create main connection
        m_mainDb = createConnection("main");
        m_mainDb.setDatabaseName(dbPath);

        // Setup encryption if key provided
        if (!encryptionKey.isEmpty())
        {
            auto result = setupEncryption(m_mainDb);
            if (result.isError())
            {
                return result;
            }
        }

        // Open main connection
        if (!m_mainDb.open())
        {
            QSqlError error = m_mainDb.lastError();
            return Result<void>::error(
                Error::create(ErrorCode::DatabaseError,
                              QString("Cannot open database: %1").arg(error.text()).toStdString()));
        }

        // Create write connection (clone of main)
        m_writeDb = createConnection("write");
        m_writeDb.setDatabaseName(dbPath);
        if (!encryptionKey.isEmpty())
        {
            auto result = setupEncryption(m_writeDb);
            if (result.isError())
            {
                m_mainDb.close();
                return result;
            }
        }
        if (!m_writeDb.open())
        {
            QSqlError error = m_writeDb.lastError();
            m_mainDb.close();
            return Result<void>::error(
                Error::create(ErrorCode::DatabaseError,
                              QString("Cannot open write connection: %1").arg(error.text()).toStdString()));
        }

        // Create read connection (clone of main)
        m_readDb = createConnection("read");
        m_readDb.setDatabaseName(dbPath);
        if (!encryptionKey.isEmpty())
        {
            auto result = setupEncryption(m_readDb);
            if (result.isError())
            {
                m_mainDb.close();
                m_writeDb.close();
                return result;
            }
        }
        if (!m_readDb.open())
        {
            QSqlError error = m_readDb.lastError();
            m_mainDb.close();
            m_writeDb.close();
            return Result<void>::error(
                Error::create(ErrorCode::DatabaseError,
                              QString("Cannot open read connection: %1").arg(error.text()).toStdString()));
        }

        // Initialize QxOrm if enabled
#ifdef USE_QXORM
        auto qxOrmResult = initializeQxOrm();
        if (qxOrmResult.isError())
        {
            // QxOrm initialization failed, but manual SQL still works
            // Emit signal so caller can log the error
            emit qxOrmInitializationFailed(QString::fromStdString(qxOrmResult.error().message));
            // Continue - database is still usable with manual SQL
        }
#endif // USE_QXORM

        // Note: Query initialization is deferred until after migrations are executed
        // Call persistence::QueryCatalog::initializeQueries(this) after executeMigrations()

        m_isOpen = true;
        emit connectionOpened();

        return Result<void>::ok();
    }

    void DatabaseManager::close()
    {
        if (!m_isOpen)
        {
            return;
        }

#ifdef USE_QXORM
        // QxOrm uses singleton pattern, no explicit cleanup needed
        // Connection cleanup is handled by QSqlDatabase::removeDatabase()
#endif // USE_QXORM

        // Close all connections
        m_readDb.close();
        m_writeDb.close();
        m_mainDb.close();

        // Remove connections
        QSqlDatabase::removeDatabase("read");
        QSqlDatabase::removeDatabase("write");
        QSqlDatabase::removeDatabase("main");

        m_isOpen = false;
        emit connectionClosed();
    }

    bool DatabaseManager::isOpen() const
    {
        return m_isOpen;
    }

    QSqlDatabase &DatabaseManager::getConnection()
    {
        return m_mainDb;
    }

    QSqlDatabase &DatabaseManager::getWriteConnection()
    {
        return m_writeDb;
    }

    QSqlDatabase &DatabaseManager::getReadConnection()
    {
        return m_readDb;
    }

    Result<void> DatabaseManager::beginTransaction()
    {
        if (!m_isOpen)
        {
            return Result<void>::error(
                Error::create(ErrorCode::DatabaseError, "Database is not open"));
        }

        QSqlQuery query(m_writeDb);
        if (!query.exec("BEGIN TRANSACTION"))
        {
            QSqlError error = query.lastError();
            QString errorMsg = QString("Cannot begin transaction: %1").arg(error.text());
            emit transactionFailed(errorMsg);
            return Result<void>::error(
                Error::create(ErrorCode::DatabaseError, errorMsg.toStdString()));
        }

        return Result<void>::ok();
    }

    Result<void> DatabaseManager::commit()
    {
        if (!m_isOpen)
        {
            return Result<void>::error(
                Error::create(ErrorCode::DatabaseError, "Database is not open"));
        }

        QSqlQuery query(m_writeDb);
        if (!query.exec("COMMIT"))
        {
            QSqlError error = query.lastError();
            QString errorMsg = QString("Cannot commit transaction: %1").arg(error.text());
            emit transactionFailed(errorMsg);
            return Result<void>::error(
                Error::create(ErrorCode::DatabaseError, errorMsg.toStdString()));
        }

        return Result<void>::ok();
    }

    Result<void> DatabaseManager::rollback()
    {
        if (!m_isOpen)
        {
            return Result<void>::error(
                Error::create(ErrorCode::DatabaseError, "Database is not open"));
        }

        QSqlQuery query(m_writeDb);
        if (!query.exec("ROLLBACK"))
        {
            QSqlError error = query.lastError();
            QString errorMsg = QString("Cannot rollback transaction: %1").arg(error.text());
            emit transactionFailed(errorMsg);
            return Result<void>::error(
                Error::create(ErrorCode::DatabaseError, errorMsg.toStdString()));
        }

        return Result<void>::ok();
    }

    /**
     * @brief Executes all pending database migrations from the migrations directory in order.
     *
     * Each migration file is executed inside a programmatic transaction.
     * Migration SQL files MUST NOT contain explicit BEGIN, COMMIT, or ROLLBACK statements.
     * Transaction commands inside migration files are ignored.
     * This ensures atomic migration execution and prevents partial schema updates.
     */
    Result<void> DatabaseManager::executeMigrations()
    {
        if (!m_isOpen)
        {
            return Result<void>::error(
                Error::create(ErrorCode::DatabaseError, "Database is not open"));
        }

        // Simple migration: Execute schema files in order
        // For production: Use a schema_version table to track which migrations have been applied
        QStringList migrations = {
            ":/schema/migrations/0001_schema.sql",
            ":/schema/migrations/0002_add_indices.sql"};

        for (const QString &migrationPath : migrations)
        {
            QFile migrationFile(migrationPath);
            if (!migrationFile.open(QIODevice::ReadOnly | QIODevice::Text))
            {
                // Migration file not found - this may be OK for embedded resources
                qWarning() << "Migration file not found:" << migrationPath << "- Skipping";
                continue;
            }

            QString sql = migrationFile.readAll();
            qInfo() << "Loaded migration" << migrationPath << "(bytes=" << sql.size() << ") first 200 chars:" << sql.left(200);
            migrationFile.close();

            // Split SQL into individual statements (QSqlQuery can't exec multiple)
            QStringList statements = sql.split(";", Qt::SkipEmptyParts);
            qInfo() << "Executing" << statements.size() << "statements from" << migrationPath;

            QSqlQuery query(m_writeDb);
            bool migrationSuccess = true;

            // Ensure foreign keys enforcement
            query.exec("PRAGMA foreign_keys = ON");

            // Begin transaction programmatically for this migration
            if (!m_writeDb.transaction())
            {
                qWarning() << "Failed to begin transaction for migration:" << migrationPath << m_writeDb.lastError().text();
                migrationSuccess = false;
            }

            // Execute each statement separately
            for (const QString &stmt : statements)
            {
                QString trimmedStmt = stmt.trimmed();

                // Skip comments and empty lines
                if (trimmedStmt.isEmpty() || trimmedStmt.startsWith("--") ||
                    trimmedStmt.contains(QRegularExpression("^\\s*--")))
                {
                    continue;
                }

                // Ignore explicit transaction commands inside files since we use programmatic transactions
                if (trimmedStmt.contains(QRegularExpression("^(BEGIN|BEGIN TRANSACTION|COMMIT|ROLLBACK)", QRegularExpression::CaseInsensitiveOption)))
                {
                    // Log once for visibility, but do not execute
                    qInfo() << "Ignoring explicit transaction statement in" << migrationPath << ":" << trimmedStmt.left(40);
                    continue;
                }

                if (!query.exec(trimmedStmt))
                {
                    QSqlError error = query.lastError();
                    // Only log error if it's not "table already exists"
                    if (!error.text().contains("already exists", Qt::CaseInsensitive))
                    {
                        qWarning() << "Migration statement failed (first 100 chars):" << trimmedStmt.left(100);
                        qWarning() << "SQL Error:" << error.text();
                        migrationSuccess = false;
                    }
                }
                else
                {
                    qInfo() << "Executed statement (first 80 chars):" << trimmedStmt.left(80);
                }
            }

            // Commit or rollback based on success
            if (migrationSuccess)
            {
                if (!m_writeDb.commit())
                {
                    qWarning() << "Failed to commit migration transaction:" << migrationPath << m_writeDb.lastError().text();
                    migrationSuccess = false;
                }
            }
            else
            {
                if (!m_writeDb.rollback())
                {
                    qWarning() << "Failed to rollback migration transaction:" << migrationPath << m_writeDb.lastError().text();
                }
            }

            if (migrationSuccess)
            {
                qInfo() << "Migration executed successfully:" << migrationPath;
            }
            else
            {
                qWarning() << "Migration completed with some errors:" << migrationPath;
            }
        }

        return Result<void>::ok();
    }

#ifdef USE_QXORM
    qx::QxSqlDatabase *DatabaseManager::getQxOrmConnection()
    {
        // Initialize QxOrm if not already initialized
        if (!m_isOpen)
        {
            // Emit error signal - database not open
            emit qxOrmInitializationFailed("Database is not open");
            // Return singleton (will fail on use, but prevents crash)
            return qx::QxSqlDatabase::getSingleton();
        }

        // Ensure QxOrm is initialized
        auto result = initializeQxOrm();
        if (result.isError())
        {
            // Emit error signal so caller can log the error
            emit qxOrmInitializationFailed(QString::fromStdString(result.error().message));
            // Return singleton anyway (will fail on use, but prevents crash)
            return qx::QxSqlDatabase::getSingleton();
        }

        // Return QxOrm singleton
        return qx::QxSqlDatabase::getSingleton();
    }

    bool DatabaseManager::isQxOrmEnabled()
    {
#ifdef USE_QXORM
        return true;
#else
        return false;
#endif
    }
#endif // USE_QXORM

    Result<void> DatabaseManager::initializeQxOrm()
    {
#ifdef USE_QXORM
        if (!m_isOpen)
        {
            return Result<void>::error(
                Error::create(ErrorCode::DatabaseError, "Database must be open before initializing QxOrm"));
        }

        // Initialize ORM registry (registers all mappings)
        persistence::OrmRegistry::initialize();

        // Configure QxOrm to use our SQLite connection
        // QxOrm uses the connection name from QSqlDatabase
        try
        {
            // Get QxOrm database singleton and configure it
            qx::QxSqlDatabase *qxDb = qx::QxSqlDatabase::getSingleton();

            // Set database connection name (QxOrm will use existing QSqlDatabase connection)
            qxDb->setDatabaseName(m_mainDb.connectionName());

            // Set database type to SQLite
            qxDb->setDriverName("QSQLITE");

            // QxOrm will use the existing QSqlDatabase connection
            // No need to call open() - connection is already open

            return Result<void>::ok();
        }
        catch (const std::exception &e)
        {
            return Result<void>::error(
                Error::create(ErrorCode::DatabaseError,
                              QString("QxOrm initialization failed: %1").arg(e.what()).toStdString()));
        }
#else
        return Result<void>::error(
            Error::create(ErrorCode::InvalidArgument, "QxOrm is not enabled (USE_QXORM not defined)"));
#endif // USE_QXORM
    }

    Result<void> DatabaseManager::setupEncryption(QSqlDatabase &db)
    {
        // SQLCipher encryption setup
        // This requires SQLCipher driver to be available
        // For now, we'll set the key using PRAGMA key

        if (m_encryptionKey.isEmpty())
        {
            return Result<void>::ok(); // No encryption requested
        }

        // Open connection first (required for PRAGMA)
        if (!db.isOpen())
        {
            if (!db.open())
            {
                QSqlError error = db.lastError();
                return Result<void>::error(
                    Error::create(ErrorCode::DatabaseError,
                                  QString("Cannot open encrypted database: %1").arg(error.text()).toStdString()));
            }
        }

        // Set encryption key using PRAGMA
        QSqlQuery query(db);
        QString pragmaSql = QString("PRAGMA key = '%1'").arg(m_encryptionKey);
        if (!query.exec(pragmaSql))
        {
            QSqlError error = query.lastError();
            return Result<void>::error(
                Error::create(ErrorCode::DatabaseError,
                              QString("Cannot set encryption key: %1").arg(error.text()).toStdString()));
        }

        // Verify encryption is working by testing a simple query
        if (!query.exec("SELECT COUNT(*) FROM sqlite_master"))
        {
            QSqlError error = query.lastError();
            return Result<void>::error(
                Error::create(ErrorCode::DatabaseError,
                              QString("Encryption key verification failed: %1").arg(error.text()).toStdString()));
        }

        return Result<void>::ok();
    }

    QSqlDatabase DatabaseManager::createConnection(const QString &connectionName)
    {
        // Ensure QSQLITE driver is available before creating named connections
        // This forces Qt to load the plugin if not already loaded
        if (!QSqlDatabase::isDriverAvailable("QSQLITE"))
        {
            qCritical() << "QSQLITE driver not available for connection:" << connectionName;
            qCritical() << "Available drivers:" << QSqlDatabase::drivers();
            return QSqlDatabase(); // Return invalid database
        }

        QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE", connectionName);
        if (!db.isValid())
        {
            qCritical() << "Failed to create database connection:" << connectionName;
        }
        return db;
    }

    Result<void> DatabaseManager::registerPreparedQuery(const QString &queryId, const QString &sql)
    {
        if (queryId.isEmpty())
        {
            return Result<void>::error(
                Error::create(ErrorCode::InvalidArgument, "Query ID cannot be empty"));
        }

        if (sql.isEmpty())
        {
            return Result<void>::error(
                Error::create(ErrorCode::InvalidArgument,
                              QString("SQL statement cannot be empty for query: %1").arg(queryId).toStdString()));
        }

        // Test that the query can be prepared (validate SQL syntax)
        QSqlQuery testQuery(m_writeDb);
        if (!testQuery.prepare(sql))
        {
            QSqlError error = testQuery.lastError();
            return Result<void>::error(
                Error::create(ErrorCode::DatabaseError,
                              QString("Failed to prepare query '%1': %2").arg(queryId, error.text()).toStdString()));
        }

        // Cache the SQL statement (queries are prepared on-demand when retrieved)
        m_querySqlCache[queryId] = sql;

        return Result<void>::ok();
    }

    QSqlQuery DatabaseManager::getPreparedQuery(const QString &queryId)
    {
        if (!m_isOpen)
        {
            qWarning() << "DatabaseManager::getPreparedQuery - database not open for query:" << queryId;
            return QSqlQuery(); // Return invalid query
        }

        if (!m_querySqlCache.contains(queryId))
        {
            // Query not registered - return invalid query
            // Error should be logged by caller
            qWarning() << "DatabaseManager::getPreparedQuery - query not registered:" << queryId;
            qWarning() << "DatabaseManager::getPreparedQuery - Available queries:" << m_querySqlCache.keys();
            return QSqlQuery();
        }

        // Verify write database is valid
        if (!m_writeDb.isValid())
        {
            qCritical() << "DatabaseManager::getPreparedQuery - write database is INVALID for query:" << queryId;
            qCritical() << "Database error:" << m_writeDb.lastError().text();
            qCritical() << "Driver name:" << m_writeDb.driverName();
            qCritical() << "Connection name:" << m_writeDb.connectionName();
            return QSqlQuery();
        }

        if (!m_writeDb.isOpen())
        {
            qCritical() << "DatabaseManager::getPreparedQuery - write database is NOT OPEN for query:" << queryId;
            return QSqlQuery();
        }

        // Create and prepare query on write connection
        QSqlQuery query(m_writeDb);
        QString sql = m_querySqlCache[queryId];

        if (!query.prepare(sql))
        {
            // Preparation failed - return invalid query
            // Error should be logged by caller
            qCritical() << "DatabaseManager::getPreparedQuery - failed to prepare query:" << queryId;
            qCritical() << "SQL:" << sql;
            qCritical() << "Error:" << query.lastError().text();
            qCritical() << "Database valid:" << query.lastQuery().isEmpty();
            return QSqlQuery();
        }

        return query;
    }

    QSqlQuery DatabaseManager::getPreparedQueryForRead(const QString &queryId)
    {
        if (!m_isOpen)
        {
            return QSqlQuery(); // Return invalid query
        }

        if (!m_querySqlCache.contains(queryId))
        {
            // Query not registered - return invalid query
            // Error should be logged by caller
            return QSqlQuery();
        }

        // Create and prepare query on read connection
        QSqlQuery query(m_readDb);
        QString sql = m_querySqlCache[queryId];

        if (!query.prepare(sql))
        {
            // Preparation failed - return invalid query
            // Error should be logged by caller
            return QSqlQuery();
        }

        return query;
    }

    bool DatabaseManager::hasQuery(const QString &queryId) const
    {
        return m_querySqlCache.contains(queryId);
    }

    QStringList DatabaseManager::getRegisteredQueries() const
    {
        return m_querySqlCache.keys();
    }

} // namespace zmon
