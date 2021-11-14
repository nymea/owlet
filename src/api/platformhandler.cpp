#include "platformhandler.h"

PlatformHandler::PlatformHandler() :
    APIHandler("Platform")
{
    registerMethod("GetInformation", &PlatformHandler::GetInformation);

}

JSONVar PlatformHandler::GetInformation(APIHandler *thiz, const JSONVar &data)
{
    Platform platform;
    JSONVar ret;
    ret["deviceId"] = platform.deviceId();
    ret["type"] = platform.type();
    return ret;
}