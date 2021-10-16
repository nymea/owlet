#ifndef TCPTRANSPORT_H
#define TCPTRANSPORT_H

#include "api/transport.h"

#ifdef ESP32
#include <AsyncTCP.h>
#elif defined ESP8266
#include <ESPAsyncTCP.h>
#endif

class TcpTransport;

class TcpClient: public OwletClient
{
    TcpClient(AsyncClient *asyncClient);

    String remoteName() const override;
    void sendData(const char *data, size_t len) override;

private:
    friend class TcpTransport;
    AsyncClient *m_asyncClient = nullptr;

};

class TcpTransport: public Transport
{
public:
    TcpTransport();
    ~TcpTransport();

    static void newClientHandler(void* thiz, AsyncClient* client);

private:
    AsyncServer *m_server = nullptr;
    std::vector<AsyncClient*> m_clients;

};

#endif
