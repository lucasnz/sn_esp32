#if defined(ESP8266)
  #include <ESP8266WiFi.h>
  #include <ESP8266WebServer.h>

#elif defined(ESP32)
  #include <WiFi.h>
  #include <WebServer.h>

#endif

#include <WiFiClient.h>
#include <RemoteDebug.h>
#include <PubSubClient.h>

#if defined(LED_PIN)
#include "Blinker.h"
#endif
#include "WebUI.h"
#include "Config.h"
#include "SpaInterface.h"
#include "SpaUtils.h"
#include "HAAutoDiscovery.h"

unsigned long bootStartMillis;  // To track when the device started
RemoteDebug Debug;

SpaInterface si;

#if defined(LED_PIN)
Blinker led(LED_PIN);
#endif
WiFiClient wifi;
PubSubClient mqttClient(wifi);

WebUI ui(&si);

ulong mqttLastConnect = 0;
ulong wifiLastConnect = millis();
ulong bootTime = millis();
ulong statusLastPublish = millis();
bool delayedStart = true; // Delay spa connection for 10sec after boot to allow for external debugging if required.
bool autoDiscoveryPublished = false;

String mqttBase = "";
String mqttStatusTopic = "";
String mqttSet = "";
String mqttAvailability = "";

String spaSerialNumber = "";

// We check the EN_PIN every loop, to allow people to configure the system
void checkButton(){
#if defined(EN_PIN)
  if(digitalRead(EN_PIN) == LOW) {
    debugI("Initial buttong press detected");
    delay(100); // wait and then test again to ensure that it is a held button not a press
    if(digitalRead(EN_PIN) == LOW) {
      debugI("Button press detected. Starting Portal");
      ui.startWiFiManager();

      ESP.restart();  // restart, dirty but easier than trying to restart services one by one
    }
  }
#endif
}

