#ifndef POWERMONITOR_H
#define POWERMONITOR_H

#include <Arduino.h>
#include <RemoteDebug.h>

extern RemoteDebug Debug;

class PowerMonitor {
public:
    // Constructor
    PowerMonitor(uint8_t currentPin, double currentCalibration, double voltage);

    // Setup function
    void setup();

    // Loop function
    void loop();

    // Accessor methods
    double getInstantaneousCurrent();     // Returns the instantaneous current in Amps
    double getInstantaneousPower();      // Returns the instantaneous power in Watts
    double getPowerConsumedLastHour();   // Returns power consumed in the last hour in kWh
    double getPowerConsumedToday();      // Returns power consumed today in kWh
    double getPowerConsumedYesterday();  // Returns power consumed yesterday in kWh

private:
    EnergyMonitor emon; // Instance of the EnergyMonitor class
    double _voltage;
    uint8_t _currentPin;
    double _currentCalibration;

    unsigned long _lastUpdateTime;  // Time of the last update in milliseconds
    unsigned long _hourStartTime;  // Start time of the current hour in milliseconds
    unsigned long _midnightTime;   // Start time of the current day in milliseconds

    double _powerConsumedThisHour;  // Energy consumed in the last hour in kWh
    double _powerConsumedToday;     // Energy consumed today in kWh
    double _powerConsumedYesterday; // Energy consumed yesterday in kWh
};

#endif // POWERMONITOR_H
