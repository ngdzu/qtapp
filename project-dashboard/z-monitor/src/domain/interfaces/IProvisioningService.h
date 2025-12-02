/**
 * @file IProvisioningService.h
 * @brief Interface for device and user provisioning.
 */
#pragma once

#include "domain/common/Result.h"
#include "domain/provisioning/DeviceAggregate.h"
#include <string>

namespace zmon
{

    class IProvisioningService
    {
    public:
        virtual ~IProvisioningService() = default;

        /**
         * @brief Provision a new device.
         * @param device Device aggregate to provision
         * @return Result<void> - Success or error details
         */
        virtual Result<void> provisionDevice(const DeviceAggregate &device) = 0;

        /**
         * @brief Update device provisioning status.
         * @param deviceId Device identifier
         * @param status New status value
         * @return Result<void> - Success or error details
         */
        virtual Result<void> updateDeviceStatus(const std::string &deviceId, const std::string &status) = 0;
    };

} // namespace zmon
