#ifndef CONFIG_H
#define CONFIG_H

#include <Arduino.h>
#include <ArduinoJson.h>
#include <RemoteDebug.h>
#include <LittleFS.h>

extern RemoteDebug Debug;

class Config {
public:
    // Constructor
    Config();

    // Getter and Setter methods
    String getMqttServer() const;
    void setMqttServer(const String& server);

    String getMqttPort() const;
    void setMqttPort(const String& port);

    String getMqttUserName() const;
    void setMqttUserName(const String& username);

    String getMqttPassword() const;
    void setMqttPassword(const String& password);

    String getSpaName() const;
    void setSpaName(const String& name);

    int getUpdateFrequency() const;
    void setUpdateFrequency(int frequency);

    // File operations
    bool readConfigFile();
    void writeConfigFile();

private:
    // Private member variables
    String mqttServer;
    String mqttPort;
    String mqttUserName;
    String mqttPassword;
    String spaName;
    int updateFrequency;
};

#endif // CONFIG_H
