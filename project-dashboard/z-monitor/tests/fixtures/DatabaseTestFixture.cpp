#include "tests/fixtures/DatabaseTestFixture.h"
#include "infrastructure/persistence/QueryRegistry.h"
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

        m_dbManager = std::make_unique<DatabaseManager>();
        auto result = m_dbManager->open("file::memory:?cache=shared");
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

    // Simple SQL splitter: splits on semicolons not inside strings.
    // Good enough for generated DDL which avoids complex constructs.
    QStringList DatabaseTestFixture::splitSqlStatements(const QString &sql) const
    {
        QStringList out;
        QString current;
        bool inString = false;
        const QChar quote('"');
        for (int i = 0; i < sql.size(); ++i)
        {
            QChar c = sql.at(i);
            if (c == quote)
            {
                inString = !inString;
                current.append(c);
            }
            else if (c == ';' && !inString)
            {
                out << current;
                current.clear();
            }
            else
            {
                current.append(c);
            }
        }
        if (!current.trimmed().isEmpty())
            out << current;
        return out;
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

            const QStringList statements = splitSqlStatements(sql);
            ASSERT_FALSE(statements.isEmpty()) << "No statements in DDL file " << filePath.toStdString();

            bool success = true;
            for (const QString &stmt : statements)
            {
                const QString trimmed = stmt.trimmed();
                if (trimmed.isEmpty())
                    continue;
                if (trimmed.startsWith("--"))
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
