#include "wifimanager.h"
#include "localstorage.h"

#include <Arduino.h>
#ifdef ESP32
#include <WiFi.h>
#elif defined(ESP8266)
#include <ESP8266WiFi.h>
#endif

#define TESTING_MODE 1

WiFiManager::WiFiManager()
{

}

void WiFiManager::configure(bool hostAP, const String &ssid, const String &password, bool reconnectNow)
{
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

#ifndef TESTING_MODE
        // For easier development,
        //        WiFi.softAP(ap_ssid);

#else
        WiFiConfig config;
        config.ssid = "Your SSID";
        config.password = "Your PW";
        connectToWiFi(config);
    }
#endif
}

WiFiManager::WiFiConfig WiFiManager::wifiConfig() const
{
    return loadWiFiConfig();
}

String WiFiManager::ip() const
{
    return WiFi.localIP().toString();
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

    JSONVar json = LocalStorage::load("wificonfig");

    if (JSON.typeof(json) != "undefined") {
        config.ap = (const char*)json["ap"] == "true";
        const char *ssid = json["ssid"];
        config.ssid = ssid;
        const char *password = json["password"];
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

    LocalStorage::store("wificonfig", json);
}
