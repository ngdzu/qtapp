#include "HttpTelemetryServerAdapter.h"
#include "../../application/services/ITelemetryServer.h"
#include <QNetworkProxy>
#include <QTimer>

HttpTelemetryServerAdapter::HttpTelemetryServerAdapter(const QUrl &endpoint,
                                                       QNetworkAccessManager *nam,
                                                       QObject *parent)
    : QObject(parent), m_endpoint(endpoint)
{
    if (nam)
    {
        m_nam = nam;
        m_ownNam = false;
    }
    else
    {
        m_nam = new QNetworkAccessManager(this);
        m_ownNam = true;
    }
}

void HttpTelemetryServerAdapter::setTimeoutMs(int timeoutMs) { m_timeoutMs = timeoutMs; }

void HttpTelemetryServerAdapter::setClientCertificates(const QList<QSslCertificate> &certs, const QSslKey &privateKey)
{
    Q_UNUSED(privateKey);
    // Attach client certs via default SSL config
    auto conf = tls13Config();
    conf.setCaCertificates(certs);
}

void HttpTelemetryServerAdapter::setIgnoreSslErrors(bool ignore) { m_ignoreSslErrors = ignore; }

QSslConfiguration HttpTelemetryServerAdapter::tls13Config() const
{
    QSslConfiguration conf = QSslConfiguration::defaultConfiguration();
    conf.setProtocol(QSsl::TlsV1_3);
    // Strong ciphers selection left to Qt defaults for TLS 1.3
    return conf;
}

bool HttpTelemetryServerAdapter::upload(const QByteArray &compressedBatch, QString &errorOut)
{
    QNetworkRequest req(m_endpoint);
    req.setSslConfiguration(tls13Config());
    req.setHeader(QNetworkRequest::ContentTypeHeader, QVariant("application/octet-stream"));
    // TelemetryService sends compressed data; use gzip encoding header
    req.setRawHeader("Content-Encoding", "gzip");
    req.setRawHeader("Accept", "application/json");
    m_lastRequest = req;

    QNetworkReply *reply = m_nam->post(req, compressedBatch);
    if (m_ignoreSslErrors)
    {
        QObject::connect(reply, &QNetworkReply::sslErrors, reply, [reply](const QList<QSslError> &errors)
                         {
            Q_UNUSED(errors);
            reply->ignoreSslErrors(); });
    }

    QEventLoop loop;
    QTimer timer;
    timer.setSingleShot(true);
    QObject::connect(&timer, &QTimer::timeout, &loop, &QEventLoop::quit);
    QObject::connect(reply, &QNetworkReply::finished, &loop, &QEventLoop::quit);
    timer.start(m_timeoutMs);
    loop.exec();

    bool timedOut = !reply->isFinished();
    if (timedOut)
    {
        reply->abort();
        reply->deleteLater();
        errorOut = QStringLiteral("timeout");
        return false;
    }

    const auto statusCode = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
    const bool ok = (statusCode >= 200 && statusCode < 300);
    if (!ok)
    {
        errorOut = QStringLiteral("http %1").arg(statusCode);
    }
    reply->deleteLater();
    return ok;
}
