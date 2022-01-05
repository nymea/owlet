#ifndef UARTAPISERVER_H
#define UARTAPISERVER_H

#include <Arduino.h>

#include "../gpiocontroller.h"


/*  
    SLIP transfere: https://tools.ietf.org/html/rfc1055

    Endiness: Big endian

    Request package format:
    --------------------------
        uint8   : command (< 0xF0)
        uint8   : requestId
        uint8[] : payload (dynamic size, max: 253)


    Response format:
    --------------------------
        uint8   : command (< 0xF0)
        uint8   : requestId (same as request)
        uint8   : status
        uint8[] : payload (optional, max: 253)


    Notification package format:
    --------------------------
        uint8   : command (>= 0xF0)
        uint8   : notificationId
        uint8[] : payload (dynamic size, max: 253)


    Commands:
    --------------------------
        
        0x00: CommandGetFirmwareVersion
            Request:
                payload: empty
            Response:
                payload:
                    uint8 : major version
                    uint8 : minor version
                    uint8 : patch version

        0x01: CommandConfigurePin
            Request:
                payload:
                    uint8 : pin number
                    uint8 : pin mode
            Response:
                payload:
                    uint8 : GPIOError

        0x02: CommandWriteDigitalPin
            Request:
                payload:
                    uint8 : pin number
                    uint8 : power (0x00 OFF, 0x01 ON)
            Response:
                payload:
                    uint8 : GPIOError

        0x03: CommandReadDigitalPin
            Request:
                payload:
                    uint8 : pin number
            Response:
                payload:
                    uint8 : GPIOError
                    uint8 : power (0x00 OFF, 0x01 ON)
            
        0x04: CommandWriteAnalogPin
            Request:
                payload:
                    uint8 : pin number
                    uint8 : PWM duty cycle
            Response:
                payload:
                    uint8 : GPIOError
            

        0x05: CommandReadAnalogPin
            Request:
                payload:
                    uint8 : pin number
            Response:
                payload:
                    uint8  : GPIOError
                    uint16 : analog value

        0x06: CommandWriteServoPin
            Request:
                payload:
                    uint8 : pin number
                    uint8 : angle [0, 180]
            Response:
                payload:
                    uint8  : GPIOError
                        
    Notifications:
    --------------------------
        0xF0: Status ready notification 
            payload: empty

        0xF1 : InputValueChanged

        0xFF: Debug message notification 
            payload: debug message characters
                

*/

class UartApiServer 
{
public:
    enum SlipProtocol {
        SlipProtocolEnd = 0xC0,
        SlipProtocolEsc = 0xDB,
        SlipProtocolTransposedEnd = 0xDC,
        SlipProtocolTransposedEsc = 0xDD
    };

    enum Command {
        CommandGetFirmwareVersion = 0x00,
        CommandConfigurePin = 0x01,
        CommandWriteDigitalPin = 0x02,
        CommandReadDigitalPin = 0x03,
        CommandWriteAnalogPin = 0x04,
        CommandReadAnalogPin = 0x05,
        CommandWriteServoPin = 0x06
    };  

    enum Notification {
        NotificationReady = 0xf0,
        NotificationGpioPinChanged = 0xf1,
        NotificationDebugMessage = 0xff
    };

    enum Status {
        StatusSuccess = 0x00,
        StatusInvalidProtocol = 0x01,
        StatusInvalidCommand = 0x02,
        StatusInvalidPlayload = 0x03,
        StatusUnknownError = 0xff
    };

    UartApiServer(HardwareSerial &serial, GPIOController *controller);
    ~UartApiServer();

    void init();
    void loop();

    void sendData(const char *data, size_t len);

private:
    // UART read
    HardwareSerial *m_hardwareSerial = nullptr;
    GPIOController *m_controller = nullptr;
    
    uint8_t m_buffer[255];
    uint8_t m_bufferIndex = 0;
    boolean m_protocolEscaping = false;
    uint8_t m_notificationId = 0;

protected:
    virtual void processReceivedByte(uint8_t receivedByte);
    virtual void streamByte(uint8_t dataByte, boolean specialCharacter = false);
    virtual void writeByte(uint8_t dataByte);
    virtual void processData(uint8_t buffer[], uint8_t length);
    virtual void sendResponse(uint8_t command, uint8_t requestId, Status status, uint8_t payload[] = nullptr, size_t payloadLenght = 0);
    virtual void sendNotification(UartApiServer::Notification notification, uint8_t payload[] = nullptr, size_t payloadLenght = 0);

};

#endif // UARTAPISERVER_H