#pragma once

#include <QObject>
#include <QByteArray>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QSslConfiguration>
#include <QSsl>
#include "../../application/services/ITelemetryServer.h"

/**
 * @brief HTTP/HTTPS adapter implementing zmon::ITelemetryServer.
 *
 * Uses QNetworkAccessManager to POST telemetry batches to a central server.
 * Configures TLS 1.3 and sets gzip Content-Encoding. Timeouts are enforced
 * with a local event loop and abort semantics.
 */
class HttpTelemetryServerAdapter : public QObject, public zmon::ITelemetryServer
{
    Q_OBJECT

public:
    /**
     * @brief Construct adapter for the given endpoint.
     * @param endpoint Target URL (http or https). For production, https is required.
     * @param nam Optional shared QNetworkAccessManager, or null to create one.
     * @param parent Parent QObject.
     */
    explicit HttpTelemetryServerAdapter(const QUrl &endpoint,
                                        QNetworkAccessManager *nam = nullptr,
                                        QObject *parent = nullptr);

    /**
     * @brief Set request timeout in milliseconds.
     * @param timeoutMs Timeout duration; default is 15000ms.
     */
    void setTimeoutMs(int timeoutMs);

    /**
     * @brief Configure client certificates for TLS connections.
     * @param certs List of CA/client certificates.
     * @param privateKey Private key (if client auth is used).
     */
    void setClientCertificates(const QList<QSslCertificate> &certs, const QSslKey &privateKey);

    /**
     * @brief Ignore SSL errors (development only).
     * @param ignore True to ignore SSL errors.
     */
    void setIgnoreSslErrors(bool ignore);

    /**
     * @brief Upload a compressed telemetry batch.
     * @param compressedBatch zlib/gzip-compressed payload from TelemetryService.
     * @param errorOut Error description when false is returned.
     * @return True on 2xx response, false otherwise.
     */
    bool upload(const QByteArray &compressedBatch, QString &errorOut) override;

    /**
     * @brief Last HTTP request used (for testing/inspection).
     */
    QNetworkRequest lastRequest() const { return m_lastRequest; }

private:
    QUrl m_endpoint;
    QNetworkAccessManager *m_nam;
    bool m_ownNam{false};
    int m_timeoutMs{15000};
    bool m_ignoreSslErrors{false};
    QNetworkRequest m_lastRequest;

    /**
     * @brief TLS 1.3 configuration for HTTPS connections.
     */
    QSslConfiguration tls13Config() const;
};
