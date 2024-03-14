#ifndef __USER_SCRIPTS_H
#define __USER_SCRIPTS_H

#include <InterfaceKit.h>

#define US_ITEM_SELECTED 'isel'

class UserScriptsView : public BView
{
public:
                 UserScriptsView  (const char* title, BString, BString, BString, BString);
    virtual void AttachedToWindow ();
    virtual void MessageReceived  (BMessage* msg);
private:
    BListView    *fListView;
    BCardView    *fCardView;
    BString ubs;
    BString uss;
    BString usf;
    BString use;
};

#endif  /* __USER_SCRIPTS_H */
