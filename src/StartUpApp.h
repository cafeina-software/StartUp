#ifndef __STARTUP_APP_H
#define __STARTUP_APP_H

#include <AppKit.h>

#include "DataLoader.h"
#include "StartUpWin.h"

class StartUpApp : public BApplication
{
public:
                                   StartUpApp            ();
    virtual                       ~StartUpApp            ();
    virtual bool                   QuitRequested         ();
    virtual void                   MessageReceived       (BMessage* msg);
    virtual void                   AboutRequested        ();
    BString                        CurrentUBS            ();
    BString                        CurrentUSS            ();
    BString                        CurrentUSF            ();
    std::vector<autolaunch_entry>  CurrentALList         ();
    BString                        CurrentUserEnv        ();
    BString                        CurrentProfileEnv     ();
    std::vector<entry>             CurrentKernelSettings ();
private:
    status_t                       _LoadSettings         ();
    status_t                       _SaveSettings         ();
private:
    StartUpWin                    *win;

    BString                        ubsdata;
    BString                        ussdata;
    BString                        usfdata;
    BString                        usedata;
    BString                        profdata;
    std::vector<autolaunch_entry>  autolaunch_list;
    std::vector<entry>             kernelsettings_list;
    BMessage                       currentSettings;
};

#endif /* __STARTUP_APP_H */
