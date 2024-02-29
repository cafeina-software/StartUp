#include "StartUpApp.h"

int main(int argc, char* argv[])
{
    StartUpApp *app = new StartUpApp();
    app->Run();
    delete app;
    return 0;
}

