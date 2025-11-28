/**
 * @file ISensorDataSource.h
 * @brief Interface for sensor data source (forward declaration stub).
 * 
 * This is a placeholder for the ISensorDataSource interface which will be
 * defined in the domain layer. For now, this allows MonitoringService to compile.
 * 
 * @note This will be moved to domain layer when fully implemented.
 */

#pragma once

namespace ZMonitor {
namespace Application {
namespace Services {

/**
 * @class ISensorDataSource
 * @brief Interface for sensor data source (placeholder).
 * 
 * This interface will be defined in the domain layer to abstract sensor
 * data acquisition (simulator, hardware, mock).
 */
class ISensorDataSource {
public:
    virtual ~ISensorDataSource() = default;
};

} // namespace Services
} // namespace Application
} // namespace ZMonitor