/*
void climateADPublish(String name, String propertyId, String deviceName, String deviceIdentifier ) {
*/
/*
{ 
   "device_class":"temperature",
   "state_topic":"homeassistant/sensor/sensorBedroom/state",
   "unit_of_measurement":"°C",
   "value_template":"{{ value_json.temperature}}",
   "unique_id":"temp01ae",
   "device":{
      "identifiers":[
         "bedroom01ae"
      ],
      "name":"Bedroom"
   }
}"
*/
/*
  JsonDocument json;
  String uniqueID = spaSerialNumber + "-" + propertyId;

  if (name != "") { json["name"]=name; }

  json["current_temperature_topic"]=mqttStatusTopic;;
  json["current_temperature_template"]="{{ value_json.temperatures.water }}";

  JsonObject device = json["device"].to<JsonObject>();
  device["name"] = deviceName;
  JsonArray identifiers = device["identifiers"].to<JsonArray>();
  identifiers.add(deviceIdentifier);

  json["initial"]=36;
  json["max_temp"]=41;
  json["min_temp"]=10;
  JsonArray modes = json["modes"].to<JsonArray>();
  modes.add("auto"); // the actual modes of the heat pump are controlled through a select control to avoid accidently turning off the HP and using the resistance heater
  json["mode_state_template"]="auto";
  json["mode_state_topic"]=mqttStatusTopic;

  json["action_topic"]=mqttStatusTopic;
  json["action_template"]="{% if value_json.status.heatingActive == 'ON' %}heating{% else %}off{% endif %}";
  
  json["temperature_command_topic"]=mqttSet+"/status_temperatureSetPoint";
  json["temperature_state_template"]="{{ value_json.status.temperatureSetPoint }}";
  json["temperature_state_topic"]=mqttStatusTopic;
  json["temperature_unit"]="C";
  json["temp_step"]=0.2;

  json["unique_id"] = uniqueID;

  JsonObject availability = json["availability"].to<JsonObject>();
  availability["topic"] =mqttAvailability;

  // <discovery_prefix>/<component>/[<node_id>/]<object_id>/config
  String discoveryTopic = "homeassistant/climate/"+ spaSerialNumber + "/" + uniqueID + "/config";
  String output = "";
  serializeJson(json,output);
  mqttClient.publish(discoveryTopic.c_str(),output.c_str(),true);

}

void fanADPublish(String name, String propertyId, String deviceName, String deviceIdentifier ) {
*/
/*
# Example using percentage based speeds with preset modes configuration.yaml
mqtt:
  - fan:
      name: "Bedroom Fan"
      state_topic: "bedroom_fan/on/state"
      command_topic: "bedroom_fan/on/set"
      direction_state_topic: "bedroom_fan/direction/state"
      direction_command_topic: "bedroom_fan/direction/set"
      oscillation_state_topic: "bedroom_fan/oscillation/state"
      oscillation_command_topic: "bedroom_fan/oscillation/set"
      percentage_state_topic: "bedroom_fan/speed/percentage_state"
      percentage_command_topic: "bedroom_fan/speed/percentage"
      preset_mode_state_topic: "bedroom_fan/preset/preset_mode_state"
      preset_mode_command_topic: "bedroom_fan/preset/preset_mode"
      preset_modes:
        -  "auto"
        -  "smart"
        -  "whoosh"
        -  "eco"
        -  "breeze"
      qos: 0
      payload_on: "true"
      payload_off: "false"
      payload_oscillation_on: "true"
      payload_oscillation_off: "false"
      speed_range_min: 1
      speed_range_max: 10
*/
/*
  JsonDocument json;
  String uniqueID = spaSerialNumber + "-" + propertyId;

  if (name != "") { json["name"]=name; }
  JsonObject device = json["device"].to<JsonObject>();
  device["name"] = deviceName;
  JsonArray identifiers = device["identifiers"].to<JsonArray>();
  identifiers.add(deviceIdentifier);


  json["state_topic"] = mqttStatusTopic;
  json["state_value_template"] = "{{ value_json."+propertyId+".state }}";

  json["command_topic"] = mqttSet + "/" + propertyId + "_state";
  
  json["percentage_state_topic"] = mqttStatusTopic;
  json["percentage_command_topic"] = mqttSet + "/" + propertyId + "_speed";
  json["percentage_value_template"] = "{{ value_json."+ propertyId + ".speed }}";
  
  json["preset_mode_state_topic"] = mqttStatusTopic;
  json["preset_mode_command_topic"] = mqttSet + "/" + propertyId + "_mode";
  json["preset_mode_value_template"] = "{{ value_json."+ propertyId + ".mode }}";
  
  JsonArray modes = json["preset_modes"].to<JsonArray>();
  modes.add("Variable");
  modes.add("Ramp");
  json["speed_range_min"]=1;
  json["speed_range_max"]=5;

  json["unique_id"] = uniqueID;

  JsonObject availability = json["availability"].to<JsonObject>();
  availability["topic"] =mqttAvailability;

  String discoveryTopic = "homeassistant/fan/"+ spaSerialNumber + "/" + uniqueID + "/config";
  String output = "";
  serializeJson(json,output);
  mqttClient.publish(discoveryTopic.c_str(),output.c_str(),true);
}


void switchADPublish (String name, String deviceClass, String stateTopic, String valueTemplate, String propertyId, String deviceName, String deviceIdentifier) {
*/
/*
{
   "name":"Irrigation",
   "command_topic":"homeassistant/switch/irrigation/set",
   "state_topic":"homeassistant/switch/irrigation/state",
   "unique_id":"irr01ad",
   "device":{
      "identifiers":[
         "garden01ad"
      ],
      "name":"Garden"
   }
}*/
/*
  JsonDocument json;

  if (deviceClass != "") { json["device_class"] = deviceClass; }
  json["name"]=name;
  json["state_topic"] = stateTopic;
  json["value_template"] = valueTemplate;
  json["command_topic"] = mqttSet + "/" + propertyId;
  json["unique_id"] = spaSerialNumber + "-" + propertyId;
  JsonObject device = json["device"].to<JsonObject>();
  device["name"] = deviceName;
  JsonArray identifiers = device["identifiers"].to<JsonArray>();
  identifiers.add(deviceIdentifier);

  JsonObject availability = json["availability"].to<JsonObject>();
  availability["topic"] =mqttAvailability;

  // <discovery_prefix>/<component>/[<node_id>/]<object_id>/config
  String discoveryTopic = "homeassistant/switch/" + spaSerialNumber + "/" + spaSerialNumber + "-" + propertyId + "/config";
  String output = "";
  serializeJson(json,output);
  mqttClient.publish(discoveryTopic.c_str(),output.c_str(),true);

}


void selectADPublish (String name, std::vector<String> options, String stateTopic, String valueTemplate, String propertyId, String deviceName, String deviceIdentifier, String category="") {
*/
/*
{
   "name":"Irrigation",
   "command_topic":"homeassistant/switch/irrigation/set",
   "state_topic":"homeassistant/switch/irrigation/state",
   "unique_id":"irr01ad",
   "device":{
      "identifiers":[
         "garden01ad"
      ],
      "name":"Garden"
   }
}*/
/*
  JsonDocument json;

  if (category != "") json["entity_category"] = category;
  json["name"]=name;
  json["state_topic"] = stateTopic;
  json["value_template"] = valueTemplate;
  json["command_topic"] = mqttSet + "/" + propertyId;
  json["unique_id"] = spaSerialNumber + "-" + propertyId;
  JsonObject device = json["device"].to<JsonObject>();
  device["name"] = deviceName;
  JsonArray identifiers = device["identifiers"].to<JsonArray>();
  identifiers.add(deviceIdentifier);

  JsonObject availability = json["availability"].to<JsonObject>();
  availability["topic"] =mqttAvailability;

  JsonArray opts = json["options"].to<JsonArray>();
  for (auto o : options) opts.add(o);
  
  // <discovery_prefix>/<component>/[<node_id>/]<object_id>/config
  String discoveryTopic = "homeassistant/select/" + spaSerialNumber + "/" + spaSerialNumber + "-" + propertyId + "/config";
  String output = "";
  serializeJson(json,output);
  debugV("json: %s", output.c_str());
  mqttClient.publish(discoveryTopic.c_str(),output.c_str(),true);

}


void textADPublish (String name, String stateTopic, String valueTemplate, String propertyId, String deviceName, String deviceIdentifier, String category="", String regex="") {
*/
/*{
  "availability": {
      "topic": "sn_esp32/21110001-20000337/available"
  },
  "command_topic": "sn_esp32/21110001-20000337/set/datetime",
  "device": {
      "identifiers": [
          "21110001-20000337"
      ],
      "name": "MySpa"
  },
	"entity_category": "config",
	"pattern": "[0-9]{4}-[0-9]{2}-[0-9]{2} [0-9]{2}:[0-9]{2}",
  "name": "Date Time",
  "state_topic": "sn_esp32/21110001-20000337/status",
  "unique_id": "21110001-20000337-datetime",
  "value_template": "{{ value_json.datetime }}"
}*/
/*
  JsonDocument json;

  JsonObject availability = json["availability"].to<JsonObject>();
  availability["topic"] =mqttAvailability;

  json["command_topic"] = mqttSet + "/" + propertyId;

  JsonObject device = json["device"].to<JsonObject>();
  device["name"] = deviceName;
  JsonArray identifiers = device["identifiers"].to<JsonArray>();
  identifiers.add(deviceIdentifier);

  if (category != "") json["entity_category"] = category;
  
  if (regex != "") json["pattern"] = regex;

  json["name"]=name;

  json["state_topic"] = stateTopic;

  json["unique_id"] = spaSerialNumber + "-" + propertyId;

  json["value_template"] = valueTemplate;

  // <discovery_prefix>/<component>/[<node_id>/]<object_id>/config
  String discoveryTopic = "homeassistant/text/" + spaSerialNumber + "/" + spaSerialNumber + "-" + propertyId + "/config";
  String output = "";
  serializeJson(json,output);
  mqttClient.publish(discoveryTopic.c_str(),output.c_str(),true);

}


void lightADPublish (String name, String deviceClass, String stateTopic, String valueTemplate, String propertyId, String deviceName, String deviceIdentifier) {
*/
/*
{
   "name":"Irrigation",
   "command_topic":"homeassistant/switch/irrigation/set",
   "state_topic":"homeassistant/switch/irrigation/state",
   "unique_id":"irr01ad",
   "device":{
      "identifiers":[
         "garden01ad"
      ],
      "name":"Garden"
   }
}*/
/*
  JsonDocument json;

  if (deviceClass != "") { json["device_class"] = deviceClass; }
  json["name"]=name;
  //json["schema"] = "json";
  json["state_topic"] = stateTopic;
  json["brightness_state_topic"] = stateTopic;
  json["color_mode_state_topic"] = stateTopic;
  json["effect_state_topic"] = stateTopic;
  json["hs_state_topic"] = stateTopic;

  json["command_topic"] = mqttSet + "/" + propertyId + "_state";
  json["brightness_command_topic"] = mqttSet + "/" + propertyId + "_brightness";
  json["effect_command_topic"] = mqttSet + "/" + propertyId + "_effect";
  json["hs_command_topic"] = mqttSet + "/" + propertyId + "_color";

  // Find the last character that is not a space or curly brace
  int lastIndex = valueTemplate.length() - 1;
  while (lastIndex >= 0 && (valueTemplate[lastIndex] == ' ' || valueTemplate[lastIndex] == '}')) {
    lastIndex--;
  }

  // Value templates to extract values from the same topic
  json["state_value_template"] = valueTemplate.substring(0, lastIndex + 1) + ".state" + valueTemplate.substring(lastIndex + 1);
  json["brightness_value_template"] = valueTemplate.substring(0, lastIndex + 1) + ".brightness" + valueTemplate.substring(lastIndex + 1);
  json["effect_value_template"] = valueTemplate.substring(0, lastIndex + 1) + ".effect" + valueTemplate.substring(lastIndex + 1);
  json["hs_value_template"] = valueTemplate.substring(0, lastIndex + 1) + ".color.h" + valueTemplate.substring(lastIndex + 1) + ","
                            + valueTemplate.substring(0, lastIndex + 1) + ".color.s" + valueTemplate.substring(lastIndex + 1);
  json["color_mode_value_template"] = valueTemplate.substring(0, lastIndex + 1) + ".color_mode" + valueTemplate.substring(lastIndex + 1);

  json["unique_id"] = spaSerialNumber + "-" + propertyId;
  JsonObject device = json["device"].to<JsonObject>();
  device["name"] = deviceName;
  JsonArray identifiers = device["identifiers"].to<JsonArray>();
  identifiers.add(deviceIdentifier);

  JsonObject availability = json["availability"].to<JsonObject>();
  availability["topic"] =mqttAvailability;

  json["brightness"] = true;
  json["brightness_scale"]=5;
  json["effect"] = true;
  JsonArray effect_list = json["effect_list"].to<JsonArray>();
  for (auto effect: si.colorModeStrings) effect_list.add(effect);
  JsonArray color_modes = json["supported_color_modes"].to<JsonArray>();
  color_modes.add("hs");

  // <discovery_prefix>/<component>/[<node_id>/]<object_id>/config
  String discoveryTopic = "homeassistant/light/" + spaSerialNumber + "/" + spaSerialNumber + "-" + propertyId + "/config";
  String output = "";
  serializeJson(json,output);
  mqttClient.publish(discoveryTopic.c_str(),output.c_str(),true);

}
*/

