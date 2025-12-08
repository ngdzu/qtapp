/**
 * @file MonitoringWorkflowTest.cpp
 * @brief Integration tests for MonitoringService end-to-end workflow.
 *
 * Tests the complete workflow from sensor data → monitoring service →
 * alarm detection → repository persistence. Verifies integration between
 * domain, application, and infrastructure layers.
 *
 * @author Z Monitor Team
 * @date 2025-12-02
 */

#include <gtest/gtest.h>
#include "application/services/MonitoringService.h"
#include "domain/monitoring/VitalRecord.h"
#include "domain/monitoring/WaveformSample.h"
#include "infrastructure/persistence/SQLiteVitalsRepository.h"
#include "infrastructure/persistence/SQLiteAlarmRepository.h"
#include "infrastructure/persistence/SQLitePatientRepository.h"
#include "infrastructure/persistence/SQLiteTelemetryRepository.h"
#include "infrastructure/persistence/DatabaseManager.h"
#include "infrastructure/caching/VitalsCache.h"
#include "infrastructure/caching/WaveformCache.h"
#include "tests/mocks/infrastructure/MockSensorDataSource.h"
#include <QSignalSpy>
#include <QTest>
#include <QTemporaryFile>
#include <QCoreApplication>
#include <QLibraryInfo>
#include <QFile>
#include <QUuid>
#include <QSqlQuery>
#include <QSqlError>
#include <QDateTime>
#include <chrono>
#include <thread>
#include <filesystem>
#include "infrastructure/persistence/QueryRegistry.h"
#include "infrastructure/persistence/SqlUtils.h"
#include "domain/events/DomainEventDispatcher.h"

using namespace zmon;

/**
 * @class MonitoringWorkflowTest
 * @brief Integration test fixture for complete monitoring workflow.
 *
 * Uses real repositories with in-memory database to verify integration
 * between all layers.
 */
class MonitoringWorkflowTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        // Ensure QCoreApplication exists for Qt SQL and signals
        static QCoreApplication *appInstance = nullptr;
        if (!appInstance)
        {
            int argc = 0;
            char *argv[] = {(char *)""};
            appInstance = new QCoreApplication(argc, argv);
            qInfo() << "QCoreApplication created for tests";

            // Ensure Qt can locate plugins like QSQLITE
            QCoreApplication::addLibraryPath(QLibraryInfo::path(QLibraryInfo::PluginsPath));
            qInfo() << "Added plugin path:" << QLibraryInfo::path(QLibraryInfo::PluginsPath);
        }

        // STEP 1: Verify QSQLITE driver availability
        qInfo() << "Available SQL drivers:" << QSqlDatabase::drivers();
        ASSERT_TRUE(QSqlDatabase::isDriverAvailable("QSQLITE"))
            << "QSQLITE driver not available. Available drivers: "
            << QSqlDatabase::drivers().join(", ").toStdString();
        qInfo() << "QSQLITE driver confirmed available";

        // Create in-memory database
        dbManager = std::make_shared<DatabaseManager>();
        // Use unique URI to avoid creating files on disk
        const QString uniqueDbUri = QString("file:test_%1?mode=memory&cache=shared")
                                        .arg(QUuid::createUuid().toString(QUuid::WithoutBraces));
        qInfo() << "Opening database with URI:" << uniqueDbUri;
        auto openResult = dbManager->open(uniqueDbUri);
        ASSERT_TRUE(openResult.isOk()) << "Failed to open database: "
                                       << (openResult.isError() ? openResult.error().message : "Unknown error");
        qInfo() << "Database opened successfully";

        // Apply migrations using DatabaseManager API (ensures correct connection + transaction)
        qInfo() << "Executing database migrations...";

        // FIX: Use direct DDL file loading instead of Qt resources (which aren't compiled into test binary)
        // This approach is proven to work in DatabaseTestFixture
        const QString baseDir = QString::fromStdString(std::filesystem::path(Z_MONITOR_SOURCE_DIR).string());
        const QString ddlPath = baseDir + "/schema/generated/ddl/create_tables.sql";
        qInfo() << "Loading DDL from:" << ddlPath;

        QFile ddlFile(ddlPath);
        ASSERT_TRUE(ddlFile.open(QIODevice::ReadOnly | QIODevice::Text))
            << "Failed to open DDL file: " << ddlPath.toStdString();

        QString ddl = QString::fromUtf8(ddlFile.readAll());
        ddlFile.close();
        qInfo() << "DDL loaded, size:" << ddl.size() << "bytes";

        // Split SQL statements by semicolon (using SqlUtils helper that handles quotes)
        QStringList statements = zmon::sql::splitSqlStatements(ddl);
        qInfo() << "Split into" << statements.size() << "SQL statements";

        // Execute each statement on write connection
        QSqlDatabase writeDb = dbManager->getWriteConnection();
        ASSERT_TRUE(writeDb.isOpen()) << "Write connection not open";

        QSqlQuery pragma(writeDb);
        pragma.exec("PRAGMA foreign_keys = OFF");

        // Drop alarms table to ensure fresh schema (in case shared memory DB has old schema)
        QSqlQuery dropAlarms(writeDb);
        dropAlarms.exec("DROP TABLE IF EXISTS alarms");
        qInfo() << "Dropped alarms table (if existed) to ensure fresh schema";

        int executedCount = 0;
        for (const QString &stmt : statements)
        {
            // Strip SQL comments using SqlUtils (handles DDL generator bug with commas in comments)
            QString cleanedStmt = zmon::sql::stripSqlComments(stmt);

            QString trimmed = cleanedStmt.trimmed();
            if (trimmed.isEmpty())
                continue;
            if (trimmed.startsWith("BEGIN", Qt::CaseInsensitive) ||
                trimmed.startsWith("COMMIT", Qt::CaseInsensitive) ||
                trimmed.startsWith("ROLLBACK", Qt::CaseInsensitive))
            {
                continue;
            }

            QSqlQuery query(writeDb);
            bool success = query.exec(trimmed);
            if (!success)
            {
                QSqlError err = query.lastError();
                // Ignore "already exists" errors
                if (!err.text().contains("already exists", Qt::CaseInsensitive))
                {
                    FAIL() << "Failed to execute DDL statement: " << err.text().toStdString()
                           << "\nStatement: " << trimmed.left(300).toStdString();
                }
            }
            else
            {
                executedCount++;
            }
        }

        pragma.exec("PRAGMA foreign_keys = ON");
        qInfo() << "Executed" << executedCount << "DDL statements successfully";

        // STEP 3: Verify schema creation - check that tables exist
        QSqlQuery schemaCheck(dbManager->getWriteConnection());
        ASSERT_TRUE(schemaCheck.exec("SELECT name FROM sqlite_master WHERE type='table' ORDER BY name"))
            << "Failed to query sqlite_master: " << schemaCheck.lastError().text().toStdString();

        QStringList tables;
        while (schemaCheck.next())
        {
            tables << schemaCheck.value(0).toString();
        }
        qInfo() << "Tables found in database:" << tables;

        // Verify required tables exist
        ASSERT_TRUE(tables.contains("alarms")) << "Missing required table: alarms";
        ASSERT_TRUE(tables.contains("vitals")) << "Missing required table: vitals";
        ASSERT_TRUE(tables.contains("patients")) << "Missing required table: patients";
        ASSERT_TRUE(tables.contains("telemetry_metrics")) << "Missing required table: telemetry_metrics";
        qInfo() << "All required tables verified";

        // Register all prepared queries for repositories (after migrations)
        qInfo() << "Registering prepared queries...";
        zmon::persistence::QueryCatalog::initializeQueries(dbManager.get());
        qInfo() << "Query registration complete";

        // Insert test patient record (required for FK constraints on vitals/alarms)
        // Use INSERT OR REPLACE since shared in-memory DB persists across tests
        QSqlQuery insertPatient(dbManager->getWriteConnection());
        insertPatient.prepare("INSERT OR REPLACE INTO patients (mrn, name, dob, sex, created_at, admission_status) VALUES (?, ?, ?, ?, ?, ?)");
        insertPatient.addBindValue("MRN-TEST-001");
        insertPatient.addBindValue("Test Patient");
        insertPatient.addBindValue("1990-01-01");
        insertPatient.addBindValue("M");
        insertPatient.addBindValue(QDateTime::currentMSecsSinceEpoch());
        insertPatient.addBindValue("ADMITTED");
        ASSERT_TRUE(insertPatient.exec()) << "Failed to insert test patient: "
                                          << insertPatient.lastError().text().toStdString();
        qInfo() << "Test patient inserted: MRN-TEST-001";

        // Register Qt metatypes for signals/slots carrying domain types
        qRegisterMetaType<zmon::VitalRecord>("VitalRecord");
        qRegisterMetaType<zmon::AlarmSnapshot>("AlarmSnapshot");

        // Create real repositories
        vitalsRepo = std::make_shared<SQLiteVitalsRepository>(std::static_pointer_cast<IDatabaseManager>(dbManager));
        alarmRepo = std::make_shared<SQLiteAlarmRepository>(std::static_pointer_cast<IDatabaseManager>(dbManager));
        patientRepo = std::make_shared<SQLitePatientRepository>(dbManager.get());
        telemetryRepo = std::make_shared<SQLiteTelemetryRepository>(std::static_pointer_cast<IDatabaseManager>(dbManager));

        // Create caches
        vitalsCache = std::make_shared<VitalsCache>();
        waveformCache = std::make_shared<WaveformCache>();

        // Create mock sensor data source
        sensorDataSource = std::make_shared<MockSensorDataSource>();
        eventDispatcher = std::make_shared<DomainEventDispatcher>();

        // Create monitoring service with real infrastructure
        service = std::make_unique<MonitoringService>(
            patientRepo, telemetryRepo, alarmRepo, vitalsRepo,
            sensorDataSource, vitalsCache, waveformCache, eventDispatcher);
    }

    void TearDown() override
    {
        service.reset();
        // Don't close DB here as it's shared in-memory
    }

    // Helper method to create a vital record
    VitalRecord createVital(const std::string &type, double value)
    {
        const auto nowMs = std::chrono::duration_cast<std::chrono::milliseconds>(
                               std::chrono::system_clock::now().time_since_epoch())
                               .count();
        return VitalRecord(type, value, nowMs, 100, "MRN-TEST-001", "ZM-TEST-01");
    }

    // Infrastructure
    std::shared_ptr<DatabaseManager> dbManager;
    std::shared_ptr<SQLiteVitalsRepository> vitalsRepo;
    std::shared_ptr<SQLiteAlarmRepository> alarmRepo;
    std::shared_ptr<SQLitePatientRepository> patientRepo;
    std::shared_ptr<SQLiteTelemetryRepository> telemetryRepo;
    std::shared_ptr<VitalsCache> vitalsCache;
    std::shared_ptr<WaveformCache> waveformCache;
    std::shared_ptr<MockSensorDataSource> sensorDataSource;
    std::shared_ptr<DomainEventDispatcher> eventDispatcher;

    // Service under test
    std::unique_ptr<MonitoringService> service;
};

