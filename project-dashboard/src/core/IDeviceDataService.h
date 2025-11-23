#pragma once

#include <QObject>

struct DeviceStats
{
    int heartRate;    // bpm
    int oxygenLevel;  // %
    int batteryLevel; // %
    int temperature;  // Celsius
    bool isConnected;
};

Q_DECLARE_METATYPE(DeviceStats)

class IDeviceDataService : public QObject
{
    Q_OBJECT
public:
    explicit IDeviceDataService(QObject *parent = nullptr) : QObject(parent) {}
    virtual ~IDeviceDataService() = default;
    virtual void start() = 0;
    virtual void stop() = 0;
signals:
    void statsUpdated(const DeviceStats &stats);
};
