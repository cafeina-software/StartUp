#include "StartUpApp.h"

int main(int argc, char* argv[])
{
    bool enable_experimental = false;

    if(argc == 2 && strcmp(argv[1], "--experimental") == 0)
        enable_experimental = true;

    StartUpApp *app = new StartUpApp(enable_experimental);
    app->Run();
    delete app;
    return 0;
}

