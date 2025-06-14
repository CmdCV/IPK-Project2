#include "../inc/TCPClient.h"

TCPClient::TCPClient(const ParsedArgs& args) :
    ProtocolClient(args.host, args.port) {
    printf_debug("TCPClient: Constructing...");

    this->ip_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (this->ip_socket < 0) {
        throw runtime_error("ERROR: Unable to create socket");
    }
    printf_debug("TCPClient: Socket created...");

    struct sockaddr_in serverAddr{};
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(this->port);

    printf_debug("TCPClient: serverAddr created...");
    if (inet_pton(AF_INET, this->host.c_str(), &serverAddr.sin_addr) <= 0) {
        close(this->ip_socket);
        throw runtime_error("ERROR: Invalid address or address not supported");
    }
    printf_debug("TCPClient: IP binded...");

    if (connect(this->ip_socket, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) < 0) {
        close(this->ip_socket);
        throw runtime_error("ERROR: Connection failed");
    }
    printf_debug("TCPClient: Connected to %s:%d...", this->host.c_str(), this->port);
}

TCPClient::~TCPClient() {
    printf_debug("TCPClient: Destructing...");
    stop();
}

void TCPClient::stop() {
    printf_debug("TCPClient: Stopping...");
    if (this->ip_socket != 0) {
        close(this->ip_socket);
        this->ip_socket = 0;
    }
}

void TCPClient::sendMessage(unique_ptr<Message> message) {
    string data = message->serialize();
    printf_debug("Message: Sending message: m=%s", data.c_str());
    if (send(this->ip_socket, data.c_str(), data.size(), 0) < 0) {
        throw runtime_error("ERROR: Failed to send message");
    }
}

unique_ptr<Message> TCPClient::receiveMessage() {
    printf_debug("TCPClient: Waiting for messages...");
    string msgStr;
    char buffer[70000];
    while (true) {
        ssize_t bytesRead = recv(this->ip_socket, buffer, sizeof(buffer) - 1, 0);
        if (bytesRead < 0) {
            if (errno == EINTR || errno == EBADF) {
                // Interrupted or socket closed: treat as shutdown
                return nullptr;
            }
            throw runtime_error("ERROR: Failed to receive message");
        }
        if (bytesRead == 0) {
            // Server closed connection gracefully
            return nullptr;
        }
        msgStr.append(buffer, static_cast<size_t>(bytesRead));
        printf_debug("TCPClient: Received chunk: %.*s", static_cast<int>(bytesRead), buffer);
        // Continue until message ends with CRLF
        if (msgStr.size() >= 2 && msgStr.compare(msgStr.size() - 2, 2, "\r\n") == 0) {
            break;
        }
    }
    printf_debug("TCPClient: Complete message: %s", msgStr.c_str());
    return MessageFactory::parseMessage(msgStr);
}
