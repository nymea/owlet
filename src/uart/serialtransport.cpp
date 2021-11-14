#include "serialtransport.h"
#include "debugutils.h"

SerialClient::SerialClient(HardwareSerial *serial) :
    m_hardwareSerial(serial)
{
    m_hardwareSerial->println("Setup client");
}

SerialClient::~SerialClient()
{
    m_hardwareSerial->end();
}

String SerialClient::remoteName() const 
{
    return "Hardware Serial";
}

void SerialClient::sendData(const char *data, size_t len)
{
    // Stream the data slip encoded
    streamByte(SlipProtocolEnd, true);
    for (size_t i = 0; i < len; i++) {
        streamByte(data[i]);
    }
    streamByte(SlipProtocolEnd, true);

    m_hardwareSerial->flush();
}

void SerialClient::init()
{

}

void SerialClient::loop()
{
    while (Serial.available()) {
        uint8_t receivedByte = Serial.read();
        processReceivedByte(receivedByte);
    }
    //Serial.println("Loop!!");
}

void SerialClient::processReceivedByte(uint8_t receivedByte)
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
            dataReceived(m_buffer, m_bufferIndex);
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

void SerialClient::streamByte(uint8_t dataByte, boolean specialCharacter)
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

void SerialClient::writeByte(uint8_t dataByte)
{
    m_hardwareSerial->write(dataByte);
}

SerialTransport::SerialTransport(HardwareSerial &serial) :
    m_hardwareSerial(&serial)
{
    DebugPrintln("Setup transport");
}

SerialTransport::~SerialTransport()
{

}

void SerialTransport::registerSerialClient()
{
    // For now, only one serial makes sense...could ba lost tough
    if (!m_client) {
        DebugPrintln("Register client");
        m_client = new SerialClient(m_hardwareSerial);
        clientConnected(m_client);
    }

    DebugPrintln("Client registered");
}

void SerialTransport::loop()
{
    if (m_client) {
        m_client->loop();
    }
}