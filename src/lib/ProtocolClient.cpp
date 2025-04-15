#include "../inc/ProtocolClient.h"

ProtocolClient::ProtocolClient(string host, uint16_t port): host(host), port(port) {}

ProtocolClient::~ProtocolClient() {
    printf_debug("ProtocolClient: Destructing...");
    if (ip_socket != 0) {
        close(ip_socket);
    }
}
