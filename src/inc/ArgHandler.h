#ifndef ARGHANDLER_H
#define ARGHANDLER_H

#include "debugPrint.h"

#include <string>
#include <cstdint>
#include <cstring>
#include <iostream>
#include <netdb.h>
#include <set>
#include <vector>
#include <arpa/inet.h>



using namespace std;

struct ParsedArgs {
    string proto;             // -t
    string host;              // -s
    uint16_t port = 4567;     // -p
    uint16_t timeout = 250;   // -d
    uint8_t retries = 3;      // -h
};

class ArgHandler {
public:
    static ParsedArgs parse(int argc, char* argv[]);
    static void printHelp();
private:

    /**
     * @brief Resolves the address of the target (domain name or IP address).
     * @param host The target address to resolve.
     *
     * This function uses the `getaddrinfo()` function to resolve the given
     * target (either a domain name or IP address) into a list of addresses.
     *
     * @throws runtime_error If `getaddrinfo()` fails.
     */
    string static resolve_address(string host);
};

#endif //ARGHANDLER_H
