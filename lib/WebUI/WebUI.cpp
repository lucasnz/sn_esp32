#include "WebUI.h"

#if defined(ESP8266)
    #define UPDATE_SIZE_UNKNOWN 0XFFFFFFFF
#endif

WebUI::WebUI(SpaInterface *spa, Config *config) {
    _spa = spa;
    _config = config;
}

const char * WebUI::getError() {
    #if defined(ESP8266)
        return Update.getErrorString().c_str();
    #elif defined(ESP32)
        return Update.errorString();
    #endif
}

void WebUI::wifiManagerSaveConfigCallback() {
  _wifiManagerSaveConfig = true;
}

void WebUI::startWiFiManager() {
  if (this->initialised) {
    this->server->stop();
  }

  WiFiManager wm;
  WiFiManagerParameter custom_spa_name("spa_name", "Spa Name", _config->getSpaName().c_str(), 40);
  WiFiManagerParameter custom_mqtt_server("server", "MQTT server", _config->getMqttServer().c_str(), 40);
  WiFiManagerParameter custom_mqtt_port("port", "MQTT port", _config->getMqttPort().c_str(), 6);
  WiFiManagerParameter custom_mqtt_username("username", "MQTT Username", _config->getMqttUserName().c_str(), 20 );
  WiFiManagerParameter custom_mqtt_password("password", "MQTT Password", _config->getMqttPassword().c_str(), 40 );
  wm.addParameter(&custom_spa_name);
  wm.addParameter(&custom_mqtt_server);
  wm.addParameter(&custom_mqtt_port);
  wm.addParameter(&custom_mqtt_username);
  wm.addParameter(&custom_mqtt_password);
  wm.setBreakAfterConfig(true);
  wm.setSaveConfigCallback(std::bind(&WebUI::wifiManagerSaveConfigCallback, this));
  wm.setConnectTimeout(300); //close the WiFiManager after 300 seconds of inactivity

  _wifiManagerSaveConfig = false;

  wm.startConfigPortal();
  debugI("Exiting Portal");

  if (_wifiManagerSaveConfig) {
    _config->setSpaName(String(custom_spa_name.getValue()));
    _config->setMqttServer(String(custom_mqtt_server.getValue()));
    _config->setMqttPort(String(custom_mqtt_port.getValue()));
    _config->setMqttUserName(String(custom_mqtt_username.getValue()));
    _config->setMqttPassword(String(custom_mqtt_password.getValue()));

    _config->writeConfigFile();
  }
}

void WebUI::setSavedConfigCallback(void (*f)()) {
    _savedConfigCallback = f;
}

