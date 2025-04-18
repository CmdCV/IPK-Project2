#include "inc/ArgHandler.h"
#include "inc/InputHandler.h"

#include <iostream>
#include <csignal>

using namespace std;

static InputHandler* globalHandler = nullptr;

void signal_handler(int signal) {
    if (signal == SIGINT && globalHandler) {
        globalHandler->stop();
        cout << "\nProgram interrupted. Closing..." << endl << flush;
        exit(EXIT_SUCCESS);
    }
}

int main(int argc, char* argv[]) {
    InputHandler handler(ArgHandler::parse(argc, argv));
    globalHandler = &handler; // Nastavení globálního ukazatele
    signal(SIGINT, signal_handler);
    handler.run();

    return 0;
}
