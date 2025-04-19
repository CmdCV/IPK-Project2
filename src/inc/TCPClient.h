#ifndef TCPCLIENT_H
#define TCPCLIENT_H

#include "ArgHandler.h"
#include "ProtocolClient.h"

class TCPClient : public ProtocolClient
{
public:
    TCPClient(const ParsedArgs& args);
    ~TCPClient();

    void stop() override;
    void sendMessage(unique_ptr<Message> message) override;
    unique_ptr<Message> receiveMessage() override;
private:
    std::string recvBuffer;
};
#endif //TCPCLIENT_H
