/**
 * @file SettingsController.cpp
 * @brief Implementation of SettingsController for QML bindings.
 *
 * @author Z Monitor Team
 * @date 2025-01-15
 */

#include "SettingsController.h"
#include "infrastructure/adapters/SettingsManager.h"

#include <QVariant>

namespace ZMonitor {
namespace Interface {
namespace Controllers {

SettingsController::SettingsController(QObject* parent)
    : QObject(parent)
    , m_settingsManager(&Infrastructure::Adapters::SettingsManager::instance())
    , m_deviceLabel(m_settingsManager->deviceLabel())
    , m_measurementUnit(m_settingsManager->measurementUnit())
    , m_serverUrl(m_settingsManager->serverUrl())
    , m_useMockServer(m_settingsManager->useMockServer())
{
    // Connect to SettingsManager signals
    connect(m_settingsManager, &Infrastructure::Adapters::SettingsManager::settingChanged,
            this, &SettingsController::onSettingChanged);
}

QString SettingsController::deviceLabel() const
{
    return m_deviceLabel;
}

void SettingsController::setDeviceLabel(const QString& deviceLabel)
{
    if (m_deviceLabel != deviceLabel) {
        m_settingsManager->setDeviceLabel(deviceLabel);
        // m_deviceLabel will be updated via onSettingChanged signal
    }
}

QString SettingsController::measurementUnit() const
{
    return m_measurementUnit;
}

void SettingsController::setMeasurementUnit(const QString& unit)
{
    if (m_measurementUnit != unit) {
        m_settingsManager->setMeasurementUnit(unit);
        // m_measurementUnit will be updated via onSettingChanged signal
    }
}

QString SettingsController::serverUrl() const
{
    return m_serverUrl;
}

void SettingsController::setServerUrl(const QString& url)
{
    if (m_serverUrl != url) {
        m_settingsManager->setServerUrl(url);
        // m_serverUrl will be updated via onSettingChanged signal
    }
}

bool SettingsController::useMockServer() const
{
    return m_useMockServer;
}

void SettingsController::setUseMockServer(bool useMock)
{
    if (m_useMockServer != useMock) {
        m_settingsManager->setUseMockServer(useMock);
        // m_useMockServer will be updated via onSettingChanged signal
    }
}

void SettingsController::onSettingChanged(const QString& key, const QVariant& value)
{
    if (key == "deviceLabel") {
        m_deviceLabel = value.toString();
        emit deviceLabelChanged(m_deviceLabel);
    } else if (key == "measurementUnit") {
        m_measurementUnit = value.toString();
        emit measurementUnitChanged(m_measurementUnit);
    } else if (key == "serverUrl") {
        m_serverUrl = value.toString();
        emit serverUrlChanged(m_serverUrl);
    } else if (key == "useMockServer") {
        m_useMockServer = value.toBool();
        emit useMockServerChanged(m_useMockServer);
    }
}

} // namespace Controllers
} // namespace Interface
} // namespace ZMonitor

