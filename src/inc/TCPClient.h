#ifndef TCPCLIENT_H
#define TCPCLIENT_H

#include "ArgHandler.h"
#include "ProtocolClient.h"
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdexcept>
#include <iostream>
#include <string>
#include <cerrno>

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
