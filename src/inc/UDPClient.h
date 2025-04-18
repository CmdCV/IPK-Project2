#ifndef UDPCLIENT_H
#define UDPCLIENT_H

#include "ArgHandler.h"
#include "ProtocolClient.h"
#include <set>
#include <arpa/inet.h>

class UDPClient: public ProtocolClient {
public:
    UDPClient(const ParsedArgs& args);
    ~UDPClient();

    void stop() override;
    void sendMessage(unique_ptr<Message> message) override;
    unique_ptr<Message> receiveMessage() override;
private:
    uint16_t timeout;
    uint8_t retries;
    uint16_t nextMsgId = 1;  // next message ID for UDP reliability
    struct sockaddr_in serverAddr;           // Remote server address (dynamic port)
    set<uint16_t> receivedMsgIds;       // Track and dedupe incoming message IDs
};
#endif //UDPCLIENT_H
