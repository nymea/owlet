#include "localstorage.h"

#if defined(ESP32) || defined(ESP8266)
#include <FS.h>
#ifndef ESP8266
#include <SPIFFS.h>
#endif
#endif

LocalStorage Storage;

LocalStorage::LocalStorage()
{
}

void LocalStorage::begin()
{
    Serial.println("Mounting SPIFFS...");
#if defined ESP32
    if (!SPIFFS.begin(true)) {
#elif defined ESP8266
    if (!SPIFFS.begin()) {
#else
    if (true) {
#endif
      Serial.println("Error mounting SPIFFS. Cannot store settings.");
    }
}

JSONVar LocalStorage::load(const String &configName)
{
#if defined ESP8266 || defined ESP32
    Serial.println("opeing file ");
    File configFile = SPIFFS.open("/" + configName + ".json", "r");
    if (!configFile) {
        Serial.println("No configuration file found.");
        return JSONVar();
    }

    Serial.println("opened file ");
    // Allocate a buffer to store contents of the file.
    size_t size = configFile.size();
    std::unique_ptr<char[]> jsonBuf(new char[size]);
    Serial.println("reading file ");
    configFile.readBytes(jsonBuf.get(), size);
    Serial.println("read file :" + String(jsonBuf.get()));
    configFile.close();

    return JSON.parse(jsonBuf.get());
#else
    return JSONVar();
#endif
}

void LocalStorage::store(const String &configName, const JSONVar &value)
{
#if defined ESP8266 || defined ESP32
    File configFile = SPIFFS.open("/" + configName + ".json", "w");
    if (!configFile) {
        Serial.println("Error opening config file for writing.");
        return;
    }
#ifdef ESP8266
    configFile.write(JSON.stringify(value).c_str());
#else
    configFile.print(JSON.stringify(value).c_str());
#endif

    configFile.close();
#endif
}
