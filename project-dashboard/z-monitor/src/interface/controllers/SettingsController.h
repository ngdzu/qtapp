/**
 * @file SettingsController.h
 * @brief QML controller for device settings UI.
 *
 * This file contains the SettingsController class which provides QML bindings
 * for device settings including device label, measurement units, and server
 * configuration.
 *
 * @author Z Monitor Team
 * @date 2025-01-15
 */

#pragma once

#include <QObject>
#include <QString>

namespace ZMonitor {
namespace Infrastructure {
namespace Adapters {
    class SettingsManager;
}
}

namespace ZMonitor {
namespace Interface {
namespace Controllers {

/**
 * @class SettingsController
 * @brief QML controller for device settings UI.
 *
 * Provides QML bindings for device settings. Exposes device configuration
 * properties and methods for the Settings View.
 *
 * @note This controller does NOT handle network settings (handled by ProvisioningController).
 *
 * @ingroup Interface
 */
class SettingsController : public QObject {
    Q_OBJECT

    Q_PROPERTY(QString deviceLabel READ deviceLabel WRITE setDeviceLabel NOTIFY deviceLabelChanged)
    Q_PROPERTY(QString measurementUnit READ measurementUnit WRITE setMeasurementUnit NOTIFY measurementUnitChanged)
    Q_PROPERTY(QString serverUrl READ serverUrl WRITE setServerUrl NOTIFY serverUrlChanged)
    Q_PROPERTY(bool useMockServer READ useMockServer WRITE setUseMockServer NOTIFY useMockServerChanged)

public:
    /**
     * @brief Constructor.
     *
     * @param parent Parent QObject
     */
    explicit SettingsController(QObject* parent = nullptr);

    /**
     * @brief Destructor.
     */
    ~SettingsController() override = default;

    /**
     * @brief Gets the device label (asset tag).
     *
     * @return Device label string
     */
    QString deviceLabel() const;

    /**
     * @brief Sets the device label (asset tag).
     *
     * @param deviceLabel Device label string
     */
    void setDeviceLabel(const QString& deviceLabel);

    /**
     * @brief Gets the measurement unit preference.
     *
     * @return "metric" or "imperial"
     */
    QString measurementUnit() const;

    /**
     * @brief Sets the measurement unit preference.
     *
     * @param unit "metric" or "imperial"
     */
    void setMeasurementUnit(const QString& unit);

    /**
     * @brief Gets the central server URL.
     *
     * @return Server URL string
     */
    QString serverUrl() const;

    /**
     * @brief Sets the central server URL.
     *
     * @param url Server URL string
     */
    void setServerUrl(const QString& url);

    /**
     * @brief Gets whether to use mock server.
     *
     * @return true if mock server enabled, false otherwise
     */
    bool useMockServer() const;

    /**
     * @brief Sets whether to use mock server.
     *
     * @param useMock true to enable mock server, false to disable
     */
    void setUseMockServer(bool useMock);

signals:
    /**
     * @brief Emitted when device label changes.
     *
     * @param deviceLabel New device label
     */
    void deviceLabelChanged(const QString& deviceLabel);

    /**
     * @brief Emitted when measurement unit changes.
     *
     * @param unit New measurement unit
     */
    void measurementUnitChanged(const QString& unit);

    /**
     * @brief Emitted when server URL changes.
     *
     * @param url New server URL
     */
    void serverUrlChanged(const QString& url);

    /**
     * @brief Emitted when mock server setting changes.
     *
     * @param useMock New mock server setting
     */
    void useMockServerChanged(bool useMock);

private slots:
    /**
     * @brief Handles setting changes from SettingsManager.
     *
     * @param key Setting key
     * @param value Setting value
     */
    void onSettingChanged(const QString& key, const QVariant& value);

private:
    Infrastructure::Qt::SettingsManager* m_settingsManager;
    QString m_deviceLabel;
    QString m_measurementUnit;
    QString m_serverUrl;
    bool m_useMockServer;
};

} // namespace Controllers
} // namespace Interface
} // namespace ZMonitor

