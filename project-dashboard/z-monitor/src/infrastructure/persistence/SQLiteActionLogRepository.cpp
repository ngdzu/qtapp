/**
 * @file SQLiteActionLogRepository.cpp
 * @brief Implementation of SQLiteActionLogRepository.
 * 
 * @author Z Monitor Team
 * @date 2025-01-15
 */

#include "SQLiteActionLogRepository.h"
#include "generated/SchemaInfo.h"
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include <QDateTime>
#include <QJsonDocument>
#include <QCryptographicHash>
#include <QMutexLocker>

namespace zmon {

namespace {
    constexpr int BATCH_FLUSH_INTERVAL_MS = 5000;  // Flush pending entries every 5 seconds
}

SQLiteActionLogRepository::SQLiteActionLogRepository(const QString& databasePath, QObject* parent)
    : IActionLogRepository(parent)
    , m_databasePath(databasePath)
    , m_flushTimer(new QTimer(this))
{
    // Set up batch write timer (flush every 5 seconds)
    m_flushTimer->setInterval(BATCH_FLUSH_INTERVAL_MS);
    m_flushTimer->setSingleShot(false);
    connect(m_flushTimer, &QTimer::timeout, this, &SQLiteActionLogRepository::flushPendingEntries);
}

SQLiteActionLogRepository::~SQLiteActionLogRepository() {
    // Flush any pending entries before destruction
    flushPendingEntries();
    
    if (m_database.isOpen()) {
        m_database.close();
    }
}

Result<void> SQLiteActionLogRepository::initialize() {
    // Open database connection
    m_database = QSqlDatabase::addDatabase("QSQLITE", "action_log_connection");
    m_database.setDatabaseName(m_databasePath);
    
    if (!m_database.open()) {
        return Result<void>::error(
            Error::create(ErrorCode::DatabaseError,
                          "Failed to open action_log database",
                          {{"databasePath", m_databasePath.toStdString()},
                           {"error", m_database.lastError().text().toStdString()}}));
    }
    
    // Create table if it doesn't exist
    auto tableResult = createTableIfNotExists();
    if (tableResult.isError()) {
        return tableResult;
    }
    
    // Start batch write timer
    m_flushTimer->start();
    
    return Result<void>::ok();
}

void SQLiteActionLogRepository::logAction(const ActionLogEntry& entry) {
    QMutexLocker locker(&m_queueMutex);
    m_pendingEntries.enqueue(entry);
}

void SQLiteActionLogRepository::logActions(const QList<ActionLogEntry>& entries) {
    QMutexLocker locker(&m_queueMutex);
    for (const auto& entry : entries) {
        m_pendingEntries.enqueue(entry);
    }
}

void SQLiteActionLogRepository::queryActions(const ActionLogFilter& filter) {
    // TODO: Implement query on background thread
    // For now, emit empty result
    // This will be implemented when DatabaseManager is available
    emit actionsQueried(QList<ActionLogEntry>());
}

void SQLiteActionLogRepository::flushPendingEntries() {
    QMutexLocker locker(&m_queueMutex);
    
    if (m_pendingEntries.isEmpty()) {
        return;
    }
    
    // Copy pending entries and clear queue
    QList<ActionLogEntry> entriesToWrite;
    while (!m_pendingEntries.isEmpty()) {
        entriesToWrite.append(m_pendingEntries.dequeue());
    }
    locker.unlock();
    
    // Write to database
    auto result = writeEntriesToDatabase(entriesToWrite);
    if (result.isOk()) {
        // Emit success signals
        for (const auto& entry : entriesToWrite) {
            emit actionLogged(entry);
        }
    } else {
        // Emit failure signals
        for (const auto& entry : entriesToWrite) {
            emit actionLogFailed(entry, result.error().message);
        }
    }
}

QString SQLiteActionLogRepository::computePreviousHash(qint64 previousId) {
    if (previousId == 0) {
        return QString(); // No previous entry
    }
    
    QSqlQuery query(m_database);
    using namespace Schema::Tables;
    using namespace Schema::Columns::ActionLog;
    
    query.prepare(QString("SELECT %1, %2, %3, %4, %5, %6, %7 FROM %8 WHERE %1 = ?")
                  .arg(ID)
                  .arg(TIMESTAMP_MS)
                  .arg(ACTION_TYPE)
                  .arg(USER_ID)
                  .arg(TARGET_ID)
                  .arg(DETAILS)
                  .arg(RESULT)
                  .arg(ACTION_LOG));
    query.addBindValue(previousId);
    
    if (!query.exec() || !query.next()) {
        return QString(); // Previous entry not found
    }
    
    // Build hash input string
    QString hashInput = QString::number(query.value(ID).toLongLong()) +
                       QString::number(query.value(TIMESTAMP_MS).toLongLong()) +
                       query.value(ACTION_TYPE).toString() +
                       query.value(USER_ID).toString() +
                       query.value(TARGET_ID).toString() +
                       query.value(DETAILS).toString() +
                       query.value(RESULT).toString();
    
    // Compute SHA-256 hash
    QCryptographicHash hash(QCryptographicHash::Sha256);
    hash.addData(hashInput.toUtf8());
    return hash.result().toHex();
}

Result<void> SQLiteActionLogRepository::createTableIfNotExists() {
    QSqlQuery query(m_database);
    using namespace Schema::Tables;
    using namespace Schema::Columns::ActionLog;
    
    QString createTableSql = QString(R"(
        CREATE TABLE IF NOT EXISTS %1 (
            %2 INTEGER PRIMARY KEY AUTOINCREMENT,
            %3 INTEGER NOT NULL,
            %4 TEXT NOT NULL,
            %5 TEXT NULL,
            %6 TEXT NULL,
            %7 TEXT NOT NULL,
            %8 TEXT NULL,
            %9 TEXT NULL,
            %10 TEXT NULL,
            %11 TEXT NOT NULL,
            %12 TEXT NULL,
            %13 TEXT NULL,
            %14 TEXT NOT NULL,
            %15 TEXT NULL,
            %16 TEXT NULL,
            %17 TEXT NULL
        )
    )")
        .arg(ACTION_LOG)
        .arg(ID)
        .arg(TIMESTAMP_MS)
        .arg(TIMESTAMP_ISO)
        .arg(USER_ID)
        .arg(USER_ROLE)
        .arg(ACTION_TYPE)
        .arg(TARGET_TYPE)
        .arg(TARGET_ID)
        .arg(DETAILS)
        .arg(RESULT)
        .arg(ERROR_CODE)
        .arg(ERROR_MESSAGE)
        .arg(DEVICE_ID)
        .arg(SESSION_TOKEN_HASH)
        .arg(IP_ADDRESS)
        .arg(PREVIOUS_HASH);
    
    if (!query.exec(createTableSql)) {
        return Result<void>::error(
            Error::create(ErrorCode::DatabaseError,
                          "Failed to create action_log table",
                          {{"error", query.lastError().text().toStdString()}}));
    }
    
    // Create indexes
    QStringList indexSqls = {
        QString("CREATE INDEX IF NOT EXISTS idx_action_log_timestamp ON %1(%2 DESC)")
            .arg(ACTION_LOG, TIMESTAMP_MS),
        QString("CREATE INDEX IF NOT EXISTS idx_action_log_user ON %1(%2, %3 DESC)")
            .arg(ACTION_LOG, USER_ID, TIMESTAMP_MS),
        QString("CREATE INDEX IF NOT EXISTS idx_action_log_action_type ON %1(%2, %3 DESC)")
            .arg(ACTION_LOG, ACTION_TYPE, TIMESTAMP_MS),
        QString("CREATE INDEX IF NOT EXISTS idx_action_log_target ON %1(%2, %3, %4 DESC)")
            .arg(ACTION_LOG, TARGET_TYPE, TARGET_ID, TIMESTAMP_MS),
        QString("CREATE INDEX IF NOT EXISTS idx_action_log_device ON %1(%2, %3 DESC)")
            .arg(ACTION_LOG, DEVICE_ID, TIMESTAMP_MS)
    };
    
    for (const QString& indexSql : indexSqls) {
        // Index creation failures are non-fatal; they affect performance, not correctness.
        query.exec(indexSql);
    }
    
    return Result<void>::ok();
}

qint64 SQLiteActionLogRepository::getLastEntryId() {
    QSqlQuery query(m_database);
    using namespace Schema::Tables;
    using namespace Schema::Columns::ActionLog;
    
    query.prepare(QString("SELECT MAX(%1) as max_id FROM %2")
                  .arg(ID)
                  .arg(ACTION_LOG));
    
    if (!query.exec() || !query.next()) {
        return 0;
    }
    
    return query.value("max_id").toLongLong();
}

Result<void> SQLiteActionLogRepository::writeEntriesToDatabase(const QList<ActionLogEntry>& entries) {
    if (entries.isEmpty()) {
        return Result<void>::ok();
    }
    
    // Start transaction
    if (!m_database.transaction()) {
        return Result<void>::error(
            Error::create(ErrorCode::DatabaseError,
                          "Failed to start transaction for action_log write",
                          {{"error", m_database.lastError().text().toStdString()}}));
    }
    
    qint64 previousId = getLastEntryId();
    
    using namespace Schema::Tables;
    using namespace Schema::Columns::ActionLog;
    
    for (const auto& entry : entries) {
        QSqlQuery query(m_database);
        query.prepare(QString(R"(
            INSERT INTO %1 (
                %2, %3, %4, %5, %6,
                %7, %8, %9, %10, %11, %12,
                %13, %14, %15, %16
            ) VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?)
        )")
            .arg(ACTION_LOG)
            .arg(TIMESTAMP_MS)
            .arg(TIMESTAMP_ISO)
            .arg(USER_ID)
            .arg(USER_ROLE)
            .arg(ACTION_TYPE)
            .arg(TARGET_TYPE)
            .arg(TARGET_ID)
            .arg(DETAILS)
            .arg(RESULT)
            .arg(ERROR_CODE)
            .arg(ERROR_MESSAGE)
            .arg(DEVICE_ID)
            .arg(SESSION_TOKEN_HASH)
            .arg(IP_ADDRESS)
            .arg(PREVIOUS_HASH));
        
        qint64 timestampMs = QDateTime::currentDateTimeUtc().toMSecsSinceEpoch();
        QString timestampIso = QDateTime::currentDateTimeUtc().toString(Qt::ISODate);
        QString previousHash = computePreviousHash(previousId);
        
        // Serialize details JSON
        QJsonDocument doc(entry.details);
        QString detailsJson = doc.toJson(QJsonDocument::Compact);
        
        query.addBindValue(timestampMs);
        query.addBindValue(timestampIso);
        query.addBindValue(entry.userId);
        query.addBindValue(entry.userRole);
        query.addBindValue(entry.actionType);
        query.addBindValue(entry.targetType);
        query.addBindValue(entry.targetId);
        query.addBindValue(detailsJson);
        query.addBindValue(entry.result);
        query.addBindValue(entry.errorCode);
        query.addBindValue(entry.errorMessage);
        query.addBindValue(entry.deviceId);
        query.addBindValue(entry.sessionTokenHash);
        query.addBindValue(entry.ipAddress);
        query.addBindValue(previousHash);
        
        if (!query.exec()) {
            Error err = Error::create(
                ErrorCode::DatabaseError,
                "Failed to insert action_log entry",
                {{"error", query.lastError().text().toStdString()}});
            m_database.rollback();
            return Result<void>::error(err);
        }
        
        previousId = query.lastInsertId().toLongLong();
    }
    
    // Commit transaction
    if (!m_database.commit()) {
        Error err = Error::create(
            ErrorCode::DatabaseError,
            "Failed to commit action_log transaction",
            {{"error", m_database.lastError().text().toStdString()}});
        m_database.rollback();
        return Result<void>::error(err);
    }
    
    return Result<void>::ok();
}

} // namespace zmon

