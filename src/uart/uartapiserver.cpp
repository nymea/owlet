#include "uartapiserver.h"
#include "../debugutils.h"
#include "../owlet.h"

UartApiServer::UartApiServer(HardwareSerial &serial, GPIOController *controller) :
    m_hardwareSerial(&serial),
    m_controller(controller)
{
    m_controller->onPowerChanged([=](uint8_t id, bool power){
        uint8_t payload[2];
        payload[0] = id;
        payload[1] = (power ? 0x01 : 0x00);
        sendNotification(NotificationGpioPinChanged, payload, 2);
    });
}

UartApiServer::~UartApiServer()
{
    m_hardwareSerial->end();
}

void UartApiServer::init()
{
    // Init sequence
    pinMode(LED_BUILTIN, OUTPUT);
    digitalWrite(LED_BUILTIN, HIGH);
    delay(100);
    digitalWrite(LED_BUILTIN, LOW);
    delay(100);
    digitalWrite(LED_BUILTIN, HIGH);
    delay(100);
    digitalWrite(LED_BUILTIN, LOW);

    // Unconfigured
    pinMode(LED_BUILTIN, INPUT);

    m_hardwareSerial->begin(115200);
    delay(100);
    sendNotification(NotificationReady);
}

void UartApiServer::loop()
{
    while (m_hardwareSerial->available()) {
        uint8_t receivedByte = m_hardwareSerial->read();
        processReceivedByte(receivedByte);
    }
}

void UartApiServer::sendData(const char *data, size_t len)
{
    // Stream the data slip encoded
    streamByte(SlipProtocolEnd, true);
    for (size_t i = 0; i < len; i++) {
        streamByte(data[i]);
    }
    streamByte(SlipProtocolEnd, true);

    m_hardwareSerial->flush();
}

void UartApiServer::processReceivedByte(uint8_t receivedByte)
{
    if (m_protocolEscaping) {
        switch (receivedByte) {
        case SlipProtocolTransposedEnd:
            m_buffer[m_bufferIndex++] = SlipProtocolEnd;
            m_protocolEscaping = false;
            break;
        case SlipProtocolTransposedEsc:
            m_buffer[m_bufferIndex++] = SlipProtocolEsc;
            m_protocolEscaping = false;
            break;
        default:
            // SLIP protocol violation...received escape, but it is not an escaped byte
            break;
        }
    }

    switch (receivedByte) {
    case SlipProtocolEnd:
        // We are done with this package, process it and reset the buffer
        if (m_bufferIndex > 0) {
            processData(m_buffer, m_bufferIndex);
        }
        m_bufferIndex = 0;
        m_protocolEscaping = false;
        break;
    case SlipProtocolEsc:
        // The next byte will be escaped, lets wait for it
        m_protocolEscaping = true;
        break;
    default:
        // Nothing special, just add to buffer
        m_buffer[m_bufferIndex++] = receivedByte;
        break;
    }
}

void UartApiServer::streamByte(uint8_t dataByte, boolean specialCharacter)
{
    // If this is a special character, write it without escaping
    if (specialCharacter) {
        writeByte(dataByte);
    } else {
        switch (dataByte) {
        case SlipProtocolEnd:
            writeByte(SlipProtocolEsc);
            writeByte(SlipProtocolTransposedEnd);
            break;
        case SlipProtocolEsc:
            writeByte(SlipProtocolEsc);
            writeByte(SlipProtocolTransposedEsc);
            break;
        default:
            writeByte(dataByte);
            break;
        }
    }
}

void UartApiServer::writeByte(uint8_t dataByte)
{
    m_hardwareSerial->write(dataByte);
}

