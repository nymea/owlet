#ifndef APIHANDLER_H
#define APIHANDLER_H

#include <Arduino.h>
#include <Arduino_JSON.h>

#include <ustd_array.h>

//#include <StringArray.h>
//#include <functional>

class APIHandler;
typedef JSONVar(*APIMethodFunc)(APIHandler *thiz, const JSONVar &data);
typedef void(*NotificationFunc)(APIHandler *thiz, const String &name, const JSONVar &data);

class APIMethod {
  public:
  String name = "";
  APIMethodFunc func;
};

class APIHandler
{
public:
    APIHandler(const String &nameSpace);
    virtual ~APIHandler() = default;

    String nameSpace() const;

    ustd::array<APIMethod> methods() const;


protected:
    void registerMethod(String name, APIMethodFunc func);
//    void registerMethod(String name, void ()(APIHandler *thiz, const JSONVar &data));
    void sendNotification(const String &name, const JSONVar &params);

private:
    String m_nameSpace;
    ustd::array<APIMethod> m_methods;

    friend class APIServer;
    void onNotification(NotificationFunc cb);
    NotificationFunc m_notificationFunc = nullptr;

};

#endif
