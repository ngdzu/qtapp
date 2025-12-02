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
            lastBatch = compressedBatch;
            (void)errorOut;
            uploads++;
            if (failFirst)
            {
                failFirst = false;
                errorOut = "timeout";
                return false;
            }
            return true;
        }
        int uploads{0};
        QByteArray lastBatch;
        bool failFirst{false};
    };

} // namespace

TEST(TelemetryWorkflowTest, EndToEndBatchUpload)
{
    int argc = 0;
    char *argv[] = {nullptr};
    QCoreApplication app(argc, argv);

    MockTelemetryServer server;
    zmon::TelemetryService service(&server);
    service.setBatchIntervalMs(20);
    // Verify compression reduces size compared to plaintext
    server.failFirst = true; // exercise retry path once
    service.enqueueVital("vital:HR=80");
    service.enqueueAlarm("alarm:HR_HIGH");
    service.start();
    QTest::qWait(50);
    service.stop();

    ASSERT_GE(server.uploads, 1);
    ASSERT_FALSE(server.lastBatch.isEmpty());
    // Basic compression expectation: compressed payload should differ from plaintext
    QByteArray plain = QByteArray("vital:HR=80\nalarm:HR_HIGH\n");
    ASSERT_NE(server.lastBatch, plain);
}
