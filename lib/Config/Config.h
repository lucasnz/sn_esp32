#ifndef CONFIG_H
#define CONFIG_H

#include <Preferences.h>
#include <Arduino.h>
#include <ArduinoJson.h>
#include <RemoteDebug.h>

extern RemoteDebug Debug;

template <typename T>
class Setting {
private:
    T _value;
    const char* _name;
    static void (*_settingCallback)(const char*, T);

public:
    Setting(const char* name, T initialValue = T()) : _name(name), _value(initialValue) {}

    T getValue() { return _value; }
    void setValue(T newval) {
        T oldvalue = _value;
        _value = newval;
        if ((_settingCallback) && (oldvalue != newval)) {
                _settingCallback(_name, _value);
        }
    }

    static void setCallback(void (*callback)(const char*, T)) {
        _settingCallback = callback;
    }
};

template<>
class Setting<int> {
private:
    int _value;
    const char* _name;
    static void (*_settingCallback)(const char*, int);

    int _minValue;
    int _maxValue;

public:
    Setting(const char* name, int initialValue, int minValue = 10, int maxValue = 3600)
        : _name(name), _value(initialValue), _minValue(minValue), _maxValue(maxValue) {}

    int getValue() { return _value; }

    void setValue(int newval) {
        int oldvalue = _value;
        _value = (newval < _minValue) ? _minValue : (newval > _maxValue) ? _maxValue : newval;
        if ((_settingCallback) && (oldvalue != _value)) {
            _settingCallback(_name, _value);
        }
    }

    static void setCallback(void (*callback)(const char*, int)) {
        _settingCallback = callback;
    }
};

/// @brief represents the controller configuration options to be stored.
class ControllerConfig {
public:
    Setting<String> MqttServer = Setting<String>("MqttServer", "mqtt");
    Setting<int> MqttPort = Setting<int>("MqttPort", 1883, 1, 65535);
    Setting<String> MqttUsername = Setting<String>("MqttUsername");
    Setting<String> MqttPassword = Setting<String>("MqttPassword");
    Setting<String> SpaName = Setting<String>("SpaName", "eSpa");
    Setting<int> spaPollFreq = Setting<int>("spaPollFrequency", 60, 10, 300);
    Setting<int> fwPollFreq = Setting<int>("firmwareUpdateCheckFrequency", 24, 0, 1000);

    // The following settings aren't saved but are used as in memory storage
    Setting<String> latestVersion = Setting<String>("latestVersion", "unknown");
    Setting<int> updateAvailable = Setting<int>("updateAvailable", 0, 0, 1);
    Setting<String> releaseNotes = Setting<String>("releaseNotes", "unknown");
    Setting<String> releaseUrl = Setting<String>("releaseUrl", "unknown");
    Setting<String> firmwareUrl = Setting<String>("firmwareUrl");
    Setting<String> spiffsUrl = Setting<String>("spiffsUrl");
    Setting<int> updateInProgress = Setting<int>("updateInProgress", 0, 0, 1);
    Setting<int> updatePercentage = Setting<int>("updatePercentage", 0, 0, 100);
    Setting<String> updateStatus = Setting<String>("updateStatus", "idle");
};

class Config : public ControllerConfig {
public:
    // Constructor
    Config();

    bool readConfig();              // Read configuration from Preferences or file
    void writeConfig();             // Write configuration to Preferences

    // Set callback for all Setting instances
    template <typename T>
    void setCallback(void (*callback)(const char*, T)) {
        Setting<T>::setCallback(callback);
    }

};

#endif // CONFIG_H
