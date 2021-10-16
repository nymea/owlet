#ifndef APISERVER_H
#define APISERVER_H

#include "apihandler.h"
#include "transport.h"

class APIServer
{
public:
    APIServer();
    ~APIServer();

    void registerTransport(Transport *transport);
    void registerHandler(APIHandler *handler);

private:
    static void dataHandler(void *ctx, OwletClient *client, const char *data, size_t len);

    void sendErrorReply(OwletClient *client, int commandId, const String &error);
    void sendReply(OwletClient *client, int commandId, const JSONVar &params);

private:
    static void onNotification(APIHandler *hander, const String &name, const JSONVar &params);

private:
    static APIServer *s_instance;

    ustd::array<OwletClient*> m_clients;
    ustd::array<APIHandler*> m_handlers;
};

#endif
