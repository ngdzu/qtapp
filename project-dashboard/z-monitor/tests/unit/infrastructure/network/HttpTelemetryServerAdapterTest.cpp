#include <gtest/gtest.h>
#include <QtCore/QCoreApplication>
#include <QtNetwork/QNetworkRequest>
#include "src/infrastructure/network/HttpTelemetryServerAdapter.h"

TEST(HttpTelemetryServerAdapterTest, SetsGzipHeaderAndTls13)
{
    int argc = 0;
    char *argv[] = {nullptr};
    QCoreApplication app(argc, argv);

    QUrl endpoint("https://example.com/telemetry");
    HttpTelemetryServerAdapter adapter(endpoint);

    QByteArray payload("test-payload");
    // We don't actually need a working server; inspect last request after call
    QString err;
    adapter.upload(payload, err);

    QNetworkRequest req = adapter.lastRequest();
    auto enc = req.rawHeader("Content-Encoding");
    EXPECT_EQ(enc, QByteArray("gzip"));

    QSslConfiguration conf = req.sslConfiguration();
    EXPECT_EQ(conf.protocol(), QSsl::TlsV1_3);
}
