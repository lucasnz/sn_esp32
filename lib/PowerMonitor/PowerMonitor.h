#ifndef POWERMONITOR_H
#define POWERMONITOR_H

#include <Arduino.h>
#include <TimeLib.h>
#include <RemoteDebug.h>

extern RemoteDebug Debug;

class PowerMonitor {
public:
    PowerMonitor();

    void setup(uint8_t currentPin, double currentCalibration, double voltage);
    void loop();

    double getInstantaneousCurrent();
    double getInstantaneousPower();
    double getPowerConsumerLastHour() { return _powerConsumedLastHour; };
    double getPowerConsumedToday() { return _powerConsumedToday; };
    double getPowerConsumedYesterday() { return _powerConsumedYesterday; };
    double getPowerConsumedTotal() { return _powerConsumedTotal; };
    void setMidnightTime(time_t tm);
    void setVoltage(int v) { _voltage = v; };
    void setCurrentPin(int p);
    void setCurrentCalibration(int c) { _currentCalibration = c; };

private:
    double _offset;
    double _voltage;
    uint8_t _currentPin;
    double _currentCalibration;

    unsigned long _lastUpdateTime;
    double _instantaneousCurrent;
    double _instantaneousPower;
    unsigned long _lastMinuteTime;
    unsigned long _midnightTime;

    double _minuteEnergy[60]; // Circular buffer to store energy for the last 60 minutes
    int _currentMinuteIndex; // Index of the current minute in the buffer
    double _powerConsumedLastHour;
    double _powerConsumedToday;
    double _powerConsumedYesterday;
    double _powerConsumedTotal;

    double calcIrms(uint16_t numSamples);
    double calculateLastHourEnergy(); // Helper function to calculate the rolling hour energy
};

#endif // POWERMONITOR_H
