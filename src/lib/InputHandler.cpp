#include "../inc/InputHandler.h"
#include <chrono>

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
        try {
            while (running) {
                processIncomingMessage();
            }
        } catch (const exception& e) {
            printf_debug("InputHandler: Receiver thread fatal error: %s", e.what());
        }

        if (this->arguments.proto == ProtocolType::TCP) {
            tcpClient->stop();
        } else {
            udpClient->stop();
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
    while (running) {
        // cout << "> ";
        getline(cin, input);

        if (input.empty()) {
            printf_debug("Input: User input was empty, skipping");
            continue;
        }

        if (input[0] == '/') {
            handleCommand(input);
        } else {
            if (!authenticated) {
                cout << "ERROR: Not authenticated.\n" << flush;
            } else {
            printf_debug("Input: No / detected, processing as message");
                handleMessage(input);
            }
        }
    }
}

void InputHandler::handleCommand(const string& command) {
    istringstream iss(command);
    string cmd;
    iss >> cmd;

    if (cmd == "/help") {
        printf_debug("Input: /help command received");
        printHelp();
    } else if (!authenticated) {
        if (cmd == "/auth") {
            string username, secret, displayName;
            iss >> username >> secret >> displayName;
            printf_debug("Input: /auth command received with parameters: u=%s s=%s d=%s", username.c_str(), secret.c_str(), displayName.c_str());
            if (username.empty() || secret.empty() || displayName.empty()) {
                cout << "ERROR: Invalid /auth parameters.\n" << flush;
            } else {
                this->displayName = displayName;
                vector<string> params;
                params.push_back(username);
                params.push_back(this->displayName);
                params.push_back(secret);
                if (arguments.proto == ProtocolType::TCP) {
                    tcpClient->sendMessage(MessageFactory::createMessage(MessageType::AUTH, params));
                } else {
                    udpClient->sendMessage(MessageFactory::createMessage(MessageType::AUTH, params));
                }
            }
        } else {
            cout << "ERROR: You need to authenticate first...\n" << flush;
        }
    } else {
        if (cmd == "/join") {
            string channel;
            iss >> channel;
            printf_debug("Input: /join command received with parameters: c=%s", channel.c_str());
            if (channel.empty()) {
                cout << "ERROR: Invalid /join parameters.\n" << flush;
            } else {
                vector<string> params;
                params.push_back(channel);
                params.push_back(this->displayName);
                if (arguments.proto == ProtocolType::TCP) {
                    tcpClient->sendMessage(MessageFactory::createMessage(MessageType::JOIN, params));
                } else {
                    udpClient->sendMessage(MessageFactory::createMessage(MessageType::JOIN, params));
                }
            }
        } else if (cmd == "/rename") {
            string displayName;
            iss >> displayName;
            printf_debug("Input: /rename command received with parameters: d=%s", displayName.c_str());
            if (displayName.empty()) {
                cout << "ERROR: Invalid /rename parameters.\n" << flush;
            } else {
                this->displayName = displayName;
            }
        } else {
            if (cmd == "/auth") {
                cout << "ERROR: You are already authenticated...\n" << flush;
            } else {
                cout << "ERROR: Unknown command '" << cmd << "', use /help for available commands.\n" << flush;
            }
        }
    }
}

void InputHandler::handleMessage(const string& message) {
    vector<string> params;
    params.push_back(this->displayName);
    params.push_back(message);
    if (this->arguments.proto == ProtocolType::TCP) {
        tcpClient->sendMessage(MessageFactory::createMessage(MessageType::MSG, params));
    } else {
        udpClient->sendMessage(MessageFactory::createMessage(MessageType::MSG, params));
    }
}

void InputHandler::processIncomingMessage() {
    try {
        unique_ptr<Message> msg = arguments.proto == ProtocolType::TCP ? tcpClient->receiveMessage() : udpClient->receiveMessage();

        if (!msg) {
            return;
        }

        switch (msg->getType()) {
            case MessageType::REPLY:
                if (!authenticated) authenticated = true;
                break;
            case MessageType::BYE:
                running = false;
                break;
            default:
                break;
        }
    } catch (const exception& e) {
        printf_debug("InputHandler: Error processing message: %s", e.what());
    }
}

void InputHandler::stop() {
    printf_debug("InputHandler: Stopping...");
    running = false;
    vector<string> params;
    params.push_back(this->displayName);
    if (this->arguments.proto == ProtocolType::TCP) {
        if (authenticated) {
            tcpClient->sendMessage(MessageFactory::createMessage(MessageType::BYE, params));
        }
        tcpClient->stop();
    } else {
        if (authenticated) {
            udpClient->sendMessage(MessageFactory::createMessage(MessageType::BYE, params));
        }
        udpClient->stop();
    }
}

void InputHandler::printHelp() {
    cout << "Available commands:\n"
        << "/auth <username> <secret> <displayName> - Authenticate user\n"
        << "/join <channel> - Join a channel\n"
        << "/rename <displayName> - Change display name\n"
        << "/help - Show this help message\n"
        << flush;
}
