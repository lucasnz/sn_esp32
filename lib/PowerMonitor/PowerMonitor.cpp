#include "PowerMonitor.h"

// Constructor
PowerMonitor::PowerMonitor(uint8_t currentPin, double currentCalibration, double voltage)
    : _voltage(voltage), _currentPin(currentPin), _currentCalibration(currentCalibration),
      _powerConsumedThisHour(0.0), _powerConsumedToday(0.0), _powerConsumedYesterday(0.0),
      _lastUpdateTime(0), _hourStartTime(0), _midnightTime(0) {}

// Setup function
void PowerMonitor::setup() {
    Serial.begin(9600);
    emon.current(_currentPin, _currentCalibration); // Initialize the current sensor
    _lastUpdateTime = millis();
    _hourStartTime = millis();
    _midnightTime = millis();
}

// Loop function
void PowerMonitor::loop() {
    return;
    unsigned long currentTime = millis();
    debugD("Reading current...");
    double Irms = emon.calcIrms(1480); // Calculate Irms
    double power = Irms * _voltage; // Calculate instantaneous power

    // Print instantaneous values
    debugD("Current (A): %d, Power (W): %d", Irms, power);

    // Update power consumption
    if (currentTime - _lastUpdateTime >= 1000) { // Update every second
        double energyConsumed = (power / 3600.0) / 1000.0; // Convert W to kWh
        _powerConsumedThisHour += energyConsumed;
        _powerConsumedToday += energyConsumed;
        _lastUpdateTime = currentTime;
    }

    // Handle hourly rollover
    if (currentTime - _hourStartTime >= 3600000) { // 1 hour in milliseconds
        _powerConsumedThisHour = 0.0;
        _hourStartTime = currentTime;
    }

    // Handle daily rollover
    if (currentTime - _midnightTime >= 86400000) { // 24 hours in milliseconds
        _powerConsumedYesterday = _powerConsumedToday;
        _powerConsumedToday = 0.0;
        _midnightTime = currentTime;
    }
}

// Get instantaneous current
double PowerMonitor::getInstantaneousCurrent() {
    return emon.calcIrms(1480); // Return instantaneous current
}

// Get instantaneous power
double PowerMonitor::getInstantaneousPower() {
    return emon.calcIrms(1480) * _voltage; // Return instantaneous power
}

// Get power consumed in the last hour
double PowerMonitor::getPowerConsumedLastHour() {
    return _powerConsumedThisHour;
}

// Get power consumed today
double PowerMonitor::getPowerConsumedToday() {
    return _powerConsumedToday;
}

// Get power consumed yesterday
double PowerMonitor::getPowerConsumedYesterday() {
    return _powerConsumedYesterday;
}
