#include "../inc/InputHandler.h"

InputHandler::InputHandler(ParsedArgs args):
    arguments(args) {
    printf_debug("Input: Constructing...");
    if (args.proto == ProtocolType::TCP) {
        printf_debug("Input: Creating TCPClient and thread");
        tcpClient = make_unique<TCPClient>(args);
    } else {
        printf_debug("Input: Creating UDPClient and thread");
        udpClient = make_unique<UDPClient>(args);
    }

    thread receiveThread([this]() {
        while (true) {
            processIncomingMessage();
        }
    });
    this->receiveThread = move(receiveThread);
}

InputHandler::~InputHandler() {
    printf_debug("Input: Destructing...");
    if (receiveThread.joinable()) {
        receiveThread.join();
    }
}

void InputHandler::run() {
    string input;
    while (true) {
        // cout << "> ";
        getline(cin, input);

        if (input.empty()) {
            printf_debug("Input: User input was empty, skipping");
            continue;
        }

        if (input[0] == '/') {
            handleCommand(input);
        } else {
            printf_debug("Input: No / detected, processing as message");
            handleMessage(input);
        }
    }
}

void InputHandler::handleCommand(const string& command) {
    istringstream iss(command);
    string cmd;
    iss >> cmd;

    if (cmd == "/auth") {
        string username, secret, displayName;
        iss >> username >> secret >> displayName;
        printf_debug("Input: /auth command received with parameters: u=%s s=%s d=%s", username.c_str(), secret.c_str(), displayName.c_str());
        if (username.empty() || secret.empty() || displayName.empty()) {
            cout << "ERROR: Invalid /auth parameters.\n";
        } else {
            this->displayName = displayName;
            // TODO: write auth logic
            // Sends AUTH message with the data provided from the command to the server (and correctly handles the Reply message), locally sets the DisplayName value (same as the /rename command)

            vector<string> params;
            params.push_back(username);
            params.push_back(secret);
            params.push_back(this->displayName);
            tcpClient->sendMessage(MessageFactory::createMessage(MessageType::AUTH, params));
        }
    } else if (cmd == "/join") {
        string channel;
        iss >> channel;
        printf_debug("Input: /join command received with parameters: c=%s", channel.c_str());
        if (channel.empty()) {
            cout << "ERROR: Invalid /join parameters.\n";
        } else {
            cout << "Joining channel " << channel << "...\n";
            // TODO: write join logic
            // Sends JOIN message with channel name from the command to the server (and correctly handles the Reply message)

            vector<string> params;
            params.push_back(channel);
            params.push_back(this->displayName);
            tcpClient->sendMessage(MessageFactory::createMessage(MessageType::JOIN, params));
        }
    } else if (cmd == "/rename") {
        string displayName;
        iss >> displayName;
        printf_debug("Input: /rename command received with parameters: d=%s", displayName.c_str());
        if (displayName.empty()) {
            cout << "ERROR: Invalid /rename parameters.\n";
        } else {
            cout << "Renaming to " << displayName << "...\n";
            this->displayName = displayName;
        }
    } else if (cmd == "/help") {
        printf_debug("Input: /help command received");
        printHelp();
    } else {
        printf_debug("Input: Unknown command %s received, skipping", cmd.c_str());
        cout << "ERROR: Unknown command, use /help for avaliable commands.\n";
    }
}

void InputHandler::handleMessage(const string& message) {
    vector<string> params;
    params.push_back(this->displayName);
    params.push_back(message);
    tcpClient->sendMessage(MessageFactory::createMessage(MessageType::MSG, params));
}

void InputHandler::processIncomingMessage() {
    try {
        switch ((this->arguments.proto == ProtocolType::TCP ? tcpClient->receiveMessage() : udpClient->receiveMessage())->getType()) {
            case MessageType::AUTH:
                break;
            case MessageType::JOIN:
                break;
            case MessageType::MSG:
                break;
            case MessageType::REPLY:
                break;
            case MessageType::ERR:
                break;
            case MessageType::BYE:
                break;
            default:
                break;
        }
    } catch (const exception& e) {
        printf_debug("InputHandler: Error processing message: %s", e.what());
    }
}

void InputHandler::printHelp() {
    cout << "Available commands:\n"
        << "/auth <username> <secret> <displayName> - Authenticate user\n"
        << "/join <channel> - Join a channel\n"
        << "/rename <displayName> - Change display name\n"
        << "/help - Show this help message\n";
}
