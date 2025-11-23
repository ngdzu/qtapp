#include <QtTest>
#include "../src/core/MockDeviceDataService.h"

class TestDataGenerator : public QObject
{
    Q_OBJECT

private slots:
    void testHeartRateRange();
    void testOxygenLevelRange();
};

void TestDataGenerator::testHeartRateRange()
{
    MockDeviceDataService service;
    QSignalSpy spy(&service, &MockDeviceDataService::statsUpdated);

    service.start();

    // Wait for a few updates
    QTRY_VERIFY_WITH_TIMEOUT(spy.count() > 0, 2000);

    for (int i = 0; i < spy.count(); ++i)
    {
        QList<QVariant> args = spy.takeFirst();
        DeviceStats stats = args.at(0).value<DeviceStats>();

        QVERIFY(stats.heartRate >= 40);
        QVERIFY(stats.heartRate <= 200);
    }
}

void TestDataGenerator::testOxygenLevelRange()
{
    MockDeviceDataService service;
    QSignalSpy spy(&service, &MockDeviceDataService::statsUpdated);

    service.start();

    QTRY_VERIFY_WITH_TIMEOUT(spy.count() > 0, 2000);

    for (int i = 0; i < spy.count(); ++i)
    {
        QList<QVariant> args = spy.takeFirst();
        DeviceStats stats = args.at(0).value<DeviceStats>();

        QVERIFY(stats.oxygenLevel >= 0);
        QVERIFY(stats.oxygenLevel <= 100);
    }
}

QTEST_MAIN(TestDataGenerator)
#include "tst_datagenerator.moc"
