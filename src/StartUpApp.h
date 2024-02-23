#ifndef __STARTUP_APP_H
#define __STARTUP_APP_H

#include <AppKit.h>

#include "DataLoader.h"
#include "StartUpWin.h"

#define kAppName "StartUp"
#define kAppSignature "application/x-vnd.Haiku-StartUp"
#define kAppVersionStr "0.1"
#define kAppHomePage ""

class StartUpApp : public BApplication
{
public:
        StartUpApp          (bool experimental = false);
    virtual
        ~StartUpApp         ();
    virtual bool
        QuitRequested       ();
    virtual void
        MessageReceived     (BMessage* msg);
    virtual void
        AboutRequested      ();
    bool
        UnstableFeaturesEnabled() {
            return _exp;
        }
    BString
        CurrentUBS          ();
    std::vector<autolaunch_entry>
        CurrentALList       ();
    std::vector<environment_entry>
        CurrentUserEnv      ();
    BString
        CurrentProfileEnv   ();
private:
    StartUpWin                   *win;
    BString                       ubsdata;
    BString profdata;
    std::vector<autolaunch_entry> autolaunch_list;
    std::vector<environment_entry> environment_list;
    bool _exp;
};

#endif /* __STARTUP_APP_H */
