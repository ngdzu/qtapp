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
#include <QFileInfo>
#include <QDir>

#ifdef USE_QXORM
#include <QxOrm.h>
#endif

namespace zmon {

DatabaseManager::DatabaseManager(QObject* parent)
    : QObject(parent)
    , m_isOpen(false)
{
}

DatabaseManager::~DatabaseManager() {
    close();
}

Result<void> DatabaseManager::open(const QString& dbPath, const QString& encryptionKey) {
    if (m_isOpen) {
        return Result<void>::error(
            Error::create(ErrorCode::InvalidArgument, "Database is already open")
        );
    }
    
    m_databasePath = dbPath;
    m_encryptionKey = encryptionKey;
    
    // Create database directory if it doesn't exist (skip for in-memory databases)
    if (dbPath != ":memory:" && !dbPath.startsWith("file::memory:")) {
        QFileInfo fileInfo(dbPath);
        QDir dir = fileInfo.absoluteDir();
        if (!dir.exists()) {
            if (!dir.mkpath(".")) {
                return Result<void>::error(
                    Error::create(ErrorCode::DatabaseError, 
                        QString("Cannot create database directory: %1").arg(dir.absolutePath()).toStdString())
                );
            }
        }
    }
    
    // Create main connection
    m_mainDb = createConnection("main");
    m_mainDb.setDatabaseName(dbPath);
    
    // Setup encryption if key provided
    if (!encryptionKey.isEmpty()) {
        auto result = setupEncryption(m_mainDb);
        if (result.isError()) {
            return result;
        }
    }
    
    // Open main connection
    if (!m_mainDb.open()) {
        QSqlError error = m_mainDb.lastError();
        return Result<void>::error(
            Error::create(ErrorCode::DatabaseError,
                QString("Cannot open database: %1").arg(error.text()).toStdString())
        );
    }
    
    // Create write connection (clone of main)
    m_writeDb = createConnection("write");
    m_writeDb.setDatabaseName(dbPath);
    if (!encryptionKey.isEmpty()) {
        auto result = setupEncryption(m_writeDb);
        if (result.isError()) {
            m_mainDb.close();
            return result;
        }
    }
    if (!m_writeDb.open()) {
        QSqlError error = m_writeDb.lastError();
        m_mainDb.close();
        return Result<void>::error(
            Error::create(ErrorCode::DatabaseError,
                QString("Cannot open write connection: %1").arg(error.text()).toStdString())
        );
    }
    
    // Create read connection (clone of main)
    m_readDb = createConnection("read");
    m_readDb.setDatabaseName(dbPath);
    if (!encryptionKey.isEmpty()) {
        auto result = setupEncryption(m_readDb);
        if (result.isError()) {
            m_mainDb.close();
            m_writeDb.close();
            return result;
        }
    }
    if (!m_readDb.open()) {
        QSqlError error = m_readDb.lastError();
        m_mainDb.close();
        m_writeDb.close();
        return Result<void>::error(
            Error::create(ErrorCode::DatabaseError,
                QString("Cannot open read connection: %1").arg(error.text()).toStdString())
        );
    }
    
    // Initialize QxOrm if enabled
#ifdef USE_QXORM
    auto qxOrmResult = initializeQxOrm();
    if (qxOrmResult.isError()) {
        // QxOrm initialization failed, but manual SQL still works
        // Emit signal so caller can log the error
        emit qxOrmInitializationFailed(QString::fromStdString(qxOrmResult.error().message));
        // Continue - database is still usable with manual SQL
    }
#endif // USE_QXORM
    
    // Initialize all queries from QueryCatalog
    QueryCatalog::initializeQueries(this);
    
    m_isOpen = true;
    emit connectionOpened();
    
    return Result<void>::ok();
}

void DatabaseManager::close() {
    if (!m_isOpen) {
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

bool DatabaseManager::isOpen() const {
    return m_isOpen;
}

QSqlDatabase& DatabaseManager::getConnection() {
    return m_mainDb;
}

QSqlDatabase& DatabaseManager::getWriteConnection() {
    return m_writeDb;
}

QSqlDatabase& DatabaseManager::getReadConnection() {
    return m_readDb;
}

Result<void> DatabaseManager::beginTransaction() {
    if (!m_isOpen) {
        return Result<void>::error(
            Error::create(ErrorCode::DatabaseError, "Database is not open")
        );
    }
    
    QSqlQuery query(m_writeDb);
    if (!query.exec("BEGIN TRANSACTION")) {
        QSqlError error = query.lastError();
        QString errorMsg = QString("Cannot begin transaction: %1").arg(error.text());
        emit transactionFailed(errorMsg);
        return Result<void>::error(
            Error::create(ErrorCode::DatabaseError, errorMsg.toStdString())
        );
    }
    
    return Result<void>::ok();
}

Result<void> DatabaseManager::commit() {
    if (!m_isOpen) {
        return Result<void>::error(
            Error::create(ErrorCode::DatabaseError, "Database is not open")
        );
    }
    
    QSqlQuery query(m_writeDb);
    if (!query.exec("COMMIT")) {
        QSqlError error = query.lastError();
        QString errorMsg = QString("Cannot commit transaction: %1").arg(error.text());
        emit transactionFailed(errorMsg);
        return Result<void>::error(
            Error::create(ErrorCode::DatabaseError, errorMsg.toStdString())
        );
    }
    
    return Result<void>::ok();
}

Result<void> DatabaseManager::rollback() {
    if (!m_isOpen) {
        return Result<void>::error(
            Error::create(ErrorCode::DatabaseError, "Database is not open")
        );
    }
    
    QSqlQuery query(m_writeDb);
    if (!query.exec("ROLLBACK")) {
        QSqlError error = query.lastError();
        QString errorMsg = QString("Cannot rollback transaction: %1").arg(error.text());
        emit transactionFailed(errorMsg);
        return Result<void>::error(
            Error::create(ErrorCode::DatabaseError, errorMsg.toStdString())
        );
    }
    
    return Result<void>::ok();
}

Result<void> DatabaseManager::executeMigrations() {
    if (!m_isOpen) {
        return Result<void>::error(
            Error::create(ErrorCode::DatabaseError, "Database is not open")
        );
    }
    
    // TODO: Implement migration execution
    // This should:
    // 1. Check schema_version table for current version
    // 2. Find all migration files in schema/migrations/
    // 3. Execute migrations in order
    // 4. Update schema_version table
    
    // For now, return success (migrations will be implemented separately)
    return Result<void>::ok();
}

#ifdef USE_QXORM
qx::QxSqlDatabase* DatabaseManager::getQxOrmConnection() {
    // Initialize QxOrm if not already initialized
    if (!m_isOpen) {
        // Emit error signal - database not open
        emit qxOrmInitializationFailed("Database is not open");
        // Return singleton (will fail on use, but prevents crash)
        return qx::QxSqlDatabase::getSingleton();
    }
    
    // Ensure QxOrm is initialized
    auto result = initializeQxOrm();
    if (result.isError()) {
        // Emit error signal so caller can log the error
        emit qxOrmInitializationFailed(QString::fromStdString(result.error().message));
        // Return singleton anyway (will fail on use, but prevents crash)
        return qx::QxSqlDatabase::getSingleton();
    }
    
    // Return QxOrm singleton
    return qx::QxSqlDatabase::getSingleton();
}

bool DatabaseManager::isQxOrmEnabled() {
#ifdef USE_QXORM
    return true;
#else
    return false;
#endif
}
#endif // USE_QXORM

Result<void> DatabaseManager::initializeQxOrm() {
#ifdef USE_QXORM
    if (!m_isOpen) {
        return Result<void>::error(
            Error::create(ErrorCode::DatabaseError, "Database must be open before initializing QxOrm")
        );
    }
    
    // Initialize ORM registry (registers all mappings)
    persistence::OrmRegistry::initialize();
    
    // Initialize QxOrm library
    qx::QxOrm::init();
    
    // Configure QxOrm to use our SQLite connection
    // QxOrm uses the connection name from QSqlDatabase
    try {
        // Get QxOrm database singleton and configure it
        qx::QxSqlDatabase* qxDb = qx::QxSqlDatabase::getSingleton();
        
        // Set database connection name (QxOrm will use existing QSqlDatabase connection)
        qxDb->setDatabaseName(m_mainDb.connectionName());
        
        // Set database type to SQLite
        qxDb->setDriverName("QSQLITE");
        
        // QxOrm will use the existing QSqlDatabase connection
        // No need to call open() - connection is already open
        
        return Result<void>::ok();
    } catch (const std::exception& e) {
        return Result<void>::error(
            Error::create(ErrorCode::DatabaseError,
                QString("QxOrm initialization failed: %1").arg(e.what()).toStdString())
        );
    }
#else
    return Result<void>::error(
        Error::create(ErrorCode::InvalidArgument, "QxOrm is not enabled (USE_QXORM not defined)")
    );
#endif // USE_QXORM
}

Result<void> DatabaseManager::setupEncryption(QSqlDatabase& db) {
    // SQLCipher encryption setup
    // This requires SQLCipher driver to be available
    // For now, we'll set the key using PRAGMA key
    
    if (m_encryptionKey.isEmpty()) {
        return Result<void>::ok();  // No encryption requested
    }
    
    // Open connection first (required for PRAGMA)
    if (!db.isOpen()) {
        if (!db.open()) {
            QSqlError error = db.lastError();
            return Result<void>::error(
                Error::create(ErrorCode::DatabaseError,
                    QString("Cannot open encrypted database: %1").arg(error.text()).toStdString())
            );
        }
    }
    
    // Set encryption key using PRAGMA
    QSqlQuery query(db);
    QString pragmaSql = QString("PRAGMA key = '%1'").arg(m_encryptionKey);
    if (!query.exec(pragmaSql)) {
        QSqlError error = query.lastError();
        return Result<void>::error(
            Error::create(ErrorCode::DatabaseError,
                QString("Cannot set encryption key: %1").arg(error.text()).toStdString())
        );
    }
    
    // Verify encryption is working by testing a simple query
    if (!query.exec("SELECT COUNT(*) FROM sqlite_master")) {
        QSqlError error = query.lastError();
        return Result<void>::error(
            Error::create(ErrorCode::DatabaseError,
                QString("Encryption key verification failed: %1").arg(error.text()).toStdString())
        );
    }
    
    return Result<void>::ok();
}

QSqlDatabase DatabaseManager::createConnection(const QString& connectionName) {
    QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE", connectionName);
    return db;
}

Result<void> DatabaseManager::registerPreparedQuery(const QString& queryId, const QString& sql) {
    if (queryId.isEmpty()) {
        return Result<void>::error(
            Error::create(ErrorCode::InvalidArgument, "Query ID cannot be empty")
        );
    }
    
    if (sql.isEmpty()) {
        return Result<void>::error(
            Error::create(ErrorCode::InvalidArgument, 
                QString("SQL statement cannot be empty for query: %1").arg(queryId).toStdString())
        );
    }
    
    // Test that the query can be prepared (validate SQL syntax)
    QSqlQuery testQuery(m_writeDb);
    if (!testQuery.prepare(sql)) {
        QSqlError error = testQuery.lastError();
        return Result<void>::error(
            Error::create(ErrorCode::DatabaseError,
                QString("Failed to prepare query '%1': %2").arg(queryId, error.text()).toStdString())
        );
    }
    
    // Cache the SQL statement (queries are prepared on-demand when retrieved)
    m_querySqlCache[queryId] = sql;
    
    return Result<void>::ok();
}

QSqlQuery DatabaseManager::getPreparedQuery(const QString& queryId) {
    if (!m_isOpen) {
        return QSqlQuery(); // Return invalid query
    }
    
    if (!m_querySqlCache.contains(queryId)) {
        // Query not registered - return invalid query
        // Error should be logged by caller
        return QSqlQuery();
    }
    
    // Create and prepare query on write connection
    QSqlQuery query(m_writeDb);
    QString sql = m_querySqlCache[queryId];
    
    if (!query.prepare(sql)) {
        // Preparation failed - return invalid query
        // Error should be logged by caller
        return QSqlQuery();
    }
    
    return query;
}

QSqlQuery DatabaseManager::getPreparedQueryForRead(const QString& queryId) {
    if (!m_isOpen) {
        return QSqlQuery(); // Return invalid query
    }
    
    if (!m_querySqlCache.contains(queryId)) {
        // Query not registered - return invalid query
        // Error should be logged by caller
        return QSqlQuery();
    }
    
    // Create and prepare query on read connection
    QSqlQuery query(m_readDb);
    QString sql = m_querySqlCache[queryId];
    
    if (!query.prepare(sql)) {
        // Preparation failed - return invalid query
        // Error should be logged by caller
        return QSqlQuery();
    }
    
    return query;
}

bool DatabaseManager::hasQuery(const QString& queryId) const {
    return m_querySqlCache.contains(queryId);
}

QStringList DatabaseManager::getRegisteredQueries() const {
    return m_querySqlCache.keys();
}

} // namespace zmon

