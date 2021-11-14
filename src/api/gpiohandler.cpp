#include "gpiohandler.h"

GPIOHandler::GPIOHandler(GPIOController *controller):
    APIHandler("GPIO"),
    m_controller(controller)
{
    registerMethod("ConfigurePin", &GPIOHandler::ConfigurePin);
    registerMethod("ControlPin", &GPIOHandler::ControlPin);

    m_controller->onPowerChanged([=](int id, bool power){
        JSONVar params;
        params["id"] = id;
        params["power"] = power;
        sendNotification("PinChanged", params);
    });

#ifdef USE_WS2812FX
    m_controller->onWs2812BrightnessChanged([=](int id, int brightness){
        JSONVar params;
        params["id"] = id;
        params["brightness"] = brightness;
        sendNotification("PinChanged", params);
    });
    m_controller->onWs2812ColorChanged([=](int id, int color){
        JSONVar params;
        params["id"] = id;
        params["color"] = color;
        sendNotification("PinChanged", params);
    });
    m_controller->onWs2812EffectChanged([=](int id, int effect){
        JSONVar params;
        params["id"] = id;
        params["effect"] = effect;
        sendNotification("PinChanged", params);
    });
#endif
}

JSONVar GPIOHandler::ConfigurePin(APIHandler *thiz, const JSONVar &data)
{
    Serial.println("Configure GPIO called");
    GPIOHandler *self = static_cast<GPIOHandler*>(thiz);
    JSONVar ret;

    JSONVar obj(data);
    Serial.println(obj["id"]);

    int id = obj["id"];
    GPIOController::PinMode pinMode = stringToPinMode((const char*)obj["mode"]);
    GPIOController::GPIOError status = self->m_controller->configurePin(id, pinMode);

    if (status != GPIOController::GPIOErrorNoError) {
        ret["status"] = errorToString(status);
        return ret;
    }

    switch (pinMode) {
    case GPIOController::PinModeGPIOInput:
        break;
    case GPIOController::PinModeGPIOOutput:
        break;
#ifdef USE_WS2812FX
    case GPIOController::PinModeWS2812:
        status = self->m_controller->configureWS2812(id, obj["ledCount"], stringToWS2812Mode((const char*)obj["ledMode"]), stringToWS2812Clock((const char*)obj["ledClock"]));
        break;
#endif
    }

    ret["error"] = errorToString(status);
    return ret;
}

JSONVar GPIOHandler::ControlPin(APIHandler *thiz, const JSONVar &data)
{
    GPIOHandler *self = static_cast<GPIOHandler*>(thiz);
    GPIOController *controller = self->m_controller;

    GPIOController::GPIOError status = GPIOController::GPIOErrorConfigurationMismatch;

    JSONVar obj(data);
    int id = obj["id"];
    Serial.println("Control GPIO called: " + JSON.stringify(obj));
    switch (controller->getPinMode(id)) {
    case GPIOController::PinModeUnconfigured:
        status = GPIOController::GPIOErrorUnconfigured;
        break;
    case GPIOController::PinModeGPIOInput:
        break;
    case GPIOController::PinModeGPIOOutput:
        status = controller->setGPIOPower(id, obj["power"]);
        break;
#ifdef USE_WS2812FX
    case GPIOController::PinModeWS2812:
        if (obj.hasOwnProperty("power")) {
            status = controller->setWs2812Power(id, obj["power"]);
        }
        if (obj.hasOwnProperty("brightness")) {
            status = controller->setWs2812Brightness(id, obj["brightness"]);
        }
        if (obj.hasOwnProperty("color")) {
            status = controller->setWs2812Color(id, obj["color"]);
        }
        if (obj.hasOwnProperty("effect")) {
            Serial.println("Set effect!");
            status = controller->setWs2812Effect(id, obj["effect"]);
        }
        break;
#endif
    }

    JSONVar ret;
    ret["error"] = errorToString(status);
    return ret;
}

String GPIOHandler::errorToString(GPIOController::GPIOError error)
{
    switch (error) {
    case GPIOController::GPIOErrorNoError:
        return "GPIOErrorNoError";
    case GPIOController::GPIOErrorUnconfigured:
        return "GPIOErrorUnconfigured";
    case GPIOController::GPIOErrorConfigurationMismatch:
        return "GPIOErrorConfigurationMisMatch";
    }
    return "";
}

