#ifndef ARGHANDLER_H
#define ARGHANDLER_H

#include <string>
#include <optional>
#include <cstdint>

using namespace std;

struct ParsedArgs {
    string proto;// -t
    string host;// -s
    uint16_t port = 4567;// -p
    uint16_t timeout = 250; // -d
    uint8_t retries = 3; // -h
};

class ArgHandler {
public:
    static ParsedArgs parse(int argc, char* argv[]);
    static void printHelp();
};

#endif //ARGHANDLER_H
