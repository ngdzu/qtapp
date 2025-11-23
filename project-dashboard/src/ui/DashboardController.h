#pragma once

#include <QObject>
#include <QVector>
#include "core/IDeviceDataService.h"

class DashboardController : public QObject
{
    Q_OBJECT
    Q_PROPERTY(int heartRate READ heartRate NOTIFY statsChanged)
    Q_PROPERTY(int oxygenLevel READ oxygenLevel NOTIFY statsChanged)
    Q_PROPERTY(int batteryLevel READ batteryLevel NOTIFY statsChanged)
    Q_PROPERTY(int temperature READ temperature NOTIFY statsChanged)
    Q_PROPERTY(bool isConnected READ isConnected NOTIFY statsChanged)
    Q_PROPERTY(QList<int> heartRateHistory READ heartRateHistory NOTIFY historyChanged)
    Q_PROPERTY(QList<int> oxygenHistory READ oxygenHistory NOTIFY historyChanged)

public:
    explicit DashboardController(QObject *parent = nullptr);

    void setService(IDeviceDataService *service);

    int heartRate() const { return m_currentStats.heartRate; }
    int oxygenLevel() const { return m_currentStats.oxygenLevel; }
    int batteryLevel() const { return m_currentStats.batteryLevel; }
    int temperature() const { return m_currentStats.temperature; }
    bool isConnected() const { return m_currentStats.isConnected; }

    QList<int> heartRateHistory() const { return m_hrHistory; }
    QList<int> oxygenHistory() const { return m_o2History; }

signals:
    void statsChanged();
    void historyChanged();

private slots:
    void onStatsUpdated(const DeviceStats &stats);

private:
    IDeviceDataService *m_service;
    DeviceStats m_currentStats;
    QList<int> m_hrHistory;
    QList<int> m_o2History;

    static const int MAX_HISTORY = 20;
};
