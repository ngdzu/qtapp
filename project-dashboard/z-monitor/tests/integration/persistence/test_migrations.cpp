/**
 * @file test_migrations.cpp
 * @brief Unit tests for database migration system.
 *
 * Tests verify that migrations apply correctly, schema version tracking works,
 * and database integrity is maintained after migrations.
 *
 * @author Z Monitor Team
 * @date 2025-01-15
 */

#include <gtest/gtest.h>
#include <QTemporaryFile>
#include <QCoreApplication>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include <QFileInfo>
#include <memory>
#include <QDir>

namespace zmon
{
    namespace persistence
    {

        class MigrationTest : public ::testing::Test
        {
        protected:
            void SetUp() override
            {
                if (!QCoreApplication::instance())
                {
                    int argc = 1;
                    char *argv[] = {const_cast<char *>("test")};
                    m_app = std::make_unique<QCoreApplication>(argc, argv);
                }

                m_tempFile = std::make_unique<QTemporaryFile>();
                m_tempFile->setAutoRemove(false);
                if (!m_tempFile->open())
                {
                    FAIL() << "Cannot create temporary database file";
                }
                m_dbPath = m_tempFile->fileName();
                m_tempFile->close();
            }

            void TearDown() override
            {
                if (m_db.isOpen())
                {
                    m_db.close();
                }
                QSqlDatabase::removeDatabase(m_db.connectionName());
                m_app.reset();
                QFile::remove(m_dbPath);
            }

            void applyMigrations()
            {
                // Open database first
                m_db = QSqlDatabase::addDatabase("QSQLITE", "migration_test");
                m_db.setDatabaseName(m_dbPath);

                if (!m_db.open())
                {
                    FAIL() << "Cannot open test database: " << m_db.lastError().text().toStdString();
                }

                // Apply migrations by reading and executing SQL files directly
                QString migrationsDir = QDir::currentPath() + "/schema/migrations";
                QFileInfo migrationsDirInfo(migrationsDir);
                if (!migrationsDirInfo.exists())
                {
                    // Try relative to project root
                    migrationsDir = QDir::currentPath() + "/../schema/migrations";
                }

                // Get list of migration files in order
                QDir dir(migrationsDir);
                QStringList filters;
                filters << "*.sql";
                QFileInfoList migrationFiles = dir.entryInfoList(filters, QDir::Files, QDir::Name);

                // Create schema_version table if it doesn't exist
                QSqlQuery createVersionTable(m_db);
                createVersionTable.exec(R"(
            CREATE TABLE IF NOT EXISTS schema_version (
                version INTEGER PRIMARY KEY,
                applied_at TEXT NOT NULL,
                description TEXT,
                migration_type TEXT DEFAULT 'schema'
            )
        )");

                // Apply each migration
                for (const QFileInfo &fileInfo : migrationFiles)
                {
                    // Extract version number from filename (e.g., "0001_initial.sql" -> 1)
                    QString baseName = fileInfo.baseName();
                    QString versionStr = baseName.split('_').first();
                    bool ok;
                    int version = versionStr.toInt(&ok);
                    if (!ok)
                        continue;

                    // Check if migration already applied
                    QSqlQuery checkQuery(m_db);
                    checkQuery.prepare("SELECT COUNT(*) FROM schema_version WHERE version = ?");
                    checkQuery.addBindValue(version);
                    if (checkQuery.exec() && checkQuery.next() && checkQuery.value(0).toInt() > 0)
                    {
                        continue; // Already applied
                    }

                    // Read and execute migration file
                    QFile file(fileInfo.absoluteFilePath());
                    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
                    {
                        FAIL() << "Cannot open migration file: " << fileInfo.absoluteFilePath().toStdString();
                    }

                    QString sql = file.readAll();
                    file.close();

                    // Execute SQL (split by semicolons for multiple statements)
                    QStringList statements = sql.split(';', Qt::SkipEmptyParts);
                    m_db.transaction();
                    bool success = true;
                    for (const QString &statement : statements)
                    {
                        QString trimmed = statement.trimmed();
                        if (trimmed.isEmpty() || trimmed.startsWith("--"))
                        {
                            continue; // Skip empty lines and comments
                        }
                        QSqlQuery query(m_db);
                        if (!query.exec(trimmed))
                        {
                            qDebug() << "Migration statement failed:" << trimmed;
                            qDebug() << "Error:" << query.lastError().text();
                            success = false;
                            break;
                        }
                    }

                    if (success)
                    {
                        m_db.commit();
                        // Record migration in schema_version
                        QSqlQuery insertVersion(m_db);
                        insertVersion.prepare("INSERT INTO schema_version (version, applied_at, description, migration_type) VALUES (?, ?, ?, ?)");
                        insertVersion.addBindValue(version);
                        insertVersion.addBindValue(QDateTime::currentDateTimeUtc().toString(Qt::ISODate));
                        insertVersion.addBindValue(fileInfo.baseName());
                        insertVersion.addBindValue("schema");
                        insertVersion.exec();
                    }
                    else
                    {
                        m_db.rollback();
                        FAIL() << "Migration failed: " << fileInfo.absoluteFilePath().toStdString();
                    }
                }
            }

