#include "../inc/UDPClient.h"

UDPClient::UDPClient(ParsedArgs args):ProtocolClient(args.host, args.port), timeout(args.timeout), retries(args.retries) {

}

UDPClient::~UDPClient() {
    printf_debug("UDPClient: Destructing...");
    stop();
}

void UDPClient::stop() {
    printf_debug("TCPClient: Stopping...");
    if (this->ip_socket != 0) {
        close(this->ip_socket);
        this->ip_socket = 0;
    }
}

void UDPClient::sendMessage(unique_ptr<Message> message) {

}

unique_ptr<Message> UDPClient::receiveMessage() {
    return nullptr;
}
