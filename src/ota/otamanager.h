#ifndef OTAMANAGER_H
#define OTAMANAGER_H

#include <Arduino.h>

class OTAManagerPrivate;

class OTAManager
{
public:
    OTAManager();
    ~OTAManager();

    void update(const String &url);

    void loop();


private:
    OTAManagerPrivate *d;
};

#endif // OTAMANAGER_H
