#ifndef INPUTHANDLER_H
#define INPUTHANDLER_H

#include "debugPrint.h"
#include "ArgHandler.h"
#include "TCPClient.h"
#include "UDPClient.h"
#include <iostream>
#include <string>
#include <sstream>
#include <chrono>
#include <sys/select.h>
#include <unistd.h>
#include <atomic>
#include <thread>

using namespace std;

class InputHandler
{

public:
    InputHandler(ParsedArgs args);
    ~InputHandler();
    void run();
    void stop();
private:
    std::atomic<bool> authenticated{false};
    std::atomic<bool> running{true};
    ParsedArgs arguments;
    string displayName;
    unique_ptr<TCPClient> tcpClient;
    unique_ptr<UDPClient> udpClient;
    thread receiveThread;

    void handleCommand(const string& command);
    void handleMessage(const string& message);
    void processIncomingMessage();
    static void printHelp();
};

#endif //INPUTHANDLER_H
