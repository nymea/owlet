#ifndef M5STICKCHELPER_H
#define M5STICKCHELPER_H

#include <Arduino.h>

class M5StickCHelper {
public:
    M5StickCHelper();

    void updateIP(const String &ip);


private:
    void paint();

    String m_ip;
};

#endif // M5STICKCHELPER_H
