#ifndef LOCALSTORAGE_H
#define LOCALSTORAGE_H

#include <Arduino.h>
#include <Arduino_JSON.h>

class LocalStorage
{
public:
    LocalStorage();

    void begin();

    void store(const String &key, const JSONVar &value);
    JSONVar load(const String &configName);
};

extern LocalStorage Storage;

#endif // LOCALSTORAGE_H
