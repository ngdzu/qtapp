/**
 * @file DeviceAggregate.cpp
 * @brief Implementation of DeviceAggregate domain aggregate.
 * 
 * @author Z Monitor Team
 * @date 2025-01-15
 */

#include "DeviceAggregate.h"
#include <chrono>

namespace zmon {
DeviceAggregate::DeviceAggregate(const std::string& deviceId)
    : m_deviceId(deviceId)
    , m_provisioningStatus(ProvisioningStatus::Unprovisioned)
    , m_serverUrl("")
    , m_certificateData("")
    , m_privateKeyData("")
    , m_caCertificateData("")
    , m_firmwareVersion("")
    , m_deviceLabel("")
    , m_provisionedAtMs(0)
    , m_errorMessage("")
{
}

DeviceAggregate::~DeviceAggregate() = default;

bool DeviceAggregate::applyProvisioningPayload(const std::string& serverUrl,
                                                const std::string& certificateData,
                                                const std::string& privateKeyData,
                                                const std::string& caCertificateData) {
    // Business rule: Device can only be provisioned once
    if (m_provisioningStatus == ProvisioningStatus::Provisioned) {
        return false;
    }
    
    // Validate provisioning payload
    if (!validateProvisioningPayload(serverUrl, certificateData, privateKeyData, caCertificateData)) {
        return false;
    }
    
    // Update state
    m_provisioningStatus = ProvisioningStatus::Provisioning;
    m_serverUrl = serverUrl;
    m_certificateData = certificateData;
    m_privateKeyData = privateKeyData;
    m_caCertificateData = caCertificateData;
    
    // Note: Domain event ProvisioningStarted would be raised here
    // (event publishing handled by application service)
    
    return true;
}

bool DeviceAggregate::markProvisioned() {
    if (m_provisioningStatus != ProvisioningStatus::Provisioning) {
        return false;
    }
    
    m_provisioningStatus = ProvisioningStatus::Provisioned;
    m_provisionedAtMs = getCurrentTimestampMs();
    m_errorMessage = "";
    
    // Note: Domain event ProvisioningCompleted would be raised here
    // (event publishing handled by application service)
    
    return true;
}

bool DeviceAggregate::markProvisioningFailed(const std::string& errorMessage) {
    if (m_provisioningStatus != ProvisioningStatus::Provisioning) {
        return false;
    }
    
    m_provisioningStatus = ProvisioningStatus::Failed;
    m_errorMessage = errorMessage;
    
    // Note: Domain event ProvisioningFailed would be raised here
    // (event publishing handled by application service)
    
    return true;
}

bool DeviceAggregate::rotateCredentials(const std::string& newCertificateData,
                                         const std::string& newPrivateKeyData) {
    // Business rule: Can only rotate credentials if device is provisioned
    if (m_provisioningStatus != ProvisioningStatus::Provisioned) {
        return false;
    }
    
    // Validate new credentials
    if (newCertificateData.empty() || newPrivateKeyData.empty()) {
        return false;
    }
    
    m_certificateData = newCertificateData;
    m_privateKeyData = newPrivateKeyData;
    
    return true;
}

int64_t DeviceAggregate::getCurrentTimestampMs() const {
    auto now = std::chrono::system_clock::now();
    auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(
        now.time_since_epoch());
    return ms.count();
}

bool DeviceAggregate::validateProvisioningPayload(const std::string& serverUrl,
                                                    const std::string& certificateData,
                                                    const std::string& privateKeyData,
                                                    const std::string& caCertificateData) const {
    // Basic validation: all fields must be non-empty
    if (serverUrl.empty() || certificateData.empty() || 
        privateKeyData.empty() || caCertificateData.empty()) {
        return false;
    }
    
    // Validate server URL format (basic check)
    if (serverUrl.find("http://") != 0 && serverUrl.find("https://") != 0) {
        return false;
    }
    
    // In real implementation, validate certificate format, key format, etc.
    
    return true;
}

} // namespace zmon