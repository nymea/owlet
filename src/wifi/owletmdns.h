#ifndef OWLET_MDNS_H
#define OWLER_MDNS_H

#ifdef ESP32
    #include <ESPmDNS.h>
#else
    #define NO_GLOBAL_MDNS
    #include <ESP8266mDNS.h>
#endif


class OwletMDNS
{
public:
    OwletMDNS();

    void begin();

    void update();

private:
#ifdef ESP32
    MDNSResponder m_mdns;
#elif defined ESP8266
    esp8266::MDNSImplementation::MDNSResponder m_mdns;
#else
    error("MDNS enabled but not supported on this platform. (-DUSE_MDNS).")
#endif
};

#endif