void mqttHaAutoDiscovery() {
  debugI("Publishing Home Assistant auto discovery");

  String output;
  String discoveryTopic;

  SpaADInformationTemplate spa;
  spa.spaName = spaName;
  spa.spaSerialNumber = spaSerialNumber;
  spa.stateTopic = mqttStatusTopic;
  spa.availabilityTopic = mqttAvailability;

  //sensorADPublish("Water Temperature","","temperature",mqttStatusTopic,"°C","{{ value_json.temperatures.water }}","measurement","WaterTemperature", spaName, spaSerialNumber);
  //sensorADPublish("Heater Temperature","diagnostic","temperature",mqttStatusTopic,"°C","{{ value_json.temperatures.heater }}","measurement","HeaterTemperature", spaName, spaSerialNumber);
  //sensorADPublish("Case Temperature","diagnostic","temperature",mqttStatusTopic,"°C","{{ value_json.temperatures.case }}","measurement","CaseTemperature", spaName, spaSerialNumber);
  //sensorADPublish("Mains Voltage","diagnostic","voltage",mqttStatusTopic,"V","{{ value_json.power.voltage }}","measurement","MainsVoltage", spaName, spaSerialNumber);
  //sensorADPublish("Mains Current","diagnostic","current",mqttStatusTopic,"A","{{ value_json.power.current }}","measurement","MainsCurrent", spaName, spaSerialNumber);
  //sensorADPublish("Power","","energy",mqttStatusTopic,"W","{{ value_json.power.energy }}","measurement","Power", spaName, spaSerialNumber);
  //sensorADPublish("Total Energy","","energy",mqttStatusTopic,"Wh","{{ value_json.totalenergy }}","total_increasing","TotalEnergy", spaName, spaSerialNumber);
  //sensorADPublish("Heatpump Ambient Temperature","","temperature",mqttStatusTopic,"°C","{{ value_json.temperatures.heatpumpAmbient }}","measurement","HPAmbTemp", spaName, spaSerialNumber);
  //sensorADPublish("Heatpump Condensor Temperature","","temperature",mqttStatusTopic,"°C","{{ value_json.temperatures.heatpumpCondensor }}","measurement","HPCondTemp", spaName, spaSerialNumber);
  //sensorADPublish("State","","",mqttStatusTopic,"","{{ value_json.status.state }}","","State", spaName, spaSerialNumber);
  spa.commandTopic = mqttSet;
  
  AutoDiscoveryInformationTemplate ADConf;

  ADConf.displayName = "Water Temperature";
  ADConf.valueTemplate = "{{ value_json.temperatures.water }}";
  ADConf.propertyId = "WaterTemperature";
  ADConf.deviceClass = "temperature";
  ADConf.entityCategory = "";
  generateSensorAdJSON(output, ADConf, spa, discoveryTopic, "measurement", "°C");
  mqttClient.publish(discoveryTopic.c_str(), output.c_str(), true);

  ADConf.displayName = "Case Temperature";
  ADConf.valueTemplate = "{{ value_json.temperatures.case }}";
  ADConf.propertyId = "CaseTemperature";
  ADConf.deviceClass = "temperature";
  ADConf.entityCategory = "diagnostic";
  generateSensorAdJSON(output, ADConf, spa, discoveryTopic, "measurement", "°C");
  mqttClient.publish(discoveryTopic.c_str(), output.c_str(), true);

  ADConf.displayName = "Heater Temperature";
  ADConf.valueTemplate = "{{ value_json.temperatures.heater }}";
  ADConf.propertyId = "HeaterTemperature";
  ADConf.deviceClass = "temperature";
  ADConf.entityCategory = "diagnostic";
  generateSensorAdJSON(output, ADConf, spa, discoveryTopic, "measurement", "°C");
  mqttClient.publish(discoveryTopic.c_str(), output.c_str(), true);

  ADConf.displayName = "Heatpump Ambient Temperature";
  ADConf.valueTemplate = "{{ value_json.temperatures.heatpumpAmbient }}";
  ADConf.propertyId = "HPAmbTemp";
  ADConf.deviceClass = "temperature";
  ADConf.entityCategory = "diagnostic";
  generateSensorAdJSON(output, ADConf, spa, discoveryTopic, "measurement", "°C");
  mqttClient.publish(discoveryTopic.c_str(), output.c_str(), true);

  ADConf.displayName = "Heatpump Condensor Temperature";
  ADConf.valueTemplate = "{{ value_json.temperatures.heatpumpCondensor }}";
  ADConf.propertyId = "HPCondTemp";
  ADConf.deviceClass = "temperature";
  ADConf.entityCategory = "diagnostic";
  generateSensorAdJSON(output, ADConf, spa, discoveryTopic, "measurement", "°C");
  mqttClient.publish(discoveryTopic.c_str(), output.c_str(), true);

  ADConf.displayName = "Mains Voltage";
  ADConf.valueTemplate = "{{ value_json.power.voltage }}";
  ADConf.propertyId = "MainsVoltage";
  ADConf.deviceClass = "voltage";
  ADConf.entityCategory = "diagnostic";
  generateSensorAdJSON(output, ADConf, spa, discoveryTopic, "measurement", "V");
  mqttClient.publish(discoveryTopic.c_str(), output.c_str(), true);

  ADConf.displayName = "Mains Current";
  ADConf.valueTemplate = "{{ value_json.power.current }}";
  ADConf.propertyId = "MainsCurrent";
  ADConf.deviceClass = "current";
  ADConf.entityCategory = "diagnostic";
  generateSensorAdJSON(output, ADConf, spa, discoveryTopic, "measurement", "A");
  mqttClient.publish(discoveryTopic.c_str(), output.c_str(), true);

  ADConf.displayName = "Power";
  ADConf.valueTemplate = "{{ value_json.power.power }}";
  ADConf.propertyId = "Power";
  ADConf.deviceClass = "power";
  ADConf.entityCategory = "diagnostic";
  generateSensorAdJSON(output, ADConf, spa, discoveryTopic, "measurement", "W");
  mqttClient.publish(discoveryTopic.c_str(), output.c_str(), true);

  ADConf.displayName = "Total Energy";
  ADConf.valueTemplate = "{{ value_json.power.totalEnergy }}";
  ADConf.propertyId = "TotalEnergy";
  ADConf.deviceClass = "energy";
  ADConf.entityCategory = "diagnostic";
  generateSensorAdJSON(output, ADConf, spa, discoveryTopic, "measurement", "kWh");
  mqttClient.publish(discoveryTopic.c_str(), output.c_str(), true);

  ADConf.displayName = "State";
  ADConf.valueTemplate = "{{ value_json.status.state }}";
  ADConf.propertyId = "State";
  ADConf.deviceClass = "";
  ADConf.entityCategory = "";
  generateSensorAdJSON(output, ADConf, spa, discoveryTopic);
  mqttClient.publish(discoveryTopic.c_str(), output.c_str(), true);

  //binarySensorADPublish("Heating Active","",mqttStatusTopic,"{{ value_json.status.heatingActive }}","HeatingActive", spaName, spaSerialNumber);
  //binarySensorADPublish("Ozone Active","",mqttStatusTopic,"{{ value_json.status.ozoneActive }}","OzoneActive", spaName, spaSerialNumber);
  ADConf.displayName = "Heating Active";
  ADConf.valueTemplate = "{{ value_json.status.heatingActive }}";
  ADConf.propertyId = "HeatingActive";
  ADConf.deviceClass = "heat";
  ADConf.entityCategory = "";
  generateBinarySensorAdJSON(output, ADConf, spa, discoveryTopic);
  mqttClient.publish(discoveryTopic.c_str(), output.c_str(), true);

  ADConf.displayName = "Ozone Active";
  ADConf.valueTemplate = "{{ value_json.status.ozoneActive }}";
  ADConf.propertyId = "OzoneActive";
  ADConf.deviceClass = "running";
  ADConf.entityCategory = "";
  generateBinarySensorAdJSON(output, ADConf, spa, discoveryTopic);
  mqttClient.publish(discoveryTopic.c_str(), output.c_str(), true);

  //climateADPublish(mqttClient, spa, spaName, "{{ value_json.temperatures }}", "Heating");
  ADConf.displayName = spaName;
  ADConf.valueTemplate = "{{ value_json.temperatures }}";
  ADConf.propertyId = "Heating";
  ADConf.deviceClass = "";
  ADConf.entityCategory = "";
  generateClimateAdJSON(output, ADConf, spa, discoveryTopic);
  mqttClient.publish(discoveryTopic.c_str(), output.c_str(), true);

  //selectADPublish(mqttClient, spa, "Heatpump Mode", "{{ value_json.heatpump.mode }}", "heatpump_mode", "", "", {"Auto","Heat","Cool","Off"});
  ADConf.displayName = "Heatpump Mode";
  ADConf.valueTemplate = "{{ value_json.heatpump.mode }}";
  ADConf.propertyId = "heatpump_mode";
  ADConf.deviceClass = "";
  ADConf.entityCategory = "";
  generateSelectAdJSON(output, ADConf, spa, discoveryTopic, si.HPMPStrings);
  mqttClient.publish(discoveryTopic.c_str(), output.c_str(), true);


  ADConf.deviceClass = "";
  ADConf.entityCategory = "";
  if (si.getPump1InstallState().startsWith("1") && !(si.getPump1InstallState().endsWith("4"))) {
     //switchADPublish(mqttClient, spa, "Pump 1", "{{ value_json.pumps.pump1.state }}", "pump1");
    ADConf.displayName = "Pump 1";
    ADConf.valueTemplate = "{{ value_json.pumps.pump1.state }}";
    ADConf.propertyId = "pump1";
    generateSwitchAdJSON(output, ADConf, spa, discoveryTopic);
    mqttClient.publish(discoveryTopic.c_str(), output.c_str(), true);
  }

  if (si.getPump2InstallState().startsWith("1") && !(si.getPump2InstallState().endsWith("4"))) {
    //switchADPublish(mqttClient, spa, "Pump 2","{{ value_json.pumps.pump2.state }}", "pump2");
    ADConf.displayName = "Pump 2";
    ADConf.valueTemplate = "{{ value_json.pumps.pump2.state }}";
    ADConf.propertyId = "pump2";
    generateSwitchAdJSON(output, ADConf, spa, discoveryTopic);
    mqttClient.publish(discoveryTopic.c_str(), output.c_str(), true);
  }

  if (si.getPump3InstallState().startsWith("1") && !(si.getPump3InstallState().endsWith("4"))) {
    //switchADPublish(mqttClient, spa, "Pump 3", "{{ value_json.pumps.pump3.state }}", "pump3");
    ADConf.displayName = "Pump 3";
    ADConf.valueTemplate = "{{ value_json.pumps.pump3.state }}";
    ADConf.propertyId = "pump3";
    generateSwitchAdJSON(output, ADConf, spa, discoveryTopic);
    mqttClient.publish(discoveryTopic.c_str(), output.c_str(), true);
  }

  if (si.getPump4InstallState().startsWith("1") && !(si.getPump4InstallState().endsWith("4"))) {
    //switchADPublish(mqttClient, spa, "Pump 4", "{{ value_json.pumps.pump4.state }}", "pump4");
    ADConf.displayName = "Pump 4";
    ADConf.valueTemplate = "{{ value_json.pumps.pump4.state }}";
    ADConf.propertyId = "pump4";
    generateSwitchAdJSON(output, ADConf, spa, discoveryTopic);
    mqttClient.publish(discoveryTopic.c_str(), output.c_str(), true);
  }

  if (si.getPump5InstallState().startsWith("1") && !(si.getPump5InstallState().endsWith("4"))) {
    //switchADPublish(mqttClient, spa, "Pump 5", "{{ value_json.pumps.pump5.state }}", "pump5");
    ADConf.displayName = "Pump 5";
    ADConf.valueTemplate = "{{ value_json.pumps.pump5.state }}";
    ADConf.propertyId = "pump5";
    generateSwitchAdJSON(output, ADConf, spa, discoveryTopic);
    mqttClient.publish(discoveryTopic.c_str(), output.c_str(), true);

  }

  //switchADPublish(mqttClient, spa, "Aux Heat Element", "{{ value_json.heatpump.auxheat }}", "heatpump_auxheat");
  ADConf.displayName = "Aux Heat Element";
  ADConf.valueTemplate = "{{ value_json.heatpump.auxheat }}";
  ADConf.propertyId = "heatpump_auxheat";
  ADConf.deviceClass = "";
  ADConf.entityCategory = "";
  generateSwitchAdJSON(output, ADConf, spa, discoveryTopic);
  mqttClient.publish(discoveryTopic.c_str(), output.c_str(), true);

  //lightADPublish(mqttClient, spa, "Lights", "{{ value_json.lights }}", "lights", "", "", colorModeStrings);
  ADConf.displayName = "Lights";
  ADConf.valueTemplate = "{{ value_json.lights }}";
  ADConf.propertyId = "lights";
  ADConf.deviceClass = "";
  ADConf.entityCategory = "";
  generateLightAdJSON(output, ADConf, spa, discoveryTopic, si.colorModeStrings);
  mqttClient.publish(discoveryTopic.c_str(), output.c_str(), true);

  //selectADPublish(mqttClient, spa, "Lights Speed","{{ value_json.lights.speed }}","lights_speed", "", "", {"1","2","3","4","5"});
  ADConf.displayName = "Lights Speed";
  ADConf.valueTemplate = "{{ value_json.lights.speed }}";
  ADConf.propertyId = "lights_speed";
  ADConf.deviceClass = "";
  ADConf.entityCategory = "";
  generateSelectAdJSON(output, ADConf, spa, discoveryTopic, si.lightSpeedMap );
  mqttClient.publish(discoveryTopic.c_str(), output.c_str(), true);

  //selectADPublish(mqttClient, spa, "Sleep Timer 1","{{ value_json.sleepTimers.timer1.state }}", "sleepTimers_1_state", "config", "", sleepStrings);
  //selectADPublish(mqttClient, spa, "Sleep Timer 2","{{ value_json.sleepTimers.timer2.state }}", "sleepTimers_2_state", "config", "", sleepStrings);
  ADConf.displayName = "Sleep Timer 1";
  ADConf.valueTemplate = "{{ value_json.sleepTimers.timer1.state }}";
  ADConf.propertyId = "sleepTimers_1_state";
  ADConf.deviceClass = "";
  ADConf.entityCategory = "config";
  generateSelectAdJSON(output, ADConf, spa, discoveryTopic, si.sleepSelection);
  mqttClient.publish(discoveryTopic.c_str(), output.c_str(), true);

  ADConf.displayName = "Sleep Timer 2";
  ADConf.valueTemplate = "{{ value_json.sleepTimers.timer2.state }}";
  ADConf.propertyId = "sleepTimers_2_state";
  ADConf.deviceClass = "";
  ADConf.entityCategory = "config";
  generateSelectAdJSON(output, ADConf, spa, discoveryTopic, si.sleepSelection);
  mqttClient.publish(discoveryTopic.c_str(), output.c_str(), true);

  ADConf.displayName = "Date Time";
  ADConf.valueTemplate = "{{ value_json.status.datetime }}";
  ADConf.propertyId = "status_datetime";
  ADConf.deviceClass = "";
  ADConf.entityCategory = "config";
  generateTextAdJSON(output, ADConf, spa, discoveryTopic, "[0-9]{4}-[0-9]{2}-[0-9]{2} [0-9]{2}:[0-9]{2}");
  mqttClient.publish(discoveryTopic.c_str(), output.c_str(), true);

  ADConf.displayName = "Sleep Timer 1 Begin";
  ADConf.valueTemplate = "{{ value_json.sleepTimers.timer1.begin }}";
  ADConf.propertyId = "sleepTimers_1_begin";
  ADConf.deviceClass = "";
  ADConf.entityCategory = "config";
  generateTextAdJSON(output, ADConf, spa, discoveryTopic, "[0-2][0-9]:[0-9]{2}");
  mqttClient.publish(discoveryTopic.c_str(), output.c_str(), true);

  ADConf.displayName = "Sleep Timer 1 End";
  ADConf.valueTemplate = "{{ value_json.sleepTimers.timer1.end }}";
  ADConf.propertyId = "sleepTimers_1_end";
  ADConf.deviceClass = "";
  ADConf.entityCategory = "config";
  generateTextAdJSON(output, ADConf, spa, discoveryTopic, "[0-2][0-9]:[0-9]{2}");
  mqttClient.publish(discoveryTopic.c_str(), output.c_str(), true);

  ADConf.displayName = "Sleep Timer 2 Begin";
  ADConf.valueTemplate = "{{ value_json.sleepTimers.timer2.begin }}";
  ADConf.propertyId = "sleepTimers_2_begin";
  ADConf.deviceClass = "";
  ADConf.entityCategory = "config";
  generateTextAdJSON(output, ADConf, spa, discoveryTopic, "[0-2][0-9]:[0-9]{2}");
  mqttClient.publish(discoveryTopic.c_str(), output.c_str(), true);

  ADConf.displayName = "Sleep Timer 2 End";
  ADConf.valueTemplate = "{{ value_json.sleepTimers.timer2.end }}";
  ADConf.propertyId = "sleepTimers_2_end";
  ADConf.deviceClass = "";
  ADConf.entityCategory = "config";
  generateTextAdJSON(output, ADConf, spa, discoveryTopic, "[0-2][0-9]:[0-9]{2}");
  mqttClient.publish(discoveryTopic.c_str(), output.c_str(), true);

  //fanADPublish(mqttClient, spa, "Blower", "{{ value_json.blower }}", "blower");
  ADConf.displayName = "Blower";
  ADConf.valueTemplate = "{{ value_json.blower }}";
  ADConf.propertyId = "blower";
  ADConf.deviceClass = "";
  ADConf.entityCategory = "";
  generateFanAdJSON(output, ADConf, spa, discoveryTopic);
  mqttClient.publish(discoveryTopic.c_str(), output.c_str(), true);

  //selectADPublish(mqttClient, spa, "Spa Mode", "{{ value_json.status.spaMode }}", "status_spaMode", "", "", spaModeStrings);
  ADConf.displayName = "Spa Mode";
  ADConf.valueTemplate = "{{ value_json.status.spaMode }}";
  ADConf.propertyId = "status_spaMode";
  ADConf.deviceClass = "";
  ADConf.entityCategory = "";
  generateSelectAdJSON(output, ADConf, spa, discoveryTopic, si.spaModeStrings);
  mqttClient.publish(discoveryTopic.c_str(), output.c_str(), true);

}

