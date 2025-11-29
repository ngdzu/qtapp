/**
 * @file SQLiteActionLogRepository.cpp
 * @brief Implementation of SQLiteActionLogRepository.
 * 
 * @author Z Monitor Team
 * @date 2025-01-15
 */

#include "SQLiteActionLogRepository.h"
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
    query.prepare(R"(
        SELECT id, timestamp_ms, action_type, user_id, target_id, details, result
        FROM action_log
        WHERE id = ?
    )");
    query.addBindValue(previousId);
    
    if (!query.exec() || !query.next()) {
        return QString(); // Previous entry not found
    }
    
    // Build hash input string
    QString hashInput = QString::number(query.value("id").toLongLong()) +
                       QString::number(query.value("timestamp_ms").toLongLong()) +
                       query.value("action_type").toString() +
                       query.value("user_id").toString() +
                       query.value("target_id").toString() +
                       query.value("details").toString() +
                       query.value("result").toString();
    
    // Compute SHA-256 hash
    QCryptographicHash hash(QCryptographicHash::Sha256);
    hash.addData(hashInput.toUtf8());
    return hash.result().toHex();
}

Result<void> SQLiteActionLogRepository::createTableIfNotExists() {
    QSqlQuery query(m_database);
    
    QString createTableSql = R"(
        CREATE TABLE IF NOT EXISTS action_log (
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            timestamp_ms INTEGER NOT NULL,
            timestamp_iso TEXT NOT NULL,
            user_id TEXT NULL,
            user_role TEXT NULL,
            action_type TEXT NOT NULL,
            target_type TEXT NULL,
            target_id TEXT NULL,
            details TEXT NULL,
            result TEXT NOT NULL,
            error_code TEXT NULL,
            error_message TEXT NULL,
            device_id TEXT NOT NULL,
            session_token_hash TEXT NULL,
            ip_address TEXT NULL,
            previous_hash TEXT NULL
        )
    )";
    
    if (!query.exec(createTableSql)) {
        return Result<void>::error(
            Error::create(ErrorCode::DatabaseError,
                          "Failed to create action_log table",
                          {{"error", query.lastError().text().toStdString()}}));
    }
    
    // Create indexes
    QStringList indexSqls = {
        "CREATE INDEX IF NOT EXISTS idx_action_log_timestamp ON action_log(timestamp_ms DESC)",
        "CREATE INDEX IF NOT EXISTS idx_action_log_user ON action_log(user_id, timestamp_ms DESC)",
        "CREATE INDEX IF NOT EXISTS idx_action_log_action_type ON action_log(action_type, timestamp_ms DESC)",
        "CREATE INDEX IF NOT EXISTS idx_action_log_target ON action_log(target_type, target_id, timestamp_ms DESC)",
        "CREATE INDEX IF NOT EXISTS idx_action_log_device ON action_log(device_id, timestamp_ms DESC)"
    };
    
    for (const QString& indexSql : indexSqls) {
        // Index creation failures are non-fatal; they affect performance, not correctness.
        query.exec(indexSql);
    }
    
    return Result<void>::ok();
}

qint64 SQLiteActionLogRepository::getLastEntryId() {
    QSqlQuery query(m_database);
    query.prepare("SELECT MAX(id) as max_id FROM action_log");
    
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
    
    for (const auto& entry : entries) {
        QSqlQuery query(m_database);
        query.prepare(R"(
            INSERT INTO action_log (
                timestamp_ms, timestamp_iso, user_id, user_role, action_type,
                target_type, target_id, details, result, error_code, error_message,
                device_id, session_token_hash, ip_address, previous_hash
            ) VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?)
        )");
        
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

