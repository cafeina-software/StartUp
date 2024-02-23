#ifndef __STARTUP_WIN_H
#define __STARTUP_WIN_H

#include <InterfaceKit.h>
#include "AutolaunchView.h"
#include "TextEditorView.h"
#include "EnvironmentView.h"

class StartUpWin : public BWindow
{
public:
        StartUpWin();
    virtual void
        MessageReceived     (BMessage* msg);
    void
        OpenDocumentation   (const char* entry);
private:
    BTabView               *tabView;
    BPopUpMenu             *optionsMenu,
                           *helpMenu;
    BMenuField             *optionsMenuField;
    AutolaunchView         *alview;
    TextEditorView         *ubsview,
                           *termenvview;
    EnvironmentView        *envview;
};

#endif /* __STARTUP_WIN_H */
