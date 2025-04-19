#include "../inc/UDPClient.h"
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/time.h>
#include <unistd.h>
#include <stdexcept>
#include <cstdio>
#include <cstring>
#include <atomic>

UDPClient::UDPClient(const ParsedArgs& args)
  : ProtocolClient(args.host, args.port),
    timeout(args.timeout),
    retries(args.retries),
    nextMsgId(1)
{
    printf_debug("UDPClient: Constructing...");
    ip_socket = socket(AF_INET, SOCK_DGRAM, 0);
    if (ip_socket < 0)
        throw runtime_error("ERROR: Unable to create UDP socket");

    // Initialize serverAddr from the command‑line host/port
    memset(&serverAddr, 0, sizeof(serverAddr));
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port   = htons(this->port);
    if (inet_pton(AF_INET, this->host.c_str(), &serverAddr.sin_addr) <= 0) {
        close(ip_socket);
        throw runtime_error("ERROR: Invalid UDP address");
    }

    // Apply receive timeout
    struct timeval tv{};
    tv.tv_sec  = timeout / 1000;
    tv.tv_usec = (timeout % 1000) * 1000;
    if (setsockopt(ip_socket, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv)) < 0)
        perror("Warning: could not set UDP receive timeout");
}

UDPClient::~UDPClient() {
    printf_debug("UDPClient: Destructing...");
    stop();
}

void UDPClient::stop() {
    printf_debug("UDPClient: Stopping...");
    if (ip_socket) {
        close(ip_socket);
        ip_socket = 0;
    }
}

void UDPClient::sendMessage(unique_ptr<Message> message) {
    waitingForConfirm.store(true, std::memory_order_release);
    uint16_t msgId = nextMsgId++;
    auto buf = message->serializeUDP(msgId);

    for (int attempt = 0; attempt <= retries; ++attempt) {
        // Send to whatever serverAddr currently holds
        ssize_t sent = sendto(ip_socket, buf.data(), buf.size(), 0,
                              reinterpret_cast<sockaddr*>(&serverAddr),
                              sizeof(serverAddr));
        if (sent < 0)
            throw runtime_error("ERROR: UDP send failed");

        // Await CONFIRM from server (possibly on new port)
        uint8_t respBuf[65536];
        sockaddr_in peer{};
        socklen_t addrLen = sizeof(peer);
        ssize_t n = recvfrom(ip_socket, respBuf, sizeof(respBuf), 0,
                             reinterpret_cast<sockaddr*>(&peer), &addrLen);
        if (n < 0) {
            printf_debug("UDPClient: No CONFIRM, retry %d", attempt+1);
            continue;
        }

        // Server may now be on a different port; update it
        serverAddr = peer;

        // Check for our CONFIRM
        if (respBuf[0] == 0) {
            uint16_t rid = (uint16_t(respBuf[1])<<8) | respBuf[2];
            if (rid == msgId) {
                printf_debug("UDPClient: Got CONFIRM for %u", msgId);
                waitingForConfirm.store(false, std::memory_order_release);
                return;
            }
        }
    }

    throw runtime_error("ERROR: No CONFIRM after retries");
}

unique_ptr<Message> UDPClient::receiveMessage() {
    if (waitingForConfirm.load(std::memory_order_acquire)) {
        return nullptr;
    }
    uint8_t buf[65536];
    sockaddr_in peer{};
    socklen_t addrLen = sizeof(peer);
    ssize_t n = recvfrom(ip_socket, buf, sizeof(buf), 0,
                         reinterpret_cast<sockaddr*>(&peer), &addrLen);
    if (n < 0) {
        return nullptr;
        // throw runtime_error("ERROR: UDP receive failed or timed out");
    }

    printf_debug("UDPClient: Received %zd bytes", n);
    serverAddr = peer;   // adopt any new server port

    uint8_t type = buf[0];
    uint16_t mid = (uint16_t(buf[1])<<8) | buf[2];

    // ACK every non‑CONFIRM packet
    if (type != 0) {
        ConfirmMessage ack;
        auto ackBuf = ack.serializeUDP(mid);
        sendto(ip_socket, ackBuf.data(), ackBuf.size(), 0,
               reinterpret_cast<sockaddr*>(&serverAddr), sizeof(serverAddr));
        printf_debug("UDPClient: Sent CONFIRM for incoming %u", mid);
    }

    // Drop duplicate messages
    if (type != 0 && !receivedMsgIds.insert(mid).second) {
        printf_debug("UDPClient: Duplicate %u, dropping", mid);
        return nullptr;
    }

    // Don’t expose CONFIRM frames up
    if (type == 0) return nullptr;

    // Parse and return all others
    return MessageFactory::parseUDP(buf, static_cast<size_t>(n));
}