#pragma region MQTT Publish / Subscribe

void mqttPublishStatusString(String s){

  mqttClient.publish(String(mqttBase+"rfResponse").c_str(),s.c_str());

}

void mqttPublishStatus() {
  String json;
  if (generateStatusJson(si, json)) {
    mqttClient.publish(mqttStatusTopic.c_str(),json.c_str());
  } else {
    debugD("Error generating json");
  }
}


void mqttCallback(char* topic, byte* payload, unsigned int length) {
  String t = String(topic);

  String p = "";
  for (uint x = 0; x < length; x++) {
    p += char(*payload);
    payload++;
  }

  debugD("MQTT subscribe received '%s' with payload '%s'",topic,p.c_str());

  String property = t.substring(t.lastIndexOf("/")+1);

  debugI("Received update for %s to %s",property.c_str(),p.c_str());

  if (property == "temperatures_setPoint") {
    si.setSTMP(int(p.toFloat()*10));
  } else if (property == "heatpump_mode") {
    si.setHPMP(p);
  } else if (property == "pump1") {
    si.setRB_TP_Pump1(p=="OFF"?0:1);
  } else if (property == "pump2") {
    si.setRB_TP_Pump2(p=="OFF"?0:1);
  } else if (property == "pump3") {
    si.setRB_TP_Pump3(p=="OFF"?0:1);
  } else if (property == "pump4") {
    si.setRB_TP_Pump4(p=="OFF"?0:1);
  } else if (property == "pump5") {
    si.setRB_TP_Pump5(p=="OFF"?0:1);
  } else if (property == "heatpump_auxheat") {
    si.setHELE(p=="OFF"?0:1);
  } else if (property == "status_datetime") {
    tmElements_t tm;
    tm.Year=CalendarYrToTm(p.substring(0,4).toInt());
    tm.Month=p.substring(5,7).toInt();
    tm.Day=p.substring(8,10).toInt();
    tm.Hour=p.substring(11,13).toInt();
    tm.Minute=p.substring(14,16).toInt();
    tm.Second=p.substring(17).toInt();
    si.setSpaTime(makeTime(tm));
  } else if (property == "lights_state") {
    si.setRB_TP_Light(p=="ON"?1:0);
  } else if (property == "lights_effect") {
    si.setColorMode(p);
  } else if (property == "lights_brightness") {
    si.setLBRTValue(p.toInt());
  } else if (property == "lights_color") {
    int pos = p.indexOf(',');
    if ( pos > 0) {
      int value = p.substring(0, pos).toInt();
      si.setCurrClr(si.colorMap[value/15]);
    }
  } else if (property == "lights_speed") {
    si.setLSPDValue(p);
  } else if (property == "blower_state") {
    si.setOutlet_Blower(p=="OFF"?2:0);
  } else if (property == "blower_speed") {
    if (p=="0") si.setOutlet_Blower(2);
    else si.setVARIValue(p.toInt());
  } else if (property == "blower_mode") {
    si.setOutlet_Blower(p=="Variable"?0:1);
  } else if (property == "sleepTimers_1_state" || property == "sleepTimers_2_state") {
    int member=0;
    for (const auto& i : si.sleepSelection) {
      if (i == p) {
        if (property == "sleepTimers_1_state")
          si.setL_1SNZ_DAY(si.sleepBitmap[member]);
        else if (property == "sleepTimers_2_state")
          si.setL_2SNZ_DAY(si.sleepBitmap[member]);
        break;
      }
      member++;
    }
  } else if (property == "sleepTimers_1_begin") {
    si.setL_1SNZ_BGN(convertToInteger(p));
  } else if (property == "sleepTimers_1_end") {
    si.setL_1SNZ_END(convertToInteger(p));
  } else if (property == "sleepTimers_2_begin") {
    si.setL_2SNZ_BGN(convertToInteger(p));
  } else if (property == "sleepTimers_2_end") {
    si.setL_2SNZ_END(convertToInteger(p));
  } else if (property == "status_spaMode") {
    si.setMode(p);
  } else {
    debugE("Unhandled property - %s",property.c_str());
  }
}