/**
 * @test Verify complete workflow: sensor → monitoring → alarm → persistence.
 */
TEST_F(MonitoringWorkflowTest, EndToEndAlarmWorkflow)
{
    // Start monitoring service
    ASSERT_TRUE(service->start());

    // Create signal spies
    QSignalSpy alarmSpy(service.get(), &MonitoringService::alarmRaised);
    QSignalSpy vitalSpy(service.get(), &MonitoringService::vitalProcessed);

    // Simulate sensor emitting high HR vital
    VitalRecord vital = createVital("HR", 150.0); // Above threshold (120)
    emit sensorDataSource->vitalSignsReceived(vital);

    // Wait for processing
    QTest::qWait(50); // Allow async processing

    // Verify vital was processed
    EXPECT_EQ(vitalSpy.count(), 1);

    // Verify alarm was raised
    EXPECT_EQ(alarmSpy.count(), 1);

    // Verify alarm was persisted to database
    std::vector<AlarmSnapshot> alarms = service->getActiveAlarms();
    ASSERT_GT(alarms.size(), 0);

    QString alarmId = QString::fromStdString(alarms[0].alarmId);

    // Verify alarm exists in database
    auto dbAlarms = alarmRepo->getHistory("MRN-TEST-001", 0, LLONG_MAX);
    ASSERT_GT(dbAlarms.size(), 0);
    EXPECT_EQ(dbAlarms[0].alarmType, "HR_HIGH");
}

