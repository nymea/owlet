#include "gpiocontroller.h"
#include "localstorage.h"


GPIOController::GPIOController()
{
}

GPIOController::GPIOError GPIOController::configurePin(int id, GPIOController::PinMode mode)
{
    Serial.println(String("Configuring Pin ") + id + " as " + mode);
    switch (mode) {
    case PinModeGPIOInput:
        pinMode(id, INPUT);
        break;
    case PinModeGPIOOutput:
        pinMode(id, OUTPUT);
        break;
#ifdef USE_WS2812FX
    case PinModeWS2812:
        break;
#endif
    case PinModeUnconfigured:
        break;
    }

    m_pinModes[id] = mode;
    return GPIOErrorNoError;
}

int GPIOController::gpioCount() const
{
    return 6;
}

GPIOController::PinMode GPIOController::getPinMode(int id) const
{
    return m_pinModes[id];
}

GPIOController::GPIOError GPIOController::setGPIOPower(int id, bool power)
{
    int idx = m_pinModes.find(id);
    if (idx == -1) {
        return GPIOErrorUnconfigured;
    }
    if (m_pinModes[id] != PinModeGPIOOutput) {
        return GPIOErrorConfigurationMismatch;
    }
    digitalWrite(id, power ? HIGH : LOW);

    for (unsigned int i = 0; i < m_powerChangedHandlers.length(); i++) {
        m_powerChangedHandlers[i](id, power);
    }

    return GPIOErrorNoError;
}

void GPIOController::onPowerChanged(PowerChangedHandlerFunction callback)
{
    m_powerChangedHandlers.add(callback);
}


#ifdef USE_WS2812FX
GPIOController::GPIOError GPIOController::configureWS2812(int id, int ledCount, WS2812Mode mode, WS2812Clock clock)
{
    if (m_pinModes[id] != PinModeWS2812) {
        return GPIOErrorConfigurationMismatch;
    }

    int idx = m_ws2812map.find(id);
    if (idx >= 0) {
        delete m_ws2812map[id];
        m_ws2812map.erase(id);
    }

    WS2812FX *ws2812fx = new WS2812FX(ledCount, id, mode + clock);
    ws2812fx->init();

    ws2812fx->setSpeed(200);
//    ws2812fx->setColor(0x00ff00);
//    ws2812fx->setMode(FX_MODE_STATIC);
    m_ws2812map[id] = ws2812fx;
    setWs2812Power(id, false);
    return GPIOErrorNoError;
}

void GPIOController::onWs2812BrightnessChanged(BrightnessChangedHandlerFunction callback)
{
    m_brightnessChangedHandlers.add(callback);
}

void GPIOController::onWs2812ColorChanged(ColorChangedHandlerFunction callback)
{
    m_colorChangedHandlers.add(callback);
}

void GPIOController::onWs2812EffectChanged(EffectChangedHandlerFunction callback)
{
    m_effectChangedHandlers.add(callback);
}

GPIOController::GPIOError GPIOController::setWs2812Power(int id, bool power)
{
    int idx = m_pinModes.find(id);
    if (idx == -1) {
        return GPIOErrorUnconfigured;
    }
    if (m_pinModes[id] != PinModeWS2812) {
        return GPIOErrorConfigurationMismatch;
    }
    WS2812FX *ws2812fx = m_ws2812map[id];
    if (!ws2812fx) {
        return GPIOErrorUnconfigured;
    }
    if (power) {
        ws2812fx->start();
    } else {
        ws2812fx->stop();
    }

    for (unsigned int i = 0; i < m_powerChangedHandlers.length(); i++) {
        m_powerChangedHandlers[i](id, power);
    }

    return GPIOErrorNoError;
}

GPIOController::GPIOError GPIOController::setWs2812Brightness(int id, int brightness)
{
    int idx = m_pinModes.find(id);
    if (idx == -1) {
        return GPIOErrorUnconfigured;
    }
    if (m_pinModes[id] != PinModeWS2812) {
        return GPIOErrorConfigurationMismatch;
    }
    Serial.println(String("Setting ws2812 brightness: ") + brightness);
    WS2812FX *ws2812fx = m_ws2812map[id];
    if (!ws2812fx) {
        return GPIOErrorUnconfigured;
    }
    ws2812fx->setBrightness(brightness);

    for (unsigned int i = 0; i < m_brightnessChangedHandlers.length(); i++) {
        m_brightnessChangedHandlers[i](id, brightness);
    }

    return GPIOErrorNoError;
}

GPIOController::GPIOError GPIOController::setWs2812Color(int id, int color)
{
    int idx = m_pinModes.find(id);
    if (idx == -1) {
        return GPIOErrorUnconfigured;
    }
    if (m_pinModes[id] != PinModeWS2812) {
        return GPIOErrorConfigurationMismatch;
    }
    WS2812FX *ws2812fx = m_ws2812map[id];
    if (!ws2812fx) {
        return GPIOErrorUnconfigured;;
    }
    Serial.println(String("Setting ws2812 color: ") + color);
    ws2812fx->setColor(color);

    for (unsigned int i = 0; i < m_colorChangedHandlers.length(); i++) {
        m_colorChangedHandlers[i](id, color);
    }

    return GPIOErrorNoError;
}

GPIOController::GPIOError GPIOController::setWs2812Effect(int id, int effect)
{
    int idx = m_pinModes.find(id);
    if (idx == -1) {
        return GPIOErrorUnconfigured;
    }
    if (m_pinModes[id] != PinModeWS2812) {
        return GPIOErrorConfigurationMismatch;
    }
    WS2812FX *ws2812fx = m_ws2812map[id];
    if (!ws2812fx) {
        return GPIOErrorUnconfigured;;
    }
    Serial.println(String("Setting effect:")  + effect);
    ws2812fx->setMode(effect);

    for (unsigned int i = 0; i < m_colorChangedHandlers.length(); i++) {
        m_effectChangedHandlers[i](id, effect);
    }

    return GPIOErrorNoError;
}
#endif

void GPIOController::loop()
{
    ustd::array<int> pins = m_pinModes.keysArray();
    for (unsigned int i = 0; i < pins.length(); i++) {
        int pin = pins[i];
        if (m_pinModes[pin] == PinModeGPIOInput) {
            int state = digitalRead(pin);
            if (state != m_inputStates[pin]) {
                Serial.println(String("Pin ") + pin + " state changed: " + state);
                m_inputStates[pin] = state;
                for (int j = 0; j < m_powerChangedHandlers.length(); j++) {
                    Serial.println("Calling callback");
                    m_powerChangedHandlers[j](pin, state == HIGH);
                }
            }
        }
    }

#ifdef USE_WS2812FX
    ustd::array<int> keys = m_ws2812map.keysArray();
    for (unsigned int i = 0; i < keys.length(); i++) {
        int key = keys[i];
        m_ws2812map[key]->service();
    }
#endif
}

