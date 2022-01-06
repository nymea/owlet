//#ifdef ESP32
//#include <WiFi.h>
//#elif defined(ESP8266)
//#include <ESP8266WiFi.h>
//#endif
//#include <ESPAsyncTCP.h>
#include "ESPAsyncWebServer.h"

#include <StringArray.h>


class WiFiManager;
class OTAManager;

class OwletWebServer {

public:
  OwletWebServer(int port);
  ~OwletWebServer();

  void begin();

  void enableWiFiManager(WiFiManager *wifiManager);
  void enableOTA(OTAManager *otaManager);

private:
    String processor(const String& var);

    AsyncWebServer m_webServer;

    WiFiManager *m_wifiManager = nullptr;
    OTAManager *m_otaManager = nullptr;
};

