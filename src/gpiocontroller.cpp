#include "gpiocontroller.h"
#include "debugutils.h"

GPIOController::GPIOController()
{
    
}

GPIOController::GPIOError GPIOController::configurePin(uint8_t id, GPIOController::PinMode mode)
{
    DebugPrintln(String("Configuring Pin ") + id + " as " + mode);
    
    switch (mode) {
    case PinModeGPIOInput:
        pinMode(id, INPUT);
        break;
    case PinModeGPIOOutput:
        pinMode(id, OUTPUT);
        break;
    case PinModeAnalogInput:
        // Configure to input and read once the value
        pinMode(id, INPUT);
        analogRead(id);
        break;
    case PinModeAnalogOutput:
        // Configure to input and read once the value
        pinMode(id, OUTPUT);
        analogWrite(id, 0);
        break;
    case PinModeServo: {
        int index = m_servos.find(id);
        if (index >= 0) {
            m_servos[id].detach();
            m_servos.erase(id);
        }

        m_servos[id] = Servo();
        uint8_t channel = m_servos[id].attach(id);
        if (channel == INVALID_SERVO) {
            m_servos.erase(id);
            return GPIOErrorInvalidPin;
        }
        break;
    }
#ifdef USE_WS2812FX
    case PinModeWS2812:
        break;
#endif
    case PinModeUnconfigured: {
        if (m_servos.find(id) >= 0) {
            m_servos[id].detach();
            m_servos.erase(id);
        }

        // Default to input
        pinMode(id, INPUT);
        break;
    }
    }

    m_pinModes[id] = mode;
    return GPIOErrorNoError;
}

uint8_t GPIOController::gpioCount() const
{
    return 6;
}

GPIOController::PinMode GPIOController::getPinMode(uint8_t id) const
{
    return m_pinModes[id];
}

GPIOController::GPIOError GPIOController::setGPIOPower(uint8_t id, bool power)
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

int GPIOController::readDigitalValue(uint8_t id)
{
    return digitalRead(id);
}

int GPIOController::readAnalogValue(uint8_t id)
{
    return analogRead(id);
}

GPIOController::GPIOError GPIOController::writeAnalogValue(uint8_t id, uint8_t value)
{
    int idx = m_pinModes.find(id);
    if (idx == -1) {
        return GPIOErrorUnconfigured;
    }

    if (m_pinModes[id] != PinModeAnalogOutput) {
        return GPIOErrorConfigurationMismatch;
    }

    analogWrite(id, value);
    return GPIOErrorNoError;
}

GPIOController::GPIOError GPIOController::writeServoValue(uint8_t id, uint8_t value)
{
    if (m_pinModes.find(id) < 0) {
        return GPIOErrorUnconfigured;
    }

    if (m_pinModes[id] != PinModeServo) {
        return GPIOErrorConfigurationMismatch;
    }

    if (!m_servos[id].attached()) {
        return GPIOErrorUnconfigured;
    }

    m_servos[id].write(value);
    return GPIOErrorNoError;
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
    return GPIOErrorNoError;
}

void GPIOController::onBrightnessChanged(BrightnessChangedHandlerFunction callback)
{
    m_brightnessChangedHandlers.add(callback);
}

void GPIOController::onColorChanged(ColorChangedHandlerFunction callback)
{
    m_colorChangedHandlers.add(callback);
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
    DebugPrintln(String("Setting ws2812 brightness: ") + brightness);
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
//    DebugPrintln(String("Setting ws2812 color: ") + color);
    ws2812fx->setColor(color);
    ws2812fx->setMode(FX_MODE_STATIC);
//    ws2812fx->setMode(effect);

    for (unsigned int i = 0; i < m_colorChangedHandlers.length(); i++) {
        m_colorChangedHandlers[i](id, color);
    }

    return GPIOErrorNoError;
}
#endif

void GPIOController::loop()
{
    ustd::array<uint8_t> pins = m_pinModes.keysArray();
    for (unsigned int i = 0; i < pins.length(); i++) {
        uint8_t pin = pins[i];
        if (m_pinModes[pin] == PinModeGPIOInput) {
            uint8_t state = digitalRead(pin);
            if (state != m_inputStates[pin]) {
                DebugPrintln(String("Pin ") + pin + " state changed: " + state);
                m_inputStates[pin] = state;
                for (unsigned int j = 0; j < m_powerChangedHandlers.length(); j++) {
                    DebugPrintln("Calling callback");
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

