/**
 * @file MockDatabaseManager.cpp
 * @brief Mock implementation of DatabaseManager for testing.
 *
 * @author Z Monitor Team
 * @date 2025-11-29
 */

#include "MockDatabaseManager.h"
#include <QSqlQuery>
#include <QSqlError>
#include <QDebug>

namespace zmon
{

    int MockDatabaseManager::s_connectionCounter = 0;

    MockDatabaseManager::MockDatabaseManager(QObject *parent)
        : QObject(parent), m_inTransaction(false)
    {
        // Create unique connection name for this mock instance
        QString connectionName = QString("mock_db_%1").arg(s_connectionCounter++);
        m_db = QSqlDatabase::addDatabase("QSQLITE", connectionName);
        m_db.setDatabaseName(":memory:");

        if (!m_db.open())
        {
            qWarning() << "MockDatabaseManager: Failed to open in-memory database:" << m_db.lastError().text();
            return;
        }

        // Initialize test schema
        auto result = initializeTestSchema();
        if (result.isError())
        {
            qWarning() << "MockDatabaseManager: Failed to initialize schema:" << QString::fromStdString(result.error().message);
        }
    }

    MockDatabaseManager::~MockDatabaseManager()
    {
        close();
        QString connectionName = m_db.connectionName();
        m_db = QSqlDatabase(); // Release the database
        QSqlDatabase::removeDatabase(connectionName);
    }

    Result<void> MockDatabaseManager::open(const QString &, const QString &)
    {
        // Already opened in constructor - this is a no-op
        return Result<void>::ok();
    }

    void MockDatabaseManager::close()
    {
        if (m_db.isOpen())
        {
            m_db.close();
        }
    }

    bool MockDatabaseManager::isOpen() const
    {
        return m_db.isOpen();
    }

    QSqlDatabase &MockDatabaseManager::getConnection()
    {
        return m_db;
    }

    Result<void> MockDatabaseManager::beginTransaction()
    {
        if (m_inTransaction)
        {
            return Result<void>::error(Error::create(ErrorCode::Conflict, "Transaction already in progress"));
        }

        if (!m_db.transaction())
        {
            return Result<void>::error(Error::create(ErrorCode::DatabaseError, m_db.lastError().text().toStdString()));
        }

        m_inTransaction = true;
        return Result<void>::ok();
    }

    Result<void> MockDatabaseManager::commit()
    {
        if (!m_inTransaction)
        {
            return Result<void>::error(Error::create(ErrorCode::Conflict, "No transaction in progress"));
        }

        if (!m_db.commit())
        {
            return Result<void>::error(Error::create(ErrorCode::DatabaseError, m_db.lastError().text().toStdString()));
        }

        m_inTransaction = false;
        return Result<void>::ok();
    }

    Result<void> MockDatabaseManager::rollback()
    {
        if (!m_inTransaction)
        {
            return Result<void>::error(Error::create(ErrorCode::Conflict, "No transaction in progress"));
        }

        if (!m_db.rollback())
        {
            return Result<void>::error(Error::create(ErrorCode::DatabaseError, m_db.lastError().text().toStdString()));
        }

        m_inTransaction = false;
        return Result<void>::ok();
    }

    Result<void> MockDatabaseManager::registerPreparedQuery(const QString &queryId, const QString &sql)
    {
        m_preparedQueries[queryId] = sql;
        return Result<void>::ok();
    }

    QSqlQuery MockDatabaseManager::getPreparedQuery(const QString &queryId)
    {
        if (!m_preparedQueries.contains(queryId))
        {
            qWarning() << "MockDatabaseManager: Query not registered:" << queryId;
            return QSqlQuery(m_db);
        }

        QSqlQuery query(m_db);
        if (!query.prepare(m_preparedQueries[queryId]))
        {
            qWarning() << "MockDatabaseManager: Failed to prepare query:" << queryId << "-" << query.lastError().text();
        }

        return query;
    }

    bool MockDatabaseManager::hasQuery(const QString &queryId) const
    {
        return m_preparedQueries.contains(queryId);
    }

    Result<void> MockDatabaseManager::initializeTestSchema()
    {
        QSqlQuery query(m_db);

        // Create vitals table (simplified schema for testing)
        QString createVitalsTable = R"(
            CREATE TABLE IF NOT EXISTS vitals (
                id INTEGER PRIMARY KEY AUTOINCREMENT,
                patient_mrn TEXT NOT NULL,
                timestamp INTEGER NOT NULL,
                heart_rate REAL,
                spo2 REAL,
                respiration_rate REAL,
                signal_quality INTEGER,
                source TEXT,
                is_synced INTEGER DEFAULT 0
            )
        )";

        if (!query.exec(createVitalsTable))
        {
            return Result<void>::error(Error::create(ErrorCode::DatabaseError,
                                                     QString("Failed to create vitals table: %1").arg(query.lastError().text()).toStdString()));
        }

        // Create patients table (for FK constraints if needed)
        QString createPatientsTable = R"(
            CREATE TABLE IF NOT EXISTS patients (
                mrn TEXT PRIMARY KEY,
                first_name TEXT,
                last_name TEXT,
                date_of_birth TEXT,
                created_at INTEGER
            )
        )";

        if (!query.exec(createPatientsTable))
        {
            return Result<void>::error(Error::create(ErrorCode::DatabaseError,
                                                     QString("Failed to create patients table: %1").arg(query.lastError().text()).toStdString()));
        }

        return Result<void>::ok();
    }

} // namespace zmon
