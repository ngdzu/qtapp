#include <QtTest/QtTest>
#include "tests/mocks/infrastructure/MockDatabaseManager.h"
#include "infrastructure/persistence/SQLiteVitalsRepository.h"
#include "infrastructure/persistence/QueryRegistry.h"
#include "domain/monitoring/VitalRecord.h"

using namespace zmon;
using namespace zmon::persistence;

class TestSQLiteVitalsRepository : public QObject {
    Q_OBJECT
private slots:
    void save_single_vital_ok();
};

void TestSQLiteVitalsRepository::save_single_vital_ok()
{
    int argc = 0; char* argv[] = {nullptr};
    QCoreApplication app(argc, argv);

    auto dbManager = std::make_shared<MockDatabaseManager>(&app);
    
    // Register the query that SQLiteVitalsRepository will use
    dbManager->registerPreparedQuery(
        QueryId::Vitals::INSERT,
        "INSERT INTO vitals (patient_mrn, timestamp, heart_rate, spo2, respiration_rate, signal_quality, source, is_synced) "
        "VALUES (:patient_mrn, :timestamp, :heart_rate, :spo2, :respiration_rate, :signal_quality, :source, :is_synced)"
    );

    auto repo = std::make_shared<SQLiteVitalsRepository>(dbManager);

    VitalRecord vital{"HR", 72.0, static_cast<int64_t>(QDateTime::currentMSecsSinceEpoch()), 90, "MRN-TEST-1", "DEV-001"};
    auto res = repo->save(vital);
    QVERIFY2(res.isOk(), "Expected save() to return ok result for single vital");
}

QTEST_MAIN(TestSQLiteVitalsRepository)
#include "test_sqlite_vitals_repository.moc"
