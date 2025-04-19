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
        try {
            while (running.load(std::memory_order_acquire)) {
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
    while (running.load(std::memory_order_acquire)) {
        // wait for stdin with timeout to allow exit without blocking
        fd_set readfds;
        FD_ZERO(&readfds);
        FD_SET(STDIN_FILENO, &readfds);
        struct timeval tv = {0, 100000}; // 100 ms timeout
        int ret = select(STDIN_FILENO + 1, &readfds, nullptr, nullptr, &tv);
        if (ret > 0 && FD_ISSET(STDIN_FILENO, &readfds)) {
            if (!std::getline(std::cin, input)) {
                if (authenticated.load(std::memory_order_acquire)) {
                    stop();
                }
                break;
            }
        } else {
            continue;
        }

        if (running) {
            if (input.empty()) {
                printf_debug("Input: User input was empty, skipping");
                continue;
            }

            if (input[0] == '/') {
                handleCommand(input);
            } else {
                if (!authenticated.load(std::memory_order_acquire)) {
                    cout << "ERROR: Not authenticated.\n" << flush;
                } else {
                    printf_debug("Input: No / detected, processing as message");
                    handleMessage(input);
                }
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
    } else if (!authenticated.load(std::memory_order_acquire)) {
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
            // No message (connection closed), stop processing
            return;
        }

        switch (msg->getType()) {
            case MessageType::REPLY: {
                // Only set authenticated on a successful reply
                if (!authenticated.load(std::memory_order_acquire)) {
                    auto replyMsg = dynamic_cast<ReplyMessage*>(msg.get());
                    if (replyMsg && replyMsg->isSuccess()) {
                        authenticated.store(true, std::memory_order_release);
                    }
                }
                break;
            }
            case MessageType::ERR:
            case MessageType::BYE:
                stop();
                break;
            default:
                break;
        }
    } catch (const exception& e) {
        if (running.load(std::memory_order_acquire)) {
            printf_debug("InputHandler: Error processing message: %s", e.what());
            cout << "ERROR: Invalid message.\n" << flush;

            vector<string> params;
            params.push_back(this->displayName);
            params.push_back("Invalid message");
            if (arguments.proto == ProtocolType::TCP) {
                tcpClient->sendMessage(MessageFactory::createMessage(MessageType::ERR, params));
            } else {
                udpClient->sendMessage(MessageFactory::createMessage(MessageType::ERR, params));
            }
            stop();
        }
    }
}

void InputHandler::stop() {
    printf_debug("InputHandler: Stopping...");
    running.store(false, std::memory_order_release);
    vector<string> params;
    params.push_back(this->displayName);
    if (this->arguments.proto == ProtocolType::TCP) {
        if (authenticated.load(std::memory_order_acquire)) {
            tcpClient->sendMessage(MessageFactory::createMessage(MessageType::BYE, params));
        }
        tcpClient->stop();
    } else {
        if (authenticated.load(std::memory_order_acquire)) {
            udpClient->sendMessage(MessageFactory::createMessage(MessageType::BYE, params));
        }
        udpClient->stop();
    }
}

void InputHandler::printHelp() {
    cout << "Available commands:\n"
        << "/auth <username> <secret> <displayName> - Authenticate user\n"
        << "/join <channelID> - Join a channel\n"
        << "/rename <displayName> - Change display name\n"
        << "/help - Show this help message\n"
        << flush;
}
