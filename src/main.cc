#pragma comment( linker, "/subsystem:windows /entry:mainCRTStartup")

#include <iostream>
#include "log.h"
#include "application.h"

int main(int argc, char **argv)
{
    initLogger(INFO);
    Application app("VideoPlayer");
    app.run();
    app.wait();
    return 0;
}