#include "otahandler.h"
#include "debugutils.h"
#include "owlet.h"

OTAHandler::OTAHandler():
    APIHandler("OTA")
{
    registerMethod("GetVersion", &OTAHandler::GetVersion);
}

JSONVar OTAHandler::GetVersion(APIHandler *thiz, const JSONVar &data)
{
    DebugPrintln("Version called!");
    JSONVar json;
    json["version"] = FIRMWARE_VERSION;
    return json;
}
