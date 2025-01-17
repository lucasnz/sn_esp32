#include "Config.h"

template<typename T>
void (*Setting<T>::_settingCallback)(const char*, T) = nullptr;
void (*Setting<int>::_settingCallback)(const char*, int) = nullptr;

Preferences preferences;

// Constructor
Config::Config() { }

// Read configuration
bool Config::readConfig() {
  debugI("Reading config from Preferences or file");

  // Check if Preferences are available
  if (preferences.begin("eSpa-config", true)) {
    debugI("Using Preferences for configuration");

    MqttServer.setValue(preferences.getString("MqttServer", ""));
    MqttPort.setValue(preferences.getInt("MqttPort", 1883));
    MqttUsername.setValue(preferences.getString("MqttUsername", ""));
    MqttPassword.setValue(preferences.getString("MqttPassword", ""));
    SpaName.setValue(preferences.getString("SpaName", "eSpa"));
    spaPollFreq.setValue(preferences.getInt("spaPollFreq", 60));
  #ifdef INCLUDE_UPDATES
    fwPollFreq.setValue(preferences.getInt("fwPollFreq", 24));
  #endif // INCLUDE_UPDATES

    preferences.end();
    return true;
  } else {
    debugI("Preferences not found.");
  }
  return false;
}

// Write configuration to Preferences
void Config::writeConfig() {
  debugI("Writing configuration to Preferences");
  if (preferences.begin("eSpa-config", false)) {
    preferences.putString("MqttServer", MqttServer.getValue());
    preferences.putInt("MqttPort", MqttPort.getValue());
    preferences.putString("MqttUsername", MqttUsername.getValue());
    preferences.putString("MqttPassword", MqttPassword.getValue());
    preferences.putString("SpaName", SpaName.getValue());
    preferences.putInt("spaPollFreq", spaPollFreq.getValue());
  #ifdef INCLUDE_UPDATES
    preferences.putInt("fwPollFreq", fwPollFreq.getValue());
  #endif // INCLUDE_UPDATES
    preferences.end();
  } else {
    debugE("Failed to open Preferences for writing");
  }
}
