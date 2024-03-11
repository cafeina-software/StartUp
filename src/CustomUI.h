#ifndef __STARTUP_CUSTOM_UI_H
#define __STARTUP_CUSTOM_UI_H

#include <InterfaceKit.h>

#define TAB_SELECTED 'tabs'

class MyTab : public BTab
{
public:
                  MyTab         (uint32 id, BView* contentsView = NULL);
    virtual void  Select        (BView* owner);
    uint32        Id            ();
private:
    uint32        identificador;
};

#endif /* __STARTUP_CUSTOM_UI_H */

