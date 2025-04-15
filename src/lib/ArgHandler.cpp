//
// Created by Aleš Urbánek on 14.04.2025.
//

#include "../inc/ArgHandler.h"
#include <cstring>
#include <iostream>

using namespace std;

ParsedArgs ArgHandler::parse(int argc, char* argv[]) {
    ParsedArgs args;
    bool valid = true;

    for (int i = 1; i < argc; ++i) {
        if (!strcmp(argv[i], "-h")) {
            printHelp();
            exit(0);
        } else if (!strcmp(argv[i], "-t") && i + 1 < argc) {
            args.proto = argv[++i];
        } else if (!strcmp(argv[i], "-s") && i + 1 < argc) {
            args.host = argv[++i];
        } else if (!strcmp(argv[i], "-p") && i + 1 < argc) {
            args.port = stoi(argv[++i]);
        } else if (!strcmp(argv[i], "-d") && i + 1 < argc) {
            args.timeout = stoi(argv[++i]);
        } else if (!strcmp(argv[i], "-r") && i + 1 < argc) {
            args.retries = stoi(argv[++i]);
        } else {
            valid = false;
        }
    }


    if (!valid || args.proto.empty() || args.host.empty()) {
	    cerr << "ERROR: Unknown or incomplete argument: " << argv[i] << endl;
	    printHelp();
	    exit(1);
    }



    return args;
}

void ArgHandler::printHelp() {
    cout <<
        "Usage: ./ipk25-chat -t tcp|udp -s server [-p port] [-d timeout] [-r retries]\n"
        "Options:\n"
        "  -t <tcp|udp>    Transport protocol used for connection (required)\n"
        "  -s <address>    Server IP or hostname (required)\n"
        "  -p <port>       Server port (default: 4567)\n"
        "  -d <timeout>    UDP confirmation timeout in milliseconds (default: 250)\n"
        "  -r <retries>    Maximum number of UDP retransmissions (default: 3)\n"
        "  -h              Prints this program help output and exits\n";
}
