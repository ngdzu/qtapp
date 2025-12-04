#include "tests/fixtures/DatabaseTestFixture.h"
#include "infrastructure/persistence/QueryRegistry.h"
#include "infrastructure/persistence/SqlUtils.h"
#include <filesystem>

namespace zmon::test
{
    void DatabaseTestFixture::SetUp()
    {
        if (!QCoreApplication::instance())
        {
            int argc = 1;
            char *argv[] = {const_cast<char *>("test")};
            m_app = std::make_unique<QCoreApplication>(argc, argv);
        }
        // CRITICAL FIX: Use unique database name for each test instance
        // Problem: file::memory:?cache=shared creates a SHARED cache that persists across tests
        // Solution: Generate unique URI for each test to ensure complete isolation
        const QString uniqueDbUri = QString("file:test_%1?mode=memory&cache=shared")
                                        .arg(QUuid::createUuid().toString(QUuid::WithoutBraces));

        m_dbManager = std::make_unique<DatabaseManager>();
        auto result = m_dbManager->open(uniqueDbUri);
        if (result.isError())
        {
            FAIL() << "Failed to open in-memory database: " << result.error().message;
        }

        // Enforce foreign keys on write connection
        QSqlQuery pragma(m_dbManager->getWriteConnection());
        pragma.exec("PRAGMA foreign_keys = ON");

        applyMigrations();

        // Skip global query registration to avoid noisy failures in focused tests.
    }

    void DatabaseTestFixture::TearDown()
    {
        if (m_dbManager)
        {
            // Force finish any pending queries by clearing result sets
            // This prevents "connection is still in use" warnings
            {
                QSqlQuery q(m_dbManager->getWriteConnection());
                q.finish();
            }
            {
                QSqlQuery q(m_dbManager->getReadConnection());
                q.finish();
            }

            m_dbManager->close();
            m_dbManager.reset();
        }
        m_app.reset();
    }

    QSqlDatabase &DatabaseTestFixture::db()
    {
        return m_dbManager->getWriteConnection();
    }

    DatabaseManager *DatabaseTestFixture::databaseManager()
    {
        return m_dbManager.get();
    }

    void DatabaseTestFixture::applyMigrations()
    {
        // Apply generated DDL in deterministic order: tables then indices
        const QString baseDir = QString::fromStdString(std::filesystem::path(Z_MONITOR_SOURCE_DIR).string());
        const QString ddlDir = baseDir + "/schema/generated/ddl";
        const QStringList files = {ddlDir + "/create_tables.sql"};

        QSqlQuery pragma(db());
        pragma.exec("PRAGMA foreign_keys = OFF");

        for (const QString &filePath : files)
        {
            QFile f(filePath);
            ASSERT_TRUE(f.open(QIODevice::ReadOnly | QIODevice::Text)) << "Cannot open DDL file " << filePath.toStdString();
            const QString sql = QString::fromUtf8(f.readAll());
            f.close();

            // Use centralized SQL utility to parse statements
            const QStringList statements = zmon::sql::splitSqlStatements(sql);
            ASSERT_FALSE(statements.isEmpty()) << "No statements in DDL file " << filePath.toStdString();

            bool success = true;
            for (const QString &stmt : statements)
            {
                const QString trimmed = stmt.trimmed();

                // Skip empty statements, comments, and transaction control
                if (zmon::sql::isSqlComment(trimmed))
                    continue;
                if (trimmed.startsWith("BEGIN", Qt::CaseInsensitive) ||
                    trimmed.startsWith("COMMIT", Qt::CaseInsensitive) ||
                    trimmed.startsWith("ROLLBACK", Qt::CaseInsensitive))
                {
                    continue;
                }

                QSqlQuery q(db());
                if (!q.exec(trimmed))
                {
                    const QSqlError err = q.lastError();
                    if (!err.text().contains("already exists", Qt::CaseInsensitive))
                    {
                        ADD_FAILURE() << "DDL statement failed in " << filePath.toStdString() << ": " << err.text().toStdString();
                        success = false;
                        break;
                    }
                }
            }

            ASSERT_TRUE(success) << "Failed applying DDL file " << filePath.toStdString();
        }

        // Ensure patients table exists for repository tests
        {
            QSqlQuery q(db());
            const char *createPatients =
                "CREATE TABLE IF NOT EXISTS patients (\n"
                "    mrn TEXT PRIMARY KEY NOT NULL,\n"
                "    name TEXT,\n"
                "    dob TEXT,\n"
                "    sex TEXT\n"
                ");";
            q.exec(createPatients);
        }

        pragma.exec("PRAGMA foreign_keys = ON");
    }

} // namespace zmon::test