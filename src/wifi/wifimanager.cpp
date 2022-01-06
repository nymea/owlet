#include "wifimanager.h"
#include "localstorage.h"
#include "platform.h"

#include <Arduino.h>
#ifdef ESP32
#include <WiFi.h>
#elif defined(ESP8266)
#include <ESP8266WiFi.h>
#endif

#define TESTING_MODE 1

unsigned long previousMillis = 0;
unsigned long interval = 30000;

WiFiManager::WiFiManager()
{

}

void WiFiManager::configure(bool hostAP, const String &ssid, const String &password, bool reconnectNow)
{
    Serial.println("Configuring WiFi to SSID: " + ssid);
    WiFiConfig config;
    config.ap = hostAP;
    config.ssid = ssid;
    config.password = password;
    config.valid = true;
    storeWiFiConfig(config);

    if (reconnectNow) {
        begin();
    }
}

void WiFiManager::begin()
{
    WiFiConfig wifiConfig = loadWiFiConfig();
    bool wifiConnected = false;
    if (wifiConfig.valid) {
        Serial.print("Loaded wifi ssid: ");
        Serial.println(wifiConfig.ssid);
        wifiConnected = connectToWiFi(wifiConfig);
    }

    if (!wifiConnected) {
        Platform platform;
        String deviceId = platform.deviceId();
        Serial.println("Device serial is: " + deviceId);
        String wifiName = "nymea-owlet-" + deviceId.substring(deviceId.length() - 6, deviceId.length());
        WiFi.softAP(wifiName.c_str());
    }
}

WiFiManager::WiFiConfig WiFiManager::wifiConfig() const
{
    return loadWiFiConfig();
}

String WiFiManager::ip() const
{
    return WiFi.localIP().toString();
}

void WiFiManager::loop()
{
    unsigned long currentMillis = millis();
    // if WiFi is down, try reconnecting every CHECK_WIFI_TIME seconds
    if ((WiFi.status() != WL_CONNECTED) && (currentMillis - previousMillis >=interval)) {
      Serial.print(millis());
      Serial.println("Reconnecting to WiFi...");
      WiFi.disconnect();
      WiFi.reconnect();
      previousMillis = currentMillis;
    }
}

bool WiFiManager::connectToWiFi(WiFiConfig wifiConfig)
{
    WiFi.mode(WIFI_STA);
    Serial.print("Connecting to WiFi network ");
    Serial.println(wifiConfig.ssid);
    WiFi.begin(wifiConfig.ssid.c_str(), wifiConfig.password.c_str());

    for (int i = 0; WiFi.status() != WL_CONNECTED && i < 30; i++) {
        delay(500);
        Serial.print(".");
    }
    if (WiFi.status() != WL_CONNECTED) {
        Serial.println("Failed to connect to WiFi");
        return false;
    }
    Serial.print("Connected to WiFi ");
    Serial.print(wifiConfig.ssid);
    Serial.print(". IP address: ");
    Serial.println(WiFi.localIP());
    return true;
}

WiFiManager::WiFiConfig WiFiManager::loadWiFiConfig() const
{
    WiFiConfig config;
    config.valid = false;

    JSONVar json = Storage.load("wificonfig");

    Serial.println("loaded wifi config");

    if (JSON.typeof(json) != "undefined") {
        Serial.println("Have a WiFi config");
        config.ap = json.hasOwnProperty("ap_mode") && json["ap_mode"];
        const char *ssid = json.hasOwnProperty("ssid") ? json["ssid"] : "";
        config.ssid = ssid;
        const char *password = json.hasOwnProperty("password") ? json["password"] : "";
        config.password = password;
        Serial.println(String("Loaded wifi config: ") + (config.ap ? "AP" : "Client") + " SSID: " + config.ssid + ", Pass: " + config.password);
        config.valid = true;
    } else {
        Serial.println("failed to load json config");
    }

    return config;
}

void WiFiManager::storeWiFiConfig(WiFiConfig wifiConfig)
{
    JSONVar json;
    json["ap_mode"] = wifiConfig.ap;
    json["ssid"] = wifiConfig.ssid;
    json["password"] = wifiConfig.password;

    Storage.store("wificonfig", json);
}
