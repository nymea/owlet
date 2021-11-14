#ifndef PLATFORMHANDLER_H
#define PLATFORMHANDLER_H

#include "api/apihandler.h"
#include "platform.h"

class PlatformHandler: public APIHandler
{
public:
    PlatformHandler();

    static JSONVar GetInformation(APIHandler *thiz, const JSONVar &data);

};

#endif // PLATFORMHANDLER_H