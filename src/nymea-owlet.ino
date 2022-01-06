/*

*/

#define NO_GLOBAL_INSTANCES

#include "gpiocontroller.h"
#include "platform.h"
#include "localstorage.h"

#include "api/apiserver.h"
#include "api/gpiohandler.h"

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

#define VERSION "0.0.1"

GPIOController gpioController;
APIServer apiServer;

void setup() {
    Serial.begin(115200);
//    printWelcome();

#ifdef USE_M5STICKC
    M5StickCHelper *m5StickCHelper = new M5StickCHelper();
#endif

    Storage.begin();

#ifdef USE_WIFI
    wifiManager.begin();

#ifdef USE_M5STICKC
    m5StickCHelper->updateIP(wifiManager.ip());
#endif

    webServer.enableWiFiManager(&wifiManager);
    webServer.enableGPIOController(&gpioController);

#ifdef USE_OTA
    webServer.enableOTA(&otaManager);
#endif

    webServer.begin();

    mdns.begin();
#endif

    apiServer.registerHandler(new GPIOHandler(&gpioController));

#ifdef USE_OTA
    apiServer.registerHandler(new OTAHandler());
#endif

#ifdef USE_WIFI
    apiServer.registerTransport(new TcpTransport());
#endif
}

void loop()
{
    gpioController.loop();

#ifdef USE_OTA
    otaManager.loop();
#endif

#ifdef USE_WIFI
    wifiManager.loop();
    mdns.update();
#endif
}

//void printWelcome() {
//    Serial.println("");
//    Serial.println("     .");
//    Serial.println("     ++,");
//    Serial.println("    |`--`+-.");
//    Serial.println("     ``--`-++. .;;+.");
//    Serial.println("     \\``--*++++;;;/@\\          _ __  _   _ _ __ ___   ___  __ _");
//    Serial.println("      \\`*#;.++++\\;+|/         | '_ \\| | | | '_ ` _ \\ / _ \\/ _` |");
//    Serial.println("       `-###+++++;`           | | | | |_| | | | | | |  __/ (_| |");
//    Serial.println("          /###+++             |_| |_|\\__, |_| |_| |_|\\___|\\__,_|");
//    Serial.println("          |+++#`                      __/ |           ");
//    Serial.println("          `###+.                     |___/            O W L E T");
//    Serial.print("           `###+                                      ");
//    Serial.println(VERSION);
//    Serial.println("             `#+");
//    Serial.println("               `");
//    Serial.println("");
//}

