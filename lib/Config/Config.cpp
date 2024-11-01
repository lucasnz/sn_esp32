#include "Config.h"

// Constructor
Config::Config()
    : mqttServer("mqtt"),
      mqttPort("1883"),
      mqttUserName(""),
      mqttPassword(""),
      spaName("MySpa"),
      updateFrequency(60) {  }

// Getters and Setters

String Config::getMqttServer() const { return mqttServer; }
void Config::setMqttServer(const String& server) { mqttServer = server; }

String Config::getMqttPort() const { return mqttPort; }
void Config::setMqttPort(const String& port) { mqttPort = port; }

String Config::getMqttUserName() const { return mqttUserName; }
void Config::setMqttUserName(const String& username) { mqttUserName = username; }

String Config::getMqttPassword() const { return mqttPassword; }
void Config::setMqttPassword(const String& password) { mqttPassword = password; }

String Config::getSpaName() const { return spaName; }
void Config::setSpaName(const String& name) { spaName = name; }

int Config::getUpdateFrequency() const { return updateFrequency; }
void Config::setUpdateFrequency(int frequency) { updateFrequency = (frequency < 10) ? 10 : frequency; }

// Read config from file
bool Config::readConfigFile() {
  debugI("Reading config file");
  File configFile = LittleFS.open("/config.json","r");
  if (!configFile) {
    return false;
  } else {
    size_t size = configFile.size();
    std::unique_ptr<char[]> buf(new char[size]);
    configFile.readBytes(buf.get(), size);

    JsonDocument json;
    auto deserializeError = deserializeJson(json, buf.get());
    serializeJson(json, Serial);

    if (!deserializeError) {
      debugI("Parsed JSON");

      if (json["mqtt_server"].is<String>()) mqttServer = json["mqtt_server"].as<String>();
      if (json["mqtt_port"].is<String>()) mqttPort = json["mqtt_port"].as<String>();
      if (json["mqtt_username"].is<String>()) mqttUserName = json["mqtt_username"].as<String>();
      if (json["mqtt_password"].is<String>()) mqttPassword = json["mqtt_password"].as<String>();
      if (json["spa_name"].is<String>()) spaName = json["spa_name"].as<String>();
      if (json["update_frequency"].is<int>()) updateFrequency = json["update_frequency"].as<int>();
    } else {
      debugW("Failed to parse config file");
    }
    configFile.close();
  }

  if (updateFrequency < 10) updateFrequency = 10;
  return true;
}

// Write config to file
void Config::writeConfigFile() {
  debugI("Updating config file");
  JsonDocument json;

  json["mqtt_server"] = mqttServer;
  json["mqtt_port"] = mqttPort;
  json["mqtt_username"] = mqttUserName;
  json["mqtt_password"] = mqttPassword;
  json["spa_name"] = spaName;
  json["update_frequency"] = updateFrequency;

  File configFile = LittleFS.open("/config.json", "w");
  if (!configFile) {
    debugE("Failed to open config file for writing");
  } else {
    serializeJson(json, configFile);
    configFile.close();
    debugI("Config file updated");
  }
}
