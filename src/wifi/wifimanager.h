#ifndef WIFIMANAGER_H
#define WIFIMANAGER_H

#include <Arduino.h>

class WiFiManager
{
public:
    typedef struct {
      bool valid;
      bool ap;
      String ssid;
      String password;
    } WiFiConfig;


    WiFiManager();

    void configure(bool hostAP, const String &ssid, const String &password, bool reconnectNow);

    void begin();

    WiFiConfig wifiConfig() const;

    String ip() const;

private:

    bool connectToWiFi(WiFiConfig wifiConfig);

    WiFiConfig loadWiFiConfig() const;
    void storeWiFiConfig(WiFiConfig wifiConfig);
};

#endif // WIFIMANAGER_H
