#ifndef TRANSPORT_H
#define TRANSPORT_H

#include <ustd_map.h>
#include <ustd_functional.h>


class OwletClient;
class Transport;
typedef ustd::function<void(void* ctx, OwletClient *client)> OwletClientConnectHandler;
typedef ustd::function<void(void* ctx, OwletClient *client, const char *data, size_t len)> OwletClientDataHandler;
typedef ustd::function<void(void* ctx, OwletClient *client)> OwletClientDisconnectHandler;

class OwletClient
{

public:
    OwletClient();
    virtual ~OwletClient() = default;

    virtual String remoteName() const = 0;

    void onData(OwletClientDataHandler func, void *ctx);
    void onDisconnect(OwletClientDisconnectHandler func, void *ctx);

    virtual void sendData(const char* data, size_t len) = 0;

protected:
    void dataReceived(const char* data, size_t len);
    void disconnected();

private:
    friend class Transport;

    OwletClientDataHandler m_dataHandler;
    void *m_dataHandlerContext = nullptr;

    OwletClientDisconnectHandler m_disconnectHandler;
    void *m_disconnectHandlerContext = nullptr;

};

class Transport
{
public:
    Transport();
    virtual ~Transport() = default;

    void onConnect(OwletClientConnectHandler func, void *ctx);

    ustd::array<OwletClient*> clients() const;

protected:
    void clientConnected(OwletClient *client);
    void clientDisconnected(OwletClient *client);
    void clientData(OwletClient *client, char *data, size_t len);

private:
    OwletClientConnectHandler m_connectHandler;
    void *m_connectHandlerContext = nullptr;
};

#endif
