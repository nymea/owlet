#include "apiserver.h"

#ifdef ESP32
#include <HardwareSerial.h>
#else
#include <SoftwareSerial.h>
#endif

#include <assert.h>

APIServer *APIServer::s_instance = nullptr;

APIServer::APIServer()
{
    assert(s_instance == nullptr);
    s_instance = this;
}

APIServer::~APIServer()
{
}

void APIServer::registerTransport(Transport *transport)
{
    transport->onConnect([=](void *thiz, OwletClient *client){
        Serial.println(String("New client connected: ") + client->remoteName());
        m_clients.add(client);

        client->onData(&dataHandler, this);

        client->onDisconnect([=](void *thiz, OwletClient *client){
            Serial.println(String("Client disconnected: ") + client->remoteName());
            for (unsigned int i = 0; i < m_clients.length(); i++) {
                if (m_clients[i] == client) {
                    m_clients.erase(i);
                    Serial.println(String("API client ") + client->remoteName() + " removed.");
                    delete client;
                }
            }
        }, this);
    }, this);

}

void APIServer::registerHandler(APIHandler *handler)
{
    handler->onNotification((NotificationFunc)&APIServer::onNotification);
//    handler->onNotification([=](APIHandler *hander, const String &name, const JSONVar &params){
//        for (auto it = m_clients.begin(); it != m_clients.end(); ++it) {
//            AsyncClient *client = *it;
//            JSONVar message;
//            message["notification"] = name;
//            message["params"] = params;
//            String data = JSON.stringify(message);
//            data += "\n";
//            client->add(data.c_str(), data.length());
//            client->send();
//        }
//    });
//    Serial.println("registering api handler");
    Serial.println(handler->nameSpace());
    m_handlers.add(handler);
}

void APIServer::dataHandler(void *ctx, OwletClient *client, const char* data, size_t len)
{
    APIServer *ref = static_cast<APIServer*>(ctx);

    char buf[len +1];
    memset(buf, 0, len+1);
    memcpy(buf, data, len);
    JSONVar json = JSON.parse(buf);

//    Serial.print(String("Data received from client ") + client->remoteName() + ": " + buf);
    if (JSON.typeof(json) == "undefined") {
//        Serial.println(String("Invalid data from client ") + client->remoteName());
        ref->sendErrorReply(client, -1, "JSON parse error.");
        return;
    }

    if (!json.hasOwnProperty("id")) {
//        Serial.println(String("Invalid method call. Missing id field. Client: ") + client->remoteName());
        ref->sendErrorReply(client, -1, "Missing id field.");
        return;
    }

    int commandId = json["id"];

    String method((const char*)json["method"]);
    int dotIndex = method.indexOf(".");
    if (dotIndex <= 0 || dotIndex >= method.length() - 1) {
        ref->sendErrorReply(client, commandId, "Invalid method");
        return;
    }
    String nameSpace = method.substring(0, dotIndex);
    method = method.substring(dotIndex + 1, method.length());

//    Serial.print("Method call ");
//    Serial.print(nameSpace);
//    Serial.print(".");
//    Serial.println(method);

    for (auto it = ref->m_handlers.begin(); it != ref->m_handlers.end(); ++it) {
        APIHandler *handler = (*it);
        if (handler->nameSpace() == nameSpace) {
//            Serial.println("Found handler");

            for (unsigned int i = 0; i < handler->methods().length(); i++) {
                APIMethod m = handler->methods()[i];
//                Serial.println(String("Testing method ") + m.name);
                if (m.name == method) {
//                    Serial.println("Found method!");
                    JSONVar ret = m.func(handler, json["params"]);
                    ref->sendReply(client, commandId, ret);
                    return;
                }
//                Serial.println("Nope... not the one");
            }
//            Serial.println("Method not found");
            break;
        }
    }

    ref->sendErrorReply(client, commandId, "Invalid method");
}

void APIServer::sendErrorReply(OwletClient *client, int commandId, const String &error)
{
    JSONVar reply;
    reply["id"] = commandId;
    reply["success"] = "false";
    reply["error"] = error;
    String data = JSON.stringify(reply);
    data += "\n";
    client->sendData(data.c_str(), data.length());
}

void APIServer::sendReply(OwletClient *client, int commandId, const JSONVar &params)
{
    JSONVar reply;
    reply["id"] = commandId;
    reply["success"] = "true";
    reply["params"] = params;
    String data = JSON.stringify(reply);
    data += "\n";
    client->sendData(data.c_str(), data.length());
}

void APIServer::onNotification(APIHandler *hander, const String &name, const JSONVar &params)
{
    Serial.println("notification");
    for (auto it = s_instance->m_clients.begin(); it != s_instance->m_clients.end(); ++it) {
        OwletClient *client = *it;
        Serial.println(String("Notifying client: ") + client->remoteName());
        JSONVar message;
        message["notification"] = name;
        message["params"] = params;
        String data = JSON.stringify(message);
        data += "\n";
        client->sendData(data.c_str(), data.length());
    }
}