            std::unique_ptr<QTemporaryFile> m_tempFile;
            QString m_dbPath;
            std::unique_ptr<QCoreApplication> m_app;
            QSqlDatabase m_db;
        };

        TEST_F(MigrationTest, SchemaVersionTableExists)
        {
            applyMigrations();

            QSqlQuery query(m_db);
            query.prepare("SELECT name FROM sqlite_master WHERE type='table' AND name='schema_version'");
            ASSERT_TRUE(query.exec());
            ASSERT_TRUE(query.next());
            EXPECT_EQ(query.value(0).toString(), "schema_version");
        }

        TEST_F(MigrationTest, AllRequiredTablesExist)
        {
            applyMigrations();

            QStringList requiredTables = {
                "patients", "vitals", "telemetry_metrics", "alarms",
                "admission_events", "action_log", "settings", "users",
                "certificates", "security_audit_log", "snapshots",
                "annotations", "infusion_events", "device_events",
                "notifications", "predictive_scores", "archival_jobs",
                "db_encryption_meta", "schema_version"};

            QSqlQuery query(m_db);
            query.prepare("SELECT name FROM sqlite_master WHERE type='table' AND name NOT LIKE 'sqlite_%' ORDER BY name");
            ASSERT_TRUE(query.exec());

            QStringList existingTables;
            while (query.next())
            {
                existingTables.append(query.value(0).toString());
            }

            for (const QString &table : requiredTables)
            {
                EXPECT_TRUE(existingTables.contains(table))
                    << "Required table missing: " << table.toStdString();
            }
        }

        TEST_F(MigrationTest, PatientsTableHasAllColumns)
        {
            applyMigrations();

            QStringList requiredColumns = {
                "mrn", "name", "dob", "sex", "allergies", "room",
                "created_at", "last_lookup_at", "lookup_source",
                "bed_location", "admitted_at", "discharged_at",
                "admission_source", "admission_status", "device_label"};

            QSqlQuery query(m_db);
            query.prepare("PRAGMA table_info(patients)");
            ASSERT_TRUE(query.exec());

            QStringList existingColumns;
            while (query.next())
            {
                existingColumns.append(query.value(1).toString()); // Column name is at index 1
            }

            for (const QString &column : requiredColumns)
            {
                EXPECT_TRUE(existingColumns.contains(column))
                    << "Required column missing in patients table: " << column.toStdString();
            }
        }

        TEST_F(MigrationTest, VitalsTableHasPatientMrn)
        {
            applyMigrations();

            QSqlQuery query(m_db);
            query.prepare("PRAGMA table_info(vitals)");
            ASSERT_TRUE(query.exec());

            bool hasPatientMrn = false;
            bool patientMrnNotNull = false;

            while (query.next())
            {
                QString columnName = query.value(1).toString();
                if (columnName == "patient_mrn")
                {
                    hasPatientMrn = true;
                    int notNull = query.value(3).toInt(); // notnull is at index 3
                    patientMrnNotNull = (notNull == 1);
                    break;
                }
            }

            EXPECT_TRUE(hasPatientMrn) << "vitals table must have patient_mrn column";
            EXPECT_TRUE(patientMrnNotNull) << "patient_mrn must be NOT NULL for patient association";
        }

