#include "wifimanager.h"
#include "localstorage.h"
#include "debugutils.h"

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
        DebugPrint("Loaded wifi ssid: ");
        DebugPrintln(wifiConfig.ssid);
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
    DebugPrint("Connecting to WiFi network ");
    DebugPrintln(wifiConfig.ssid);
    WiFi.begin(wifiConfig.ssid.c_str(), wifiConfig.password.c_str());

    for (int i = 0; WiFi.status() != WL_CONNECTED && i < 30; i++) {
        delay(500);
        DebugPrint(".");
    }
    if (WiFi.status() != WL_CONNECTED) {
        DebugPrintln("Failed to connect to WiFi");
        return false;
    }
    DebugPrint("Connected to WiFi ");
    DebugPrint(wifiConfig.ssid);
    DebugPrint(". IP address: ");
    DebugPrintln(WiFi.localIP());
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
        DebugPrintln(String("Loaded wifi config: ") + (config.ap ? "AP" : "Client") + " SSID: " + config.ssid + ", Pass: " + config.password);
        config.valid = true;
    } else {
        DebugPrintln("failed to load json config");
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
