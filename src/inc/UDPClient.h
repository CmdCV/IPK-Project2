#ifndef UDPCLIENT_H
#define UDPCLIENT_H

#include "ArgHandler.h"
#include "ProtocolClient.h"

class UDPClient: ProtocolClient {
public:
    UDPClient(ParsedArgs args);
    ~UDPClient();

    void stop() override;
    void sendMessage(unique_ptr<Message> message) override;
    unique_ptr<Message> receiveMessage() override;
private:
    uint16_t timeout;
    uint8_t retries;
};
#endif //UDPCLIENT_H
