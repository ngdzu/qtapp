// Simplified telemetry workflow test using current ITelemetryServer API
#include <gtest/gtest.h>
#include <QCoreApplication>
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
            Q_UNUSED(errorOut);
            lastBatch = compressedBatch;
            uploads++;
            return true;
        }
        int uploads{0};
        QByteArray lastBatch;
    };

} // namespace

TEST(TelemetryWorkflowTest, DISABLED_EndToEndBatchUpload)
{
    int argc = 0;
    char *argv[] = {nullptr};
    QCoreApplication app(argc, argv);

    MockTelemetryServer server;
    zmon::TelemetryService service(&server);
    service.setBatchIntervalMs(20);
    service.enqueueVital("vital:HR=80");
    service.start();
    QTest::qWait(50);
    service.stop();

    ASSERT_GE(server.uploads, 1);
    ASSERT_FALSE(server.lastBatch.isEmpty());
}
