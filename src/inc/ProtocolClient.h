#ifndef PROTOCOLCLIENT_H
#define PROTOCOLCLIENT_H

#include "debugPrint.h"
#include "Message.h"
#include <unistd.h>
#include <string>

using namespace std;

class ProtocolClient {
public:
    ProtocolClient(string host, uint16_t port);
    ~ProtocolClient();

    virtual void stop() = 0;
    virtual void sendMessage(unique_ptr<Message> message) = 0;
    virtual unique_ptr<Message> receiveMessage() = 0;
    // virtual void connect();
    // virtual void disconnect();

protected:
    string host;
    uint16_t port;
    int ip_socket = 0;

};


#endif //PROTOCOLCLIENT_H
