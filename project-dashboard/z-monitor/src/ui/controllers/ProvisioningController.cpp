/**
 * @file ProvisioningController.cpp
 * @brief Implementation of ProvisioningController.
 *
 * @author Z Monitor Team
 * @date 2025-11-29
 */

#include "ProvisioningController.h"

namespace zmon
{
    ProvisioningController::ProvisioningController(QObject *parent) : QObject(parent)
    {
        // TODO: Connect to provisioning service
    }

    void ProvisioningController::enterProvisioningMode()
    {
        // TODO: Enter provisioning mode (requires technician permission)
        m_provisioningState = "provisioning";
        emit provisioningStateChanged();
    }

    void ProvisioningController::exitProvisioningMode()
    {
        m_provisioningState = m_isProvisioned ? "provisioned" : "not_provisioned";
        emit provisioningStateChanged();
    }

    void ProvisioningController::generateQRCode()
    {
        // TODO: Generate QR code with device info
        emit qrCodeDataChanged();
    }

    void ProvisioningController::scanQRCode(const QString &qrData)
    {
        Q_UNUSED(qrData)
        // TODO: Parse QR code and provision device
    }
} // namespace zmon