/**
 * @test Verify vital is cached correctly during workflow.
 */
TEST_F(MonitoringWorkflowTest, VitalCachedDuringWorkflow)
{
    // Start monitoring service
    ASSERT_TRUE(service->start());

    // Simulate sensor emitting vital
    VitalRecord vital = createVital("HR", 75.0);
    emit sensorDataSource->vitalSignsReceived(vital);

    // Wait for processing
    QTest::qWait(50);

    // Verify vital was cached
    const auto start = vital.timestampMs - 1000;
    const auto end = vital.timestampMs + 1000;
    auto cachedVitals = vitalsCache->getRange(start, end);
    ASSERT_GE(cachedVitals.size(), 1);
    const auto &last = cachedVitals.back();
    EXPECT_DOUBLE_EQ(last.value, 75.0);
    EXPECT_EQ(last.vitalType, std::string("HR"));
}

/**
 * @test Verify alarm acknowledge workflow persists to database.
 */
TEST_F(MonitoringWorkflowTest, AlarmAcknowledgeWorkflow)
{
    // Start monitoring service
    ASSERT_TRUE(service->start());

    // Raise an alarm
    VitalRecord vital = createVital("HR", 150.0);
    emit sensorDataSource->vitalSignsReceived(vital);
    QTest::qWait(50);

    // Get alarm ID
    std::vector<AlarmSnapshot> alarms = service->getActiveAlarms();
    ASSERT_GT(alarms.size(), 0);
    QString alarmId = QString::fromStdString(alarms[0].alarmId);

    // Acknowledge alarm
    bool success = service->acknowledgeAlarm(alarmId, "USER-001");
    ASSERT_TRUE(success);

    // Verify alarm status updated in database
    auto dbAlarms = alarmRepo->getHistory("MRN-TEST-001", 0, LLONG_MAX);
    ASSERT_GT(dbAlarms.size(), 0);

    // Find acknowledged alarm
    bool found = false;
    for (const auto &alarm : dbAlarms)
    {
        if (alarm.alarmId == alarmId.toStdString())
        {
            EXPECT_EQ(alarm.status, AlarmStatus::Acknowledged);
            found = true;
            break;
        }
    }
    EXPECT_TRUE(found);
}

/**
 * @test Verify sensor error handling doesn't crash the service.
 */
TEST_F(MonitoringWorkflowTest, SensorErrorHandling)
{
    // Start monitoring service
    ASSERT_TRUE(service->start());

    // Simulate sensor error
    SensorError error;
    error.code = SensorErrorCode::CommunicationError;
    error.message = QStringLiteral("Sensor connection lost");
    error.sensorType = QStringLiteral("ECG");
    error.timestamp = QDateTime::currentDateTimeUtc();
    error.recoverable = true;

    // Should not crash
    EXPECT_NO_THROW({
        emit sensorDataSource->sensorError(error);
        QTest::qWait(50);
    });
}

