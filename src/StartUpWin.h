#ifndef __STARTUP_WIN_H
#define __STARTUP_WIN_H

#include <InterfaceKit.h>
#include "AutolaunchView.h"
#include "KernelSettingsView.h"
#include "TextEditorView.h"
#include "UserScriptsView.h"

class StartUpWin : public BWindow
{
public:
                         StartUpWin          ();
    virtual             ~StartUpWin          ();
    virtual void         MessageReceived     (BMessage* msg);
    void                 StartMonitoring     ();
    void                 StopMonitoring      ();
    status_t             LoadSettings        (BMessage* indata);
    status_t             SaveSettings        (BMessage* outdata);
private:
    void                 OpenDocumentation   (const char* entry);
private:
    BTabView               *tabView;
    BPopUpMenu             *optionsMenu;
    BMenu                  *helpMenu,
                           *settingsMenu;
    BMenuField             *optionsMenuField;
    AutolaunchView         *alview;
    UserScriptsView        *usview;
    TextEditorView         *ubsview,
                           *termenvview;
    KernelSettingsView     *kernview;

    bool                    kswarningshown;
};

#endif /* __STARTUP_WIN_H */
