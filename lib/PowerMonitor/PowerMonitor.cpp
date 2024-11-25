#include "PowerMonitor.h"

#define MINUTES_IN_HOUR 60
#define RMS_SAMPLES 1480
#define ONE_MINUTE_MS 60000
#define ONE_HOUR_MS 3600000
#define ONE_DAY_MS 86400000
#define ADC_BITS 12
#define ADC_COUNTS  (1<<ADC_BITS)
#define INIT_VOLTAGE 239.0

#define IS_VALID_ADC_PIN(pin) ((pin) >= 1 && (pin) <= 34)

// Constructor
PowerMonitor::PowerMonitor()
    : _voltage(INIT_VOLTAGE), _currentPin(0), _currentCalibration(0.0),
      _powerConsumedToday(0.0), _powerConsumedYesterday(0.0), _powerConsumedTotal(0.0),
      _lastUpdateTime(0), _currentMinuteIndex(0) {
    memset(_minuteEnergy, 0, sizeof(_minuteEnergy)); // Initialize the buffer to 0
    u_long currentTime = millis();
    _lastMinuteTime =  currentTime;
    _midnightTime =  currentTime + ONE_DAY_MS;
    _offset = ADC_COUNTS>>1;
}

void PowerMonitor::setup(uint8_t currentPin, double currentCalibration, double voltage) {
    _voltage = voltage;
    setCurrentPin(currentPin);
    _currentCalibration = currentCalibration;
}

void PowerMonitor::setCurrentPin(int currentPin) {
    if (IS_VALID_ADC_PIN(currentPin)) {
        _currentPin = currentPin;
    }
}

// Loop function
void PowerMonitor::loop() {
    // Skip if current pin is not defined
    if (!IS_VALID_ADC_PIN(_currentPin)) return;

    u_long currentTime = millis();
    if (_lastUpdateTime == 0) {
        _lastUpdateTime = currentTime; // Initialize to avoid incorrect `diff`
        return; // Skip energy calculation on the first loop iteration
    }
    u_long diff = (currentTime - _lastUpdateTime) % UINT32_MAX;

    int lastMinuteIndex = MINUTES_IN_HOUR - 1;
    _instantaneousCurrent = calcIrms(RMS_SAMPLES); // Calculate Irms
    _instantaneousPower = _instantaneousCurrent * _voltage; // Calculate instantaneous power
    double energyConsumed = 0;

    if ((u_long)(currentTime - _lastMinuteTime) >= ONE_MINUTE_MS) { // 1 minute in milliseconds
        _lastMinuteTime = currentTime;
        int nextMinuteIndex = (_currentMinuteIndex + 1) % 60; // Advance minute index
        _powerConsumedLastHour = _powerConsumedLastHour - _minuteEnergy[nextMinuteIndex] + _minuteEnergy[_currentMinuteIndex];
        _currentMinuteIndex = nextMinuteIndex;
        _minuteEnergy[_currentMinuteIndex] = 0.0; // Reset this minute's energy
    }
    if (_currentMinuteIndex > 0) lastMinuteIndex = _currentMinuteIndex - 1;

    // Update power consumption
    energyConsumed = _instantaneousPower * diff / ONE_HOUR_MS; // Energy in Wh = W * (diff (ms) / ( 1000 * 60 * 60 = 3600000))
    _minuteEnergy[_currentMinuteIndex] += energyConsumed; // Accumulate for the current minute
    _powerConsumedToday += energyConsumed;
    _powerConsumedTotal += energyConsumed;

    _lastUpdateTime = currentTime;

    // Handle daily rollover
    if ((u_long)(currentTime - _midnightTime) >= 0) {
        _powerConsumedYesterday = _powerConsumedToday;
        _powerConsumedToday = 0.0;

        // Calculate the next midnight
        _midnightTime = currentTime + ONE_DAY_MS;
    }

    // Print instantaneous values
    debugD("Current (A): %lf, Power (W): %lf, diff: %u, energyConsumed: %lf, lastMinuteIndex: %i, _minuteEnergy: %lf, _powerConsumedLastHour: %lf", _instantaneousCurrent, _instantaneousPower, diff, energyConsumed, lastMinuteIndex, _minuteEnergy[lastMinuteIndex], _powerConsumedLastHour);
}

// Get instantaneous current
double PowerMonitor::getInstantaneousCurrent() {
    if ((_lastUpdateTime + 1000) < millis()) {
        debugD("Calling calcIrms...");
        return calcIrms(RMS_SAMPLES); // Return instantaneous current
    } else {
        return _instantaneousCurrent;
    }
}

// Get instantaneous power
double PowerMonitor::getInstantaneousPower() {
    if ((_lastUpdateTime + 1000) < millis()) {
        debugD("Calling calcIrms...");
        return calcIrms(RMS_SAMPLES) * _voltage; // Return instantaneous power
    } else {
        return _instantaneousPower;
    }
}

void PowerMonitor::setMidnightTime(time_t tm) {
    tmElements_t midnight;
    breakTime(tm, midnight);

    // Set the time to midnight
    midnight.Hour = 0;
    midnight.Minute = 0;
    midnight.Second = 0;

    // Calculate the next midnight
    time_t nextMidnight = makeTime(midnight);
    if (tm >= nextMidnight) {
        nextMidnight += SECS_PER_DAY;
    }

    _midnightTime = (nextMidnight - tm) * 1000; // Convert to milliseconds
    debugD("Next midnight calculated: %ld milliseconds from now", _midnightTime);
}

double PowerMonitor::calcIrms(uint16_t numSamples) {
    // Skip if current pin is not defined
    if (!IS_VALID_ADC_PIN(_currentPin)) return 0.0;

    debugV("Calculating root mean squared current (A)");
    double sum = 0;
    double SupplyVoltage = 3.3;

    for (uint16_t i = 0; i < numSamples; i++) {
        // Read raw current value
        int raw = analogRead(_currentPin);

        // Digital low pass filter extracts the 1.65 V dc _offset,
        //  then subtract this - signal is now centered on 0 counts.
        _offset = _offset + ((raw - _offset) / ADC_COUNTS);
        double filtered = raw - _offset;

        // Square the current value and accumulate
        sum += filtered * filtered;
    }

    // Apply calibration factor,
    double I_RATIO = _currentCalibration * (SupplyVoltage / ADC_COUNTS);
    // Calculate RMS value
    return I_RATIO * sqrt(sum / numSamples);
}