        TEST_F(MigrationTest, ActionLogTableHasHashChain)
        {
            applyMigrations();

            QSqlQuery query(m_db);
            query.prepare("PRAGMA table_info(action_log)");
            ASSERT_TRUE(query.exec());

            bool hasPreviousHash = false;

            while (query.next())
            {
                if (query.value(1).toString() == "previous_hash")
                {
                    hasPreviousHash = true;
                    break;
                }
            }

            EXPECT_TRUE(hasPreviousHash) << "action_log table must have previous_hash column for tamper detection";
        }

        TEST_F(MigrationTest, IndicesCreated)
        {
            applyMigrations();

            QSqlQuery query(m_db);
            query.prepare("SELECT name FROM sqlite_master WHERE type='index' AND name NOT LIKE 'sqlite_%'");
            ASSERT_TRUE(query.exec());

            QStringList indices;
            while (query.next())
            {
                indices.append(query.value(0).toString());
            }

            // Check for key indices
            EXPECT_TRUE(indices.contains("idx_patients_mrn")) << "Missing index on patients.mrn";
            EXPECT_TRUE(indices.contains("idx_vitals_patient_time")) << "Missing index on vitals(patient_mrn, timestamp)";
            EXPECT_TRUE(indices.contains("idx_action_log_timestamp")) << "Missing index on action_log.timestamp_ms";
            EXPECT_TRUE(indices.contains("idx_alarms_patient_priority")) << "Missing index on alarms(patient_mrn, priority, start_time)";
        }

        TEST_F(MigrationTest, ForeignKeyConstraints)
        {
            applyMigrations();

            // Check that vitals.patient_mrn references patients.mrn
            QSqlQuery query(m_db);
            query.prepare("PRAGMA foreign_key_list(vitals)");
            ASSERT_TRUE(query.exec());

            bool hasPatientFk = false;
            while (query.next())
            {
                QString table = query.value(2).toString(); // Referenced table
                QString from = query.value(3).toString();  // From column
                if (table == "patients" && from == "patient_mrn")
                {
                    hasPatientFk = true;
                    break;
                }
            }

            EXPECT_TRUE(hasPatientFk) << "vitals table must have foreign key to patients.mrn";
        }

        TEST_F(MigrationTest, SettingsTableSupportsRequiredKeys)
        {
            applyMigrations();

            // Insert test settings
            QSqlQuery insertQuery(m_db);
            insertQuery.prepare("INSERT INTO settings (key, value, updated_at) VALUES (?, ?, ?)");

            QStringList requiredKeys = {"deviceId", "deviceLabel", "measurementUnit", "serverUrl", "useMockServer"};

            for (const QString &key : requiredKeys)
            {
                insertQuery.addBindValue(key);
                insertQuery.addBindValue("test_value");
                insertQuery.addBindValue(QDateTime::currentMSecsSinceEpoch());
                ASSERT_TRUE(insertQuery.exec()) << "Failed to insert setting: " << key.toStdString();
                insertQuery.finish();
            }

            // Verify all settings can be retrieved
            QSqlQuery selectQuery(m_db);
            selectQuery.prepare("SELECT key FROM settings WHERE key IN (?, ?, ?, ?, ?)");
            for (int i = 0; i < 5; ++i)
            {
                selectQuery.addBindValue(requiredKeys[i]);
            }
            ASSERT_TRUE(selectQuery.exec());

            int count = 0;
            while (selectQuery.next())
            {
                count++;
            }
            EXPECT_EQ(count, 5) << "All required settings keys should be insertable";
        }

        TEST_F(MigrationTest, DatabaseIntegrityAfterMigrations)
        {
            applyMigrations();

            // Verify database integrity
            QSqlQuery integrityQuery(m_db);
            integrityQuery.prepare("PRAGMA integrity_check");
            ASSERT_TRUE(integrityQuery.exec());
            ASSERT_TRUE(integrityQuery.next());
            EXPECT_EQ(integrityQuery.value(0).toString(), "ok")
                << "Database integrity check failed after migrations";
        }

    } // namespace persistence
} // namespace zmon
