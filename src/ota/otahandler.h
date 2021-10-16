#ifndef OTAHANDLER_H
#define OTAHANDLER_H

#include "api/apihandler.h"

class OTAHandler: public APIHandler
{

public:
    OTAHandler();

    static JSONVar GetVersion(APIHandler *thiz, const JSONVar &data);
};

#endif
