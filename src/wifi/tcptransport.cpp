#include "tcptransport.h"
#include "debugutils.h"

TcpTransport::TcpTransport()
{
    m_server = new AsyncServer(5555);

    m_server->onClient(&newClientHandler, this);

    m_server->begin();

}

TcpTransport::~TcpTransport()
{
    delete m_server;
}

void TcpTransport::newClientHandler(void *thiz, AsyncClient *client)
{
    TcpTransport *ref = static_cast<TcpTransport*>(thiz);
    DebugPrintln(String("New API client from ") + client->remoteIP().toString());
    ref->m_clients.push_back(client);

    TcpClient *tcpClient = new TcpClient(client);

    client->onData([=](void *, AsyncClient* client, void *data, size_t len){
        tcpClient->dataReceived((const char*)data, len);
    });

    client->onDisconnect([=](void *, AsyncClient* client){
        tcpClient->disconnected();
    });

//    client->onError(&handleError, NULL);
//    client->onTimeout(&handleTimeOut, NULL);

    ref->clientConnected(tcpClient);
}

TcpClient::TcpClient(AsyncClient *asyncClient):
    m_asyncClient(asyncClient)
{

}

String TcpClient::remoteName() const
{
    return m_asyncClient->remoteIP().toString();
}

void TcpClient::sendData(const char *data, size_t len)
{
    m_asyncClient->add(data, len);
    m_asyncClient->send();
}
