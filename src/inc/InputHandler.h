#ifndef INPUTHANDLER_H
#define INPUTHANDLER_H

#include "debugPrint.h"
#include "ArgHandler.h"

using namespace std;

class InputHandler
{

public:
    InputHandler(ParsedArgs args);
    void run();
private:
    ParsedArgs arguments;
    string displayName;
    void handleCommand(const string& command);
    void handleMessage(const string& message);
    static void printHelp();
};

#endif //INPUTHANDLER_H
