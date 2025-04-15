#include "../inc/UDPClient.h"

UDPClient::UDPClient(ParsedArgs args):ProtocolClient(args.host, args.port), timeout(args.timeout), retries(args.retries) {

}

UDPClient::~UDPClient() {
    printf_debug("UDPClient: Destructing...");
}

void UDPClient::sendMessage(unique_ptr<Message> message) {

}

unique_ptr<Message> UDPClient::receiveMessage() {
    return nullptr;
}
