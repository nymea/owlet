#ifndef OWLETPLATFORM_H
#define OWLETPLATFORM_H

#include <Arduino.h>
//#include <StringArray.h>

class Platform {
public:
    Platform();
    String deviceId() const;
    String type() const;
    String firmwareVersion() const;
};

#endif // OWLET