#pragma endregion

void setup() {
  #if defined(EN_PIN)
    pinMode(EN_PIN, INPUT_PULLUP);
  #endif

  #if !defined(ESP8266) or defined(ENABLE_SERIAL)
    Serial.begin(115200);
    Serial.setDebugOutput(true);
    Debug.setSerialEnabled(true);
  #endif

  delay(200);
  debugA("Starting... %s", WiFi.getHostname());

  WiFi.mode(WIFI_STA); 
  WiFi.begin();

  Debug.begin(WiFi.getHostname());
  Debug.setResetCmdEnabled(true);
  Debug.showProfiler(true);

  debugI("Mounting FS");

  if (!LittleFS.begin()) {
    debugW("Failed to mount file system, formatting");
    LittleFS.format();
    LittleFS.begin();
  }

  if (!readConfigFile()) {
    debugW("Failed to open config.json, starting Wi-Fi Manager");
    ui.startWiFiManager();
    //I'm not sure if we need a reboot here - probably not
  }

  mqttClient.setServer(mqttServer.c_str(),mqttPort.toInt());
  mqttClient.setCallback(mqttCallback);
  mqttClient.setBufferSize(2048);

  bootStartMillis = millis();  // Record the current boot time in milliseconds

  ui.begin();

}



void loop() {  



  checkButton();
  #if defined(LED_PIN)
  led.tick();
  #endif
  mqttClient.loop();
  Debug.handle();

  if (ui.initialised) { 
    ui.server->handleClient(); 
  }

  if (WiFi.status() != WL_CONNECTED) {
    //wifi not connected
    #if defined(LED_PIN)
    led.setInterval(100);
    #endif

    if (millis()-wifiLastConnect > 10000) {
      debugI("Wifi reconnecting...");
      wifiLastConnect = millis();
      WiFi.reconnect();
    }
  } else {
    if (delayedStart) {
      delayedStart = !(bootTime + 10000 < millis());
    } else {

      si.loop();

      if (si.isInitialised()) {
        if ( spaSerialNumber=="" ) {
          debugI("Initialising...");
      
          spaSerialNumber = si.getSerialNo1()+"-"+si.getSerialNo2();
          debugI("Spa serial number is %s",spaSerialNumber.c_str());

          mqttBase = String("sn_esp32/") + spaSerialNumber + String("/");
          mqttStatusTopic = mqttBase + "status";
          mqttSet = mqttBase + "set";
          mqttAvailability = mqttBase+"available";
          debugI("MQTT base topic is %s",mqttBase.c_str());
        }
        if (!mqttClient.connected()) {  // MQTT broker reconnect if not connected
          long now=millis();
          if (now - mqttLastConnect > 1000) {
            #if defined(LED_PIN)
            led.setInterval(500);
            #endif
            debugW("MQTT not connected, attempting connection to %s:%s",mqttServer.c_str(),mqttPort.c_str());
            mqttLastConnect = now;


            if (mqttClient.connect("sn_esp32", mqttUserName.c_str(), mqttPassword.c_str(), mqttAvailability.c_str(),2,true,"offline")) {
              debugI("MQTT connected");
    
              String subTopic = mqttBase+"set/#";
              debugI("Subscribing to topic %s", subTopic.c_str());
              mqttClient.subscribe(subTopic.c_str());

              mqttClient.publish(mqttAvailability.c_str(),"online",true);
              autoDiscoveryPublished = false;
            } else {
              debugW("MQTT connection failed");
            }

          }
        } else {
          if (!autoDiscoveryPublished) {  // This is the setup area, gets called once when communication with Spa and MQTT broker have been established.
            debugI("Publish autodiscovery information");
            mqttHaAutoDiscovery();
            autoDiscoveryPublished = true;
            si.setUpdateCallback(mqttPublishStatus);
            mqttPublishStatus();

            si.statusResponse.setCallback(mqttPublishStatusString);

          }
          #if defined(LED_PIN)
          led.setInterval(2000);
          #endif
        }
      }
    }
  }
}