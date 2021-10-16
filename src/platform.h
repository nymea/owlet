#ifndef PLATFORM_H
#define PLATFORM_H

#include <Arduino.h>
//#include <StringArray.h>

class Platform {
public:
    Platform();
    String deviceId() const;
    String type() const;
};

#endif
