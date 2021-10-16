#include "otahandler.h"

OTAHandler::OTAHandler():
    APIHandler("OTA")
{
    registerMethod("GetVersion", &OTAHandler::GetVersion);
}

JSONVar OTAHandler::GetVersion(APIHandler *thiz, const JSONVar &data)
{
    Serial.println("Version called!");
    JSONVar json;
    json["version"] = "0.0.0";
    return json;
}
