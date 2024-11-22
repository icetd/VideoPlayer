//#pragma comment( linker, "/subsystem:windows /entry:mainCRTStartup")

#include <iostream>
#include "log.h"
#include "Application.h"

#define VLD 0

#if VLD
#include <vld.h>
#endif


int main(int argc, char **argv)
{
    initLogger(INFO);
    Application app("VideoPlayer", 1160, 720);
    app.run();
    app.wait();
    return 0;
}