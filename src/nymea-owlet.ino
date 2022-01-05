/*

*/

#ifndef __UNO__
#define NO_GLOBAL_INSTANCES
#endif

#include <Arduino.h>

#include "gpiocontroller.h"
#include "platform.h"
#include "debugutils.h"
#include "owlet.h"

#ifdef USE_M5STICKC
    #include "m5stickc/m5stickchelper.h"
#endif

#ifdef USE_WIFI
    #include "wifi/wifimanager.h"
    #include "wifi/owletwebserver.h"
    #include "wifi/owletmdns.h"
    #include "wifi/tcptransport.h"

    WiFiManager wifiManager;
    OwletWebServer webServer(80);
    OwletMDNS mdns;
#endif


#ifdef USE_OTA
    #include "ota/otahandler.h"
    #include "ota/otamanager.h"
    OTAManager otaManager;
#endif

#ifdef USE_UART_TRANSPORT
    #include "uart/uartapiserver.h"
    UartApiServer *uartApiServer;
#else
    #include "api/apiserver.h"
    #include "api/gpiohandler.h"
    #include "api/platformhandler.h"
    APIServer *apiServer;
#endif

GPIOController gpioController;

void setup() 
{

#ifdef USE_UART_TRANSPORT
    uartApiServer = new UartApiServer(Serial, &gpioController);
    uartApiServer->init();
#else
    Serial.begin(115200);
#endif

#ifdef USE_M5STICKC
    M5StickCHelper *m5StickCHelper = new M5StickCHelper();
#endif


#ifdef USE_WIFI
    wifiManager.begin();
#ifdef USE_M5STICKC
    m5StickCHelper->updateIP(wifiManager.ip());
#endif

#ifndef USE_UART
    webServer.enableWiFiManager(&wifiManager);
    webServer.enableGPIOController(&gpioController);
#endif

#ifdef USE_OTA
    webServer.enableOTA(&otaManager);
#endif

    webServer.begin();
    mdns.begin();
#endif // USE_WIFI


#ifndef USE_UART_TRANSPORT
    apiServer = new APIServer();
    apiServer->registerHandler(new PlatformHandler());
    apiServer->registerHandler(new GPIOHandler(&gpioController));
#endif

#ifdef USE_OTA
    apiServer->registerHandler(new OTAHandler());
#endif

#ifdef USE_WIFI
    apiServer->registerTransport(new TcpTransport());
#endif

}

void loop()
{
    gpioController.loop();

#ifdef USE_OTA
    otaManager.loop();
#endif

#ifdef USE_WIFI
     mdns.update();
#endif

#ifdef USE_UART_TRANSPORT
    uartApiServer->loop();
#endif

}


//void printWelcome() {
//    DebugPrintln("");
//    DebugPrintln("     .");
//    DebugPrintln("     ++,");
//    DebugPrintln("    |`--`+-.");
//    DebugPrintln("     ``--`-++. .;;+.");
//    DebugPrintln("     \\``--*++++;;;/@\\          _ __  _   _ _ __ ___   ___  __ _");
//    DebugPrintln("      \\`*#;.++++\\;+|/         | '_ \\| | | | '_ ` _ \\ / _ \\/ _` |");
//    DebugPrintln("       `-###+++++;`           | | | | |_| | | | | | |  __/ (_| |");
//    DebugPrintln("          /###+++             |_| |_|\\__, |_| |_| |_|\\___|\\__,_|");
//    DebugPrintln("          |+++#`                      __/ |           ");
//    DebugPrintln("          `###+.                     |___/            O W L E T");
//    DebugPrint("           `###+                                      ");
//    DebugPrintln(VERSION);
//    DebugPrintln("             `#+");
//    DebugPrintln("               `");
//    DebugPrintln("");
//}

