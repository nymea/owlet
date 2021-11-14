#include "localstorage.h"
#include "debugutils.h"

#ifdef ESP32
#include <Preferences.h>
#elif defined(ESP8266)
#include <FS.h>
#endif

#ifdef ESP32

LocalStorage::LocalStorage()
{
}

JSONVar LocalStorage::load(const String &configName)
{
    JSONVar ret;

    return ret;
}

void LocalStorage::store(const String &configName, const JSONVar &value)
{

}

#elif defined(ESP8266)

LocalStorage::LocalStorage()
{
    if (!SPIFFS.begin()) {
      DebugPrintln("Error opening SPIFFS. Cannot store settings.");
    }
}

JSONVar LocalStorage::load(const String &configName)
{
    File configFile = SPIFFS.open("/" + configName + ".json", "r");
    if (!configFile) {
        DebugPrintln("No configuration file found.");
        return JSONVar();
    }

    // Allocate a buffer to store contents of the file.
    size_t size = configFile.size();
    std::unique_ptr<char[]> jsonBuf(new char[size]);
    configFile.readBytes(jsonBuf.get(), size);
    configFile.close();

    return JSON.parse(jsonBuf.get());
}

void LocalStorage::store(const String &configName, const JSONVar &value)
{
    File configFile = SPIFFS.open("/" + configName + ".json", "w");
    if (!configFile) {
        DebugPrintln("Error opening config file for writing.");
        return;
    }
    configFile.write(JSON.stringify(value).c_str());
    configFile.close();
}

#endif
