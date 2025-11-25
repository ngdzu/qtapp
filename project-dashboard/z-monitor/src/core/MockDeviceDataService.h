#pragma once

#include "IDeviceDataService.h"
#include <QTimer>
#include <random>

class MockDeviceDataService : public IDeviceDataService
{
    Q_OBJECT
public:
    explicit MockDeviceDataService(QObject *parent = nullptr);
    void start() override;
    void stop() override;

private slots:
    void generateData();

private:
    QTimer *m_timer;
    std::mt19937 m_rng;

    // Simulation state
    double m_time;
    int m_battery;
};
