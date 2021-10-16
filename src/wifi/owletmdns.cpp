#include "owletmdns.h"

#include "platform.h"

#include <functional>
#include <Arduino.h>
#include <StringArray.h>


OwletMDNS::OwletMDNS()
{

}

void OwletMDNS::begin()
{
    Platform platform;
    String deviceId = platform.deviceId();
    Serial.println("Device serial is: " + deviceId);
    String mdnsName = "nymea-owlet-" + deviceId.substring(deviceId.length() - 6, deviceId.length());

    if(m_mdns.begin(mdnsName.c_str())) {
        m_mdns.addService("nymea-owlet", "tcp", 5555);
        m_mdns.addServiceTxt("nymea-owlet", "tcp", "id", platform.deviceId().c_str());
        m_mdns.addServiceTxt("nymea-owlet", "tcp", "platform", platform.type().c_str());
        Serial.println("mDNS responder started");
    } else {
        Serial.println("Error starting mDNS");
    }
}

void OwletMDNS::update()
{
#ifdef ESP8266
    m_mdns.update();
#endif
}
