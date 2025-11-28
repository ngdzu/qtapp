/**
 * @file DeviceAggregate.h
 * @brief Domain aggregate representing device provisioning state and credential lifecycle.
 * 
 * This file contains the DeviceAggregate class which manages device provisioning state,
 * credential lifecycle, and firmware metadata. It enforces business invariants and
 * raises domain events (ProvisioningCompleted, ProvisioningFailed).
 * 
 * @author Z Monitor Team
 * @date 2025-01-15
 */

#pragma once

#include <string>
#include <cstdint>
#include <map>

namespace ZMonitor {
namespace Domain {
namespace Provisioning {

/**
 * @enum ProvisioningStatus
 * @brief Device provisioning status.
 */
enum class ProvisioningStatus {
    Unprovisioned,    ///< Device not yet provisioned
    Provisioning,     ///< Provisioning in progress
    Provisioned,      ///< Device successfully provisioned
    Failed            ///< Provisioning failed
};

/**
 * @class DeviceAggregate
 * @brief Domain aggregate managing device provisioning state and credential lifecycle.
 * 
 * This aggregate encapsulates device provisioning state, credentials, and firmware
 * metadata. It enforces business rules such as:
 * - Device can only be provisioned once
 * - Credentials must be valid before provisioning
 * - Firmware version must be compatible
 * 
 * The aggregate raises domain events (ProvisioningCompleted, ProvisioningFailed)
 * which are consumed by application services and UI controllers.
 * 
 * @note This is a domain aggregate - it has identity (device ID) and enforces invariants.
 * @note Domain layer has no Qt dependencies - uses standard C++ only.
 */
class DeviceAggregate {
public:
    /**
     * @brief Constructor.
     * 
     * Creates a new device aggregate in Unprovisioned state.
     * 
     * @param deviceId Device identifier
     */
    explicit DeviceAggregate(const std::string& deviceId);
    
    /**
     * @brief Destructor.
     */
    ~DeviceAggregate();
    
    /**
     * @brief Apply provisioning payload.
     * 
     * Applies provisioning configuration (certificates, server URL, etc.)
     * to the device. Transitions to Provisioning state.
     * 
     * @param serverUrl Central server URL
     * @param certificateData Client certificate data
     * @param privateKeyData Private key data
     * @param caCertificateData CA certificate data
     * @return true if provisioning payload applied successfully, false otherwise
     */
    bool applyProvisioningPayload(const std::string& serverUrl,
                                   const std::string& certificateData,
                                   const std::string& privateKeyData,
                                   const std::string& caCertificateData);
    
    /**
     * @brief Mark device as provisioned.
     * 
     * Transitions device to Provisioned state after successful provisioning.
     * Raises ProvisioningCompleted domain event.
     * 
     * @return true if marking succeeded, false if device not in Provisioning state
     */
    bool markProvisioned();
    
    /**
     * @brief Mark provisioning as failed.
     * 
     * Transitions device to Failed state. Raises ProvisioningFailed domain event.
     * 
     * @param errorMessage Error message describing failure
     * @return true if marking succeeded, false if device not in Provisioning state
     */
    bool markProvisioningFailed(const std::string& errorMessage);
    
    /**
     * @brief Rotate credentials.
     * 
     * Rotates device credentials (certificates, keys) for security.
     * 
     * @param newCertificateData New client certificate data
     * @param newPrivateKeyData New private key data
     * @return true if rotation succeeded, false if device not provisioned
     */
    bool rotateCredentials(const std::string& newCertificateData,
                           const std::string& newPrivateKeyData);
    
    /**
     * @brief Get device identifier.
     * 
     * @return Device identifier
     */
    const std::string& getDeviceId() const { return m_deviceId; }
    
    /**
     * @brief Get current provisioning status.
     * 
     * @return Current provisioning status
     */
    ProvisioningStatus getProvisioningStatus() const { return m_provisioningStatus; }
    
    /**
     * @brief Check if device is provisioned.
     * 
     * @return true if device is provisioned, false otherwise
     */
    bool isProvisioned() const { return m_provisioningStatus == ProvisioningStatus::Provisioned; }
    
    /**
     * @brief Get central server URL.
     * 
     * @return Server URL (empty if not provisioned)
     */
    const std::string& getServerUrl() const { return m_serverUrl; }
    
    /**
     * @brief Get firmware version.
     * 
     * @return Firmware version string
     */
    const std::string& getFirmwareVersion() const { return m_firmwareVersion; }
    
    /**
     * @brief Set firmware version.
     * 
     * @param version Firmware version string
     */
    void setFirmwareVersion(const std::string& version) { m_firmwareVersion = version; }
    
    /**
     * @brief Get device label.
     * 
     * @return Device label (human-readable identifier)
     */
    const std::string& getDeviceLabel() const { return m_deviceLabel; }
    
    /**
     * @brief Set device label.
     * 
     * @param label Device label
     */
    void setDeviceLabel(const std::string& label) { m_deviceLabel = label; }
    
    /**
     * @brief Get provisioning timestamp.
     * 
     * @return Timestamp when device was provisioned (0 if not provisioned)
     */
    int64_t getProvisionedAt() const { return m_provisionedAtMs; }

private:
    std::string m_deviceId;
    ProvisioningStatus m_provisioningStatus;
    std::string m_serverUrl;
    std::string m_certificateData;
    std::string m_privateKeyData;
    std::string m_caCertificateData;
    std::string m_firmwareVersion;
    std::string m_deviceLabel;
    int64_t m_provisionedAtMs;
    std::string m_errorMessage;
    
    /**
     * @brief Get current timestamp in milliseconds.
     * 
     * @return Current Unix timestamp in milliseconds
     */
    int64_t getCurrentTimestampMs() const;
    
    /**
     * @brief Validate provisioning payload.
     * 
     * @param serverUrl Server URL
     * @param certificateData Certificate data
     * @param privateKeyData Private key data
     * @param caCertificateData CA certificate data
     * @return true if payload is valid, false otherwise
     */
    bool validateProvisioningPayload(const std::string& serverUrl,
                                     const std::string& certificateData,
                                     const std::string& privateKeyData,
                                     const std::string& caCertificateData) const;
};

} // namespace Provisioning
} // namespace Domain
} // namespace ZMonitor

