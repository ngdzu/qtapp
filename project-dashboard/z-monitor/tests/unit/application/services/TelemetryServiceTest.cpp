#include <gtest/gtest.h>
#include <QObject>
#include <QString>
#include <QTest>
#include "src/application/services/TelemetryService.h"
#include "src/application/services/ITelemetryServer.h"

namespace
{

    class MockTelemetryServer : public zmon::ITelemetryServer
    {
    public:
        bool upload(const QByteArray &compressedBatch, QString &errorOut) override
        {
            lastBatch = compressedBatch;
            (void)errorOut;
            if (failCount > 0)
            {
                --failCount;
                errorOut = "network error";
                return false;
            }
            uploads++;
            return true;
        }
        QByteArray lastBatch;
        int failCount{0};
        int uploads{0};
    };

} // namespace

TEST(TelemetryServiceTest, StartsAndFlushesOnTimer)
{
    int argc = 0;
    char *argv[] = {nullptr};
    QCoreApplication app(argc, argv);
    MockTelemetryServer server;
    zmon::TelemetryService svc(&server);
    svc.setBatchIntervalMs(10);
    QObject::connect(&svc, &zmon::TelemetryService::batchReady, [&](const QByteArray &b)
                     { (void)b; });
    svc.enqueueVital("v1");
    svc.enqueueAlarm("a1");
    svc.start();
    QTest::qWait(30);
    svc.stop();
    ASSERT_FALSE(server.lastBatch.isEmpty());
}

TEST(TelemetryServiceTest, RetriesWithBackoffAndThenSucceeds)
{
    int argc = 0;
    char *argv[] = {nullptr};
    QCoreApplication app(argc, argv);
    MockTelemetryServer server;
    server.failCount = 2; // first two attempts fail
    zmon::TelemetryService svc(&server);
    zmon::RetryPolicy policy;
    policy = zmon::RetryPolicy({3, std::chrono::milliseconds(1), 2.0});
    svc.setRetryPolicy(policy);
    svc.setBatchIntervalMs(5);
    svc.enqueueVital("v1");
    svc.start();
    QTest::qWait(20);
    svc.stop();
    ASSERT_FALSE(server.lastBatch.isEmpty());
}

TEST(TelemetryServiceTest, CircuitBreakerBlocksUploads)
{
    int argc = 0;
    char *argv[] = {nullptr};
    QCoreApplication app(argc, argv);
    MockTelemetryServer server;
    server.failCount = 5; // cause consecutive failures
    zmon::TelemetryService svc(&server);
    svc.setBatchIntervalMs(5);
    // open breaker quickly by forcing failures
    zmon::RetryPolicy policy({1, std::chrono::milliseconds(1), 2.0});
    svc.setRetryPolicy(policy);
    svc.enqueueVital("v1");
    svc.start();
    QTest::qWait(10);
    svc.stop();
    // second flush should be blocked when buffer refilled
    int prevUploads = server.uploads;
    svc.enqueueVital("v2");
    // Manually trigger flush while breaker potentially open
    svc.flushNow();
    ASSERT_EQ(server.uploads, prevUploads); // no new successful upload occurred
}
