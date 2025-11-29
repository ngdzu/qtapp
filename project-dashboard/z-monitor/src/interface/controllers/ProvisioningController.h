/**
 * @file ProvisioningController.h
 * @brief QML controller for device provisioning UI.
 *
 * @author Z Monitor Team
 * @date 2025-11-29
 */

#pragma once

#include <QObject>
#include <QString>

namespace zmon
{
    /**
     * @class ProvisioningController
     * @brief QML controller for device provisioning.
     *
     * @thread Main/UI Thread
     * @ingroup Interface
     */
    class ProvisioningController : public QObject
    {
        Q_OBJECT

        Q_PROPERTY(QString provisioningState READ provisioningState NOTIFY provisioningStateChanged)
        Q_PROPERTY(QString qrCodeData READ qrCodeData NOTIFY qrCodeDataChanged)
        Q_PROPERTY(bool isProvisioned READ isProvisioned NOTIFY isProvisionedChanged)
        Q_PROPERTY(QString deviceId READ deviceId NOTIFY deviceIdChanged)

    public:
        explicit ProvisioningController(QObject *parent = nullptr);
        ~ProvisioningController() override = default;

        QString provisioningState() const { return m_provisioningState; }
        QString qrCodeData() const { return m_qrCodeData; }
        bool isProvisioned() const { return m_isProvisioned; }
        QString deviceId() const { return m_deviceId; }

        Q_INVOKABLE void enterProvisioningMode();
        Q_INVOKABLE void exitProvisioningMode();
        Q_INVOKABLE void generateQRCode();
        Q_INVOKABLE void scanQRCode(const QString &qrData);

    signals:
        void provisioningStateChanged();
        void qrCodeDataChanged();
        void isProvisionedChanged();
        void deviceIdChanged();

    private:
        QString m_provisioningState{"not_provisioned"};
        QString m_qrCodeData;
        bool m_isProvisioned{false};
        QString m_deviceId;
    };
} // namespace zmon
