#ifndef CONFIG_H
#define CONFIG_H


#include <Arduino.h>
#include <ArduinoJson.h>
#include <RemoteDebug.h>
#include <LittleFS.h>

extern String mqttServer;
extern String mqttPort;
extern String mqttUserName;
extern String mqttPassword;
extern String spaName;
extern int updateFrequency;

extern RemoteDebug Debug;

bool readConfigFile();
void writeConfigFile();

#endif // CONFIG_H
