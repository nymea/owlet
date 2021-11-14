#ifndef SERIALTRANSPORT_H
#define SERIALTRANSPORT_H

#include <Arduino.h>

#include "api/transport.h"

class SerialTransport;

class SerialClient: public OwletClient
{

public:
    SerialClient(HardwareSerial *serial);
    ~SerialClient();

    void init();
    void loop();

    String remoteName() const override;
    void sendData(const char *data, size_t len) override;

private:
    enum SlipProtocol {
        SlipProtocolEnd = 0xC0,
        SlipProtocolEsc = 0xDB,
        SlipProtocolTransposedEnd = 0xDC,
        SlipProtocolTransposedEsc = 0xDD
    };

    // UART read
    HardwareSerial *m_hardwareSerial;
    char m_buffer[255];
    uint8_t m_bufferIndex = 0;
    boolean m_protocolEscaping = false;

protected:
    virtual void processReceivedByte(uint8_t receivedByte);
    virtual void streamByte(uint8_t dataByte, boolean specialCharacter = false);
    virtual void writeByte(uint8_t dataByte);

};


class SerialTransport: public Transport
{
public:
    SerialTransport(HardwareSerial &serial);
    ~SerialTransport();

    void registerSerialClient();
    void loop();

private:
    HardwareSerial *m_hardwareSerial = nullptr;
    SerialClient *m_client = nullptr;

};


#endif // SERIALTRANSPORT_H