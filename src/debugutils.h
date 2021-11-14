#ifndef DEBUGUTILS_H
#define DEBUGUTILS_H

#include <Arduino.h>

#ifdef SERIAL_DEBUG_ENABLED 
  #define DebugPrint(...) \
        Serial.print(millis()); \
        Serial.print(": "); \
        Serial.print(__VA_ARGS__)
  #define DebugPrintln(...) \
        Serial.print(millis()); \
        Serial.print(': '); \
        Serial.println(__VA_ARGS__)
#else
  #define DebugPrint(...)
  #define DebugPrintln(...)  
#endif // SERIAL_DEBUG_ENABLED

#endif // DEBUGUTILS_H