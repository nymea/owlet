#ifndef DEBUGUTILS_H
#define DEBUGUTILS_H

#include <Arduino.h>

#ifdef SERIAL_DEBUG_ENABLED
  #ifdef USE_UART_TRANSPORT
    #define DebugPrint(...) 
    #define DebugPrintln(...)
  #else
    #define DebugPrint(...) \
          Serial.print(__VA_ARGS__)
    #define DebugPrintln(...) \
          Serial.println(__VA_ARGS__)
  #endif // USE_UART_TRANSPORT
#else
  #define DebugPrint(...)
  #define DebugPrintln(...)  
#endif // SERIAL_DEBUG_ENABLED

#endif // DEBUGUTILS_H