void WebUI::begin() {
        
    #if defined(ESP8266)
        server.reset(new ESP8266WebServer(80));
    #elif defined(ESP32)
        server.reset(new WebServer(80));
    #endif

    server->on("/", HTTP_GET, [&]() {
        debugD("uri: %s", server->uri().c_str());
        server->sendHeader("Connection", "close");
        server->send(200, "text/html", WebUI::indexPageTemplate);
    });

    server->on("/json", HTTP_GET, [&]() {
        debugD("uri: %s", server->uri().c_str());
        server->sendHeader("Connection", "close");
        String json;
        SpaInterface &si = *_spa;
        if (generateStatusJson(si, json, true)) {
            server->send(200, "text/json", json.c_str());
        } else {
            server->send(200, "text/text", "Error generating json");
        }
    });

    server->on("/reboot", HTTP_GET, [&]() {
        debugD("uri: %s", server->uri().c_str());
        server->send(200, "text/html", WebUI::rebootPage);
        debugD("Rebooting...");
        delay(200);
        server->client().stop();
        ESP.restart();
    });

    server->on("/styles.css", HTTP_GET, [&]() {
        debugD("uri: %s", server->uri().c_str());
        server->send(200, "text/css", WebUI::styleSheet);
    });

    server->on("/fota", HTTP_GET, [&]() {
        debugD("uri: %s", server->uri().c_str());
        server->sendHeader("Connection", "close");
        server->send(200, "text/html", WebUI::fotaPage);
    });

    server->on("/fota", HTTP_POST, [&]() {
        debugD("uri: %s", server->uri().c_str());
        if (Update.hasError()) {
            server->sendHeader("Connection", "close");
            server->send(200, F("text/plain"), String(F("Update error: ")) + String(getError()));
        } else {
            server->client().setNoDelay(true);
            server->sendHeader("Connection", "close");
            server->send(200, "text/plain", "OK");
            debugD("Rebooting...");
            delay(100);
            server->client().stop();
            ESP.restart();
        }
    }, [&]() {
        debugD("uri: %s", server->uri().c_str());
        HTTPUpload& upload = server->upload();
        if (upload.status == UPLOAD_FILE_START) {
            debugD("Update: %s", upload.filename.c_str());
            if (!Update.begin(UPDATE_SIZE_UNKNOWN)) { //start with max available size
                debugD("Update Error: %s",getError());
            }
        } else if (upload.status == UPLOAD_FILE_WRITE) {
            /* flashing firmware to ESP*/
            if (Update.write(upload.buf, upload.currentSize) != upload.currentSize) {
                debugD("Update Error: %s",getError());
            }
        } else if (upload.status == UPLOAD_FILE_END) {
            if (Update.end(true)) { //true to set the size to the current progress
                debugD("Update Success: %u\n", upload.totalSize);
            } else {
                debugD("Update Error: %s",getError());
            }
        }
    });

    server->on("/config", HTTP_GET, [&]() {
        debugD("uri: %s", server->uri().c_str());
        server->sendHeader("Connection", "close");
        server->send(200, "text/html", WebUI::configPageTemplate);
    });

    server->on("/config", HTTP_POST, [&]() {
        debugD("uri: %s", server->uri().c_str());
        if (server->hasArg("spaName")) _config->setSpaName(server->arg("spaName"));
        if (server->hasArg("mqttServer")) _config->setMqttServer(server->arg("mqttServer"));
        if (server->hasArg("mqttPort")) _config->setMqttPort(server->arg("mqttPort"));
        if (server->hasArg("mqttUsername")) _config->setMqttUserName(server->arg("mqttUsername"));
        if (server->hasArg("mqttPassword")) _config->setMqttPassword(server->arg("mqttPassword"));
        if (server->hasArg("updateFrequency")) _config->setUpdateFrequency(server->arg("updateFrequency").toInt());
        _config->writeConfigFile();
        server->sendHeader("Connection", "close");
        server->send(200, "text/plain", "Updated");
        if (_savedConfigCallback != nullptr) { _savedConfigCallback(); }
    });

    server->on("/json/config", HTTP_GET, [&]() {
        debugD("uri: %s", server->uri().c_str());
        String configJson = "{";
        configJson += "\"spaName\":\"" + _config->getSpaName() + "\",";
        configJson += "\"mqttServer\":\"" + _config->getMqttServer() + "\",";
        configJson += "\"mqttPort\":\"" + _config->getMqttPort() + "\",";
        configJson += "\"mqttUsername\":\"" + _config->getMqttUserName() + "\",";
        configJson += "\"mqttPassword\":\"" + _config->getMqttPassword() + "\",";
        configJson += "\"updateFrequency\":" + String(_config->getUpdateFrequency());
        configJson += "}";
        server->send(200, "application/json", configJson);
    });

    server->on("/set", HTTP_POST, [&]() {
        //In theory with minor modification, we can reuse mqttCallback here
        //for (uint8_t i = 0; i < server->args(); i++) updateSpaSetting("set/" + server->argName(0), server->arg(0));
        if (server->hasArg("temperatures_setPoint")) {
            float newTemperature = server->arg("temperatures_setPoint").toFloat();
            SpaInterface &si = *_spa;
            si.setSTMP(int(newTemperature*10));
            server->send(200, "text/plain", "Temperature updated");
        } else {
            server->send(400, "text/plain", "Invalid temperature value");
        }
    });

    server->on("/wifi-manager", HTTP_GET, [&]() {
        debugD("uri: %s", server->uri().c_str());
        WebUI::startWiFiManager();
        server->sendHeader("Connection", "close");
        server->send(200, "text/plain", "WiFi Manager launching, connect to ESP WiFi...");
    });

    server->on("/json.html", HTTP_GET, [&]() {
        debugD("uri: %s", server->uri().c_str());
        server->sendHeader("Connection", "close");
        server->send(200, "text/html", WebUI::jsonHTMLTemplate);
    });

    server->on("/status", HTTP_GET, [&]() {
        debugD("uri: %s", server->uri().c_str());
        SpaInterface &si = *_spa;
        server->sendHeader("Connection", "close");
        server->send(200, "text/plain", si.statusResponse.getValue());
    });

    server->begin();

    initialised = true;
}