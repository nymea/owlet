#ifndef LOCALSTORAGE_H
#define LOCALSTORAGE_H

#include <Arduino.h>
#include <Arduino_JSON.h>

class LocalStorage
{
public:
    LocalStorage();

    static void store(const String &key, const JSONVar &value);
    static JSONVar load(const String &configName);
};

#endif // LOCALSTORAGE_H
