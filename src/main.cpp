#include "./inc/ArgHandler.h"

#include <iostream>


using namespace std;

int main(int argc, char* argv[]) {
    ParsedArgs args = ArgHandler::parse(argc, argv);



    return 0;
}
