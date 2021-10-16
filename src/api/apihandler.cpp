#include "apihandler.h"

APIHandler::APIHandler(const String &nameSpace):
    m_nameSpace(nameSpace)
{

}

String APIHandler::nameSpace() const
{
    return m_nameSpace;
}

ustd::array<APIMethod> APIHandler::methods() const
{
    return m_methods;
}

void APIHandler::registerMethod(String name, APIMethodFunc func)
{
    APIMethod method;
    method.name = name;
    method.func = func;
    m_methods.add(method);
}

void APIHandler::sendNotification(const String &name, const JSONVar &params)
{
    m_notificationFunc(this, m_nameSpace + "." +  name, params);
}

void APIHandler::onNotification(NotificationFunc cb)
{
    m_notificationFunc = cb;
}
