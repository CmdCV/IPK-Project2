#include "../inc/ArgHandler.h"

ParsedArgs ArgHandler::parse(int argc, char* argv[]) {
    ParsedArgs args;
    bool valid = false;

    for (int i = 1; i < argc; ++i) {
        if (!strcmp(argv[i], "-h")) {
            printf_debug("CLI arguments: Help requested");
            printHelp();
            exit(0);
        } else if (!strcmp(argv[i], "-t") && i + 1 < argc) {
            i++;
            if(strcmp(argv[i], "tcp") == 0) {
            	args.proto = ProtocolType::TCP;
                valid = true;
                printf_debug("CLI arguments: Protocol set to TCP");
            } else if(strcmp(argv[i], "udp") == 0) {
                args.proto = ProtocolType::UDP;
                valid = true;
                printf_debug("CLI arguments: Protocol set to UDP");
            } else {
                cout << "ERROR: CLI arguments: Unknown protocol "<< argv[i] << "\n" << flush;
                printHelp();
                exit(1);
            }
        } else if (!strcmp(argv[i], "-s") && i + 1 < argc) {
            args.host = resolve_address(argv[++i]);
            printf_debug("CLI arguments: Host set to %s resolved from %s", args.host.c_str(), argv[i]);
        } else if (!strcmp(argv[i], "-p") && i + 1 < argc) {
            args.port = stoi(argv[++i]);
            printf_debug("CLI arguments: Port set to %d", args.port);
        } else if (!strcmp(argv[i], "-d") && i + 1 < argc) {
            args.timeout = stoi(argv[++i]);
            printf_debug("CLI arguments: Timeout set to %d", args.timeout);
        } else if (!strcmp(argv[i], "-r") && i + 1 < argc) {
            args.retries = stoi(argv[++i]);
            printf_debug("CLI arguments: Retries set to %d", args.retries);
        } else {
            cout << "ERROR: CLI arguments: Unknown argument "<< argv[i] << "\n" << flush;
            printHelp();
            exit(1);
        }
    }

    if (!valid || args.host.empty()) {
	    cout << "ERROR: CLI arguments: Host or Protocol not specified" << "\n" << flush;
	    printHelp();
	    exit(1);
    }
    printf_debug("CLI arguments: returning p=%s h=%s p=%d t=%d r=%d", args.proto == ProtocolType::TCP ? "tcp" : "udp", args.host.c_str(), args.port, args.timeout, args.retries);
    return args;
}

// Resolve the target address (either domain or IP)
// https://stackoverflow.com/questions/4780021/c-retrieve-the-ip-from-a-name-from-the-dns-linux/4780047
string ArgHandler::resolve_address(string host) {
    addrinfo hints = {};
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;

    addrinfo* address_list = nullptr; // Deklarace address_list
    int status = getaddrinfo(host.c_str(), nullptr, &hints, &address_list);
    if (status != 0) {
        cout << "Error: Couldn't resolve address: " << host << endl;
        exit(EXIT_FAILURE);
    }

    char ip_str[INET_ADDRSTRLEN];
    for (addrinfo* addr = address_list; addr != nullptr; addr = addr->ai_next) {
        sockaddr_in* ipv4 = reinterpret_cast<sockaddr_in*>(addr->ai_addr);
        if (inet_ntop(AF_INET, &(ipv4->sin_addr), ip_str, sizeof(ip_str))) {
            freeaddrinfo(address_list);
            return string(ip_str);
        }
    }

    freeaddrinfo(address_list);
    cout << "Error: No valid IPv4 address found for: " << host << endl;
    exit(EXIT_FAILURE);
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
        "  -h              Prints this program help output and exits\n"
         << flush;
}
