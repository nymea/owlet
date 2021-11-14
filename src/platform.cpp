#include "platform.h"
#include "debugutils.h"
#include "owlet.h"

Platform::Platform()
{

}

String Platform::deviceId() const
{
#ifdef ESP32
    uint64_t chipId = ESP.getEfuseMac(); // The chip ID is essentially its MAC address(length: 6 bytes).
#elif defined ESP8266
    uint64_t chipId = ESP.getChipId();
#else
#warning "Platform does not provide a serial number implementation."
    uint64_t chipId = 0;
#endif

    uint16_t chip = (uint16_t)(chipId >> 32);
    char serial[13];
    snprintf(serial, 13, "%04X%08X", chip, (uint32_t)chipId);
    return String(serial);
}

String Platform::type() const
{
#ifdef USE_M5STICKC
    return "M5Stick-C";
#elif defined ESP32
    return "ESP32";
#elif defined ESP8266
    return "ESP8266";
#elif defined ARDUINO_UNO
    return "Arduino Uno";
#elif defined ARDUINO_MINI_PRO
    return "Arduino Mini Pro";
#endif 
    return "Unknown";
}

String Platform::firmwareVersion() const
{
    return FIRMWARE_VERSION;
}