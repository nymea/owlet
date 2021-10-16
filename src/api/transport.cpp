#include "transport.h"


OwletClient::OwletClient()
{

}

void OwletClient::onData(OwletClientDataHandler func, void *ctx)
{
    m_dataHandler = func;
    m_dataHandlerContext = ctx;
}

void OwletClient::onDisconnect(OwletClientDisconnectHandler func, void *ctx)
{
    m_disconnectHandler = func;
    m_disconnectHandlerContext = ctx;
}

void OwletClient::dataReceived(const char *data, size_t len)
{
    m_dataHandler(m_dataHandlerContext, this, data, len);
}

void OwletClient::disconnected()
{
    m_disconnectHandler(m_disconnectHandlerContext, this);
}

Transport::Transport()
{

}

void Transport::onConnect(OwletClientConnectHandler func, void *ctx)
{
    m_connectHandler = func;
    m_connectHandlerContext = ctx;
}

void Transport::clientConnected(OwletClient *client)
{
    m_connectHandler(m_connectHandlerContext, client);
}

void Transport::clientDisconnected(OwletClient *client)
{
    client->m_disconnectHandler(client->m_disconnectHandlerContext, client);
}