void UartApiServer::processData(uint8_t buffer[], uint8_t length)
{
    uint8_t command = buffer[0];
    uint8_t requestId = buffer[1];

    switch (command) {
    case CommandGetFirmwareVersion: {
        if (length != 2) {
            sendResponse(command, requestId, StatusInvalidPlayload);
            return;
        }
        
        uint8_t payloadSize = 3;
        uint8_t payload[payloadSize];
        payload[0] = FIRMWARE_MAJOR;
        payload[1] = FIRMWARE_MINOR;
        payload[2] = FIRMWARE_PATCH;
        sendResponse(command, requestId, StatusSuccess, payload, payloadSize);
        break;
    }
    case CommandConfigurePin: {
        if (length != 4) {
            sendResponse(command, requestId, StatusInvalidPlayload);
            return;
        }
        
        uint8_t pinId = buffer[2];
        GPIOController::PinMode pinMode = static_cast<GPIOController::PinMode>(buffer[3]);

        // Build response
        uint8_t payloadSize = 1;
        uint8_t payload[payloadSize];
        payload[0] = m_controller->configurePin(pinId, pinMode);;
        
        sendResponse(command, requestId, StatusSuccess, payload, payloadSize);
        break;
    }

    case CommandWriteDigitalPin: {
        if (length != 4) {
            sendResponse(command, requestId, StatusInvalidPlayload);
            return;
        }
        
        uint8_t pinId = buffer[2];
        bool power = buffer[3] != 0x00;

        // Build response
        uint8_t payloadSize = 1;
        uint8_t payload[payloadSize];
        payload[0] = m_controller->setGPIOPower(pinId, power);

        sendResponse(command, requestId, StatusSuccess, payload, payloadSize);
        break;
    }

    case CommandReadDigitalPin: {
        if (length != 3) {
            sendResponse(command, requestId, StatusInvalidPlayload);
            return;
        }
        
        uint8_t pinId = buffer[2];

        // Build response
        uint8_t payloadSize = 2;
        uint8_t payload[payloadSize];
        uint8_t pinValue = 0x00;
        GPIOController::GPIOError gpioError = GPIOController::GPIOErrorNoError;
        GPIOController::PinMode mode = m_controller->getPinMode(pinId);

        if (mode == GPIOController::PinModeUnconfigured) {
            gpioError = GPIOController::GPIOErrorUnconfigured;
        } else if (mode != GPIOController::PinModeGPIOInput && mode != GPIOController::PinModeGPIOOutput) {
            gpioError = GPIOController::GPIOErrorUnsupported;
        } else {
            pinValue = (m_controller->readDigitalValue(pinId) == 0 ? 0x00 : 0x01);
        }

        payload[0] = gpioError;
        payload[1] = pinValue;

        sendResponse(command, requestId, StatusSuccess, payload, payloadSize);
        break;
    }

    case CommandWriteAnalogPin: {
        if (length != 4) {
            sendResponse(command, requestId, StatusInvalidPlayload);
            return;
        }
        
        uint8_t pinId = buffer[2];
        uint8_t value = buffer[3];

        // Build response
        uint8_t payloadSize = 1;
        uint8_t payload[payloadSize];
        GPIOController::GPIOError gpioError = GPIOController::GPIOErrorNoError;
        GPIOController::PinMode mode = m_controller->getPinMode(pinId);
        if (mode == GPIOController::PinModeUnconfigured) {
            gpioError = GPIOController::GPIOErrorUnconfigured;
        } else if (mode != GPIOController::PinModeAnalogOutput) {
            gpioError = GPIOController::GPIOErrorConfigurationMismatch;
        } else {
            gpioError = m_controller->writeAnalogValue(pinId, value);
        }

        payload[0] = gpioError;
 
        sendResponse(command, requestId, StatusSuccess, payload, payloadSize);
        break;
    }

    case CommandReadAnalogPin: {
        if (length != 3) {
            sendResponse(command, requestId, StatusInvalidPlayload);
            return;
        }
        
        uint8_t pinId = buffer[2];

        // Build response
        uint8_t payloadSize = 3;
        uint8_t payload[payloadSize];
        uint16_t pinValue = 0x0000;
        GPIOController::GPIOError gpioError = GPIOController::GPIOErrorNoError;
        GPIOController::PinMode mode = m_controller->getPinMode(pinId);
        if (mode == GPIOController::PinModeUnconfigured) {
            gpioError = GPIOController::GPIOErrorUnconfigured;
        } else if (mode != GPIOController::PinModeAnalogInput && mode != GPIOController::PinModeAnalogOutput) {
            gpioError = GPIOController::GPIOErrorUnsupported;
        } else {
            pinValue = static_cast<uint16_t>(m_controller->readAnalogValue(pinId));
        }

        payload[0] = gpioError;
        payload[1] = (pinValue >> 8) & 0xFF;
        payload[2] = pinValue & 0xFF;

        sendResponse(command, requestId, StatusSuccess, payload, payloadSize);
        break;
    }

    case CommandWriteServoPin: {
        if (length != 4) {
            sendResponse(command, requestId, StatusInvalidPlayload);
            return;
        }
        
        uint8_t pinId = buffer[2];
        uint8_t value = buffer[3];

        // Build response
        uint8_t payloadSize = 1;
        uint8_t payload[payloadSize];
        GPIOController::GPIOError gpioError = GPIOController::GPIOErrorNoError;
        GPIOController::PinMode mode = m_controller->getPinMode(pinId);
        if (mode == GPIOController::PinModeUnconfigured) {
            gpioError = GPIOController::GPIOErrorUnconfigured;
        } else if (mode != GPIOController::PinModeServo) {
            gpioError = GPIOController::GPIOErrorUnsupported;
        } else {
            if (value > 180) {
                gpioError = GPIOController::GPIOErrorInvalidParameter;
            } else {
                gpioError = m_controller->writeServoValue(pinId, value);
            }
        }

        payload[0] = gpioError;
        sendResponse(command, requestId, StatusSuccess, payload, payloadSize);
        break;
    }
    default:
        sendResponse(command, requestId, StatusInvalidCommand);
        break;
    }
}

void UartApiServer::sendResponse(uint8_t command, uint8_t requestId, Status status, uint8_t payload[], size_t payloadLenght)
{
    streamByte(SlipProtocolEnd, true);
    streamByte(command);
    streamByte(requestId);
    streamByte(status);
    for (size_t i = 0; i < payloadLenght; i++) {
        streamByte(payload[i]);
    }
    streamByte(SlipProtocolEnd, true);

    m_hardwareSerial->flush();
}

void UartApiServer::sendNotification(UartApiServer::Notification notification, uint8_t payload[], size_t payloadLenght)
{
    streamByte(SlipProtocolEnd, true);
    streamByte(notification);
    streamByte(m_notificationId++);
    for (size_t i = 0; i < payloadLenght; i++) {
        streamByte(payload[i]);
    }
    streamByte(SlipProtocolEnd, true);
    m_hardwareSerial->flush();
}