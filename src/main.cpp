#include "inc/ArgHandler.h"
#include "inc/InputHandler.h"

#include <iostream>

using namespace std;

int main(int argc, char* argv[]) {
    InputHandler handler(ArgHandler::parse(argc, argv));
    handler.run();

    return 0;
}
