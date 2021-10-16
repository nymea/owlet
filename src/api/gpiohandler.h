#ifndef GPIOHANDLER_H
#define GPIOHANDLER_H

#include "api/apihandler.h"
#include "gpiocontroller.h"

class GPIOHandler: public APIHandler
{
public:
    GPIOHandler(GPIOController *controller);

    static JSONVar ConfigurePin(APIHandler *thiz, const JSONVar &data);
    static JSONVar ControlPin(APIHandler *thiz, const JSONVar &data);

private:
    static String errorToString(GPIOController::GPIOError error);
    static GPIOController::PinMode stringToPinMode(const String &string);

#ifdef USE_WS2812FX
    static GPIOController::WS2812Mode stringToWS2812Mode(const String &string);
    static GPIOController::WS2812Clock stringToWS2812Clock(const String &string);
#endif

private:
    GPIOController *m_controller = nullptr;

};

#endif // GPIOHANDLER_H