/**
 * @test Verify waveform samples are cached but not processed for alarms.
 */
TEST_F(MonitoringWorkflowTest, WaveformCachingWorkflow)
{
    // Start monitoring service
    ASSERT_TRUE(service->start());

    // Create signal spy (should NOT trigger any alarm signals)
    QSignalSpy alarmSpy(service.get(), &MonitoringService::alarmRaised);

    // Simulate waveform sample
    const auto nowMs = std::chrono::duration_cast<std::chrono::milliseconds>(
                           std::chrono::system_clock::now().time_since_epoch())
                           .count();
    WaveformSample sample = WaveformSample::ECGLeadII(0.5, nowMs, 250.0);

    emit sensorDataSource->waveformSampleReceived(sample);
    QTest::qWait(50);

    // Verify no alarms raised (waveforms don't trigger alarms)
    EXPECT_EQ(alarmSpy.count(), 0);

    // Verify waveform was cached
    auto cachedWaveforms = waveformCache->getLastSeconds(1);
    ASSERT_GE(cachedWaveforms.size(), 1);
    EXPECT_DOUBLE_EQ(cachedWaveforms.back().value, 0.5);
}

/**
 * @test Verify telemetry batching workflow.
 */
TEST_F(MonitoringWorkflowTest, TelemetryBatchingWorkflow)
{
    // Start monitoring service
    ASSERT_TRUE(service->start());

    // Create signal spy for telemetry batch ready
    QSignalSpy batchSpy(service.get(), &MonitoringService::telemetryBatchReady);

    // Process enough vitals to trigger batch flush (100 vitals)
    for (int i = 0; i < 100; ++i)
    {
        VitalRecord vital = createVital("HR", 75.0 + i);
        emit sensorDataSource->vitalSignsReceived(vital);
    }

    // Wait for batch processing
    QTest::qWait(100);

    // Verify batch was created and flushed
    EXPECT_GT(batchSpy.count(), 0);
}

/**
 * @test Verify multiple alarm types can be raised in same workflow.
 */
TEST_F(MonitoringWorkflowTest, MultipleAlarmTypesInWorkflow)
{
    // Start monitoring service
    ASSERT_TRUE(service->start());

    // Create signal spy
    QSignalSpy alarmSpy(service.get(), &MonitoringService::alarmRaised);

    // Emit HR alarm
    VitalRecord hrVital = createVital("HR", 150.0);
    emit sensorDataSource->vitalSignsReceived(hrVital);
    QTest::qWait(50);

    // Emit SPO2 alarm
    VitalRecord spo2Vital = createVital("SPO2", 80.0);
    emit sensorDataSource->vitalSignsReceived(spo2Vital);
    QTest::qWait(50);

    // Verify both alarms raised
    EXPECT_EQ(alarmSpy.count(), 2);

    // Verify both alarms persisted
    auto dbAlarms = alarmRepo->getHistory("MRN-TEST-001", 0, LLONG_MAX);
    EXPECT_GE(dbAlarms.size(), 2);
}

/**
 * @test Verify service stop flushes pending telemetry.
 */
TEST_F(MonitoringWorkflowTest, StopFlushesTelemtry)
{
    // Start monitoring service
    ASSERT_TRUE(service->start());

    // Create signal spy for telemetry batch ready
    QSignalSpy batchSpy(service.get(), &MonitoringService::telemetryBatchReady);

    // Process a few vitals (not enough to trigger automatic flush)
    for (int i = 0; i < 10; ++i)
    {
        VitalRecord vital = createVital("HR", 75.0);
        emit sensorDataSource->vitalSignsReceived(vital);
    }

    QTest::qWait(50);

    // Stop service (should flush pending batch)
    service->stop();

    // Verify batch was flushed on stop
    EXPECT_GT(batchSpy.count(), 0);
}