GPIOController::PinMode GPIOHandler::stringToPinMode(const String &string)
{
    if (string == "GPIOInput") {
        return GPIOController::PinModeGPIOInput;
    }
    if (string == "GPIOOutput") {
        return GPIOController::PinModeGPIOOutput;
    }
#ifdef USE_WS2812FX
    if (string == "WS2812") {
        return GPIOController::PinModeWS2812;
    }
#endif
    return GPIOController::PinModeGPIOOutput;
}

#ifdef USE_WS2812FX
GPIOController::WS2812Mode GPIOHandler::stringToWS2812Mode(const String &string)
{
    Serial.println("Configuring as " + string);

    if (string == "WS2812ModeRGB") { return GPIOController::WS2812ModeRGB; }
    if (string == "WS2812ModeRBG") { return GPIOController::WS2812ModeRBG; }
    if (string == "WS2812ModeGRB") { return GPIOController::WS2812ModeGRB; }
    if (string == "WS2812ModeGBR") { return GPIOController::WS2812ModeGBR; }
    if (string == "WS2812ModeBRG") { return GPIOController::WS2812ModeBRG; }
    if (string == "WS2812ModeBGR") { return GPIOController::WS2812ModeBGR; }

    if (string == "WS2812ModeWRGB") { return GPIOController::WS2812ModeWRGB; }
    if (string == "WS2812ModeWRBG") { return GPIOController::WS2812ModeWRBG; }
    if (string == "WS2812ModeWGRB") { return GPIOController::WS2812ModeWGRB; }
    if (string == "WS2812ModeWGBR") { return GPIOController::WS2812ModeWGBR; }
    if (string == "WS2812ModeWBRG") { return GPIOController::WS2812ModeWBRG; }
    if (string == "WS2812ModeWBGR") { return GPIOController::WS2812ModeWBGR; }

    if (string == "WS2812ModeRWGB") { return GPIOController::WS2812ModeRWGB; }
    if (string == "WS2812ModeRWBG") { return GPIOController::WS2812ModeRWBG; }
    if (string == "WS2812ModeRGWB") { return GPIOController::WS2812ModeRGWB; }
    if (string == "WS2812ModeRGBW") { return GPIOController::WS2812ModeRGBW; }
    if (string == "WS2812ModeRBWG") { return GPIOController::WS2812ModeRBWG; }
    if (string == "WS2812ModeRBGW") { return GPIOController::WS2812ModeRBGW; }

    if (string == "WS2812ModeGWRB") { return GPIOController::WS2812ModeGWRB; }
    if (string == "WS2812ModeGWBR") { return GPIOController::WS2812ModeGWBR; }
    if (string == "WS2812ModeGRWB") { return GPIOController::WS2812ModeGRWB; }
    if (string == "WS2812ModeGRBW") { return GPIOController::WS2812ModeGRBW; }
    if (string == "WS2812ModeGBWR") { return GPIOController::WS2812ModeGBWR; }
    if (string == "WS2812ModeGBRW") { return GPIOController::WS2812ModeGBRW; }

    if (string == "WS2812ModeBWRG") { return GPIOController::WS2812ModeBWRG; }
    if (string == "WS2812ModeBWGR") { return GPIOController::WS2812ModeBWGR; }
    if (string == "WS2812ModeBRWG") { return GPIOController::WS2812ModeBRWG; }
    if (string == "WS2812ModeBRGW") { return GPIOController::WS2812ModeBRGW; }
    if (string == "WS2812ModeBGWR") { return GPIOController::WS2812ModeBGWR; }
    if (string == "WS2812ModeBGRW") { return GPIOController::WS2812ModeBGRW; }

    return GPIOController::WS2812ModeRGB;
}

GPIOController::WS2812Clock GPIOHandler::stringToWS2812Clock(const String &string)
{
    if (string == "WS2812Clock400kHz") {
        return GPIOController::WS2812Clock400kHz;
    }
    if (string == "WS2812Clock800kHz") {
        return GPIOController::WS2812Clock800kHz;
    }
    return GPIOController::WS2812Clock800kHz;
}
#endif
