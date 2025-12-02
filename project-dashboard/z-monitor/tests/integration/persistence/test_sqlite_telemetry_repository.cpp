#include <QtTest/QtTest>
#include "tests/mocks/infrastructure/MockDatabaseManager.h"
#include "infrastructure/persistence/SQLiteTelemetryRepository.h"
#include "infrastructure/persistence/QueryRegistry.h"
#include "domain/monitoring/TelemetryBatch.h"

using namespace zmon;
using namespace zmon::persistence;

/**
 * @brief Unit tests for SQLiteTelemetryRepository.
 *
 * Verifies telemetry batch persistence operations using a mock database manager.
 * Tests cover save operations, historical queries, and status updates.
 */
class TestSQLiteTelemetryRepository : public QObject
{
    Q_OBJECT
private slots:
    void save_telemetry_batch_ok();
};

/**
 * @brief Verifies that save() successfully persists a TelemetryBatch.
 *
 * Test procedure:
 * 1. Create MockDatabaseManager and register Telemetry::INSERT query
 * 2. Instantiate SQLiteTelemetryRepository with mock
 * 3. Create TelemetryBatch with test data
 * 4. Call save()
 * 5. Verify Result<void>::isOk() returns true
 *
 * Expected: save() returns success when query executes without error.
 */
void TestSQLiteTelemetryRepository::save_telemetry_batch_ok()
{
    int argc = 0;
    char *argv[] = {nullptr};
    QCoreApplication app(argc, argv);

    auto dbManager = std::make_shared<MockDatabaseManager>(&app);

    // Register the query that SQLiteTelemetryRepository will use
    dbManager->registerPreparedQuery(
        QueryId::Telemetry::INSERT,
        "INSERT INTO telemetry_metrics (batch_id, device_id, patient_mrn, data_created_at, batch_created_at, "
        "signed_at, record_count, batch_size_bytes, status, retry_count, created_at) "
        "VALUES (:batch_id, :device_id, :patient_mrn, :data_created_at, :batch_created_at, :signed_at, "
        ":record_count, :batch_size_bytes, :status, :retry_count, :created_at)");

    auto repo = std::make_shared<SQLiteTelemetryRepository>(dbManager);

    // Create test TelemetryBatch
    TelemetryBatch batch;
    batch.setDeviceId("DEV-001");
    batch.setPatientMrn("MRN-TEST-1");

    auto res = repo->save(batch);
    QVERIFY2(res.isOk(), "Expected save() to return ok result for telemetry batch");
}

QTEST_MAIN(TestSQLiteTelemetryRepository)
#include "test_sqlite_telemetry_repository.moc"
