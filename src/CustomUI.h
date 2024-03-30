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

class MyOutlineListView : public BOutlineListView
{
public:
                  MyOutlineListView(const char* name,
                    list_view_type type	= B_SINGLE_SELECTION_LIST,
                    BPopUpMenu* _menu = nullptr,
                    uint32 flags = B_WILL_DRAW | B_FRAME_EVENTS | B_NAVIGABLE);
    virtual void  MouseDown(BPoint where);
    virtual void  MessageReceived(BMessage* msg);
private:
    BPopUpMenu   *menu;
};

class IconStringItem : public BStringItem
{
public:
                  IconStringItem (const char* text, BBitmap* icon = nullptr,
                                  BPopUpMenu* menu = nullptr, BView* owner = nullptr,
                                  uint32 outlineLevel = 0, bool expanded = true,
                                  BString _extra = NULL);
    virtual void  DrawItem       (BView* owner, BRect frame, bool complete = false);
    virtual void  MouseDown      (BOutlineListView* owner, BPoint where,
                                  uint32 buttons, bool issuper, bool isexpanded);
    void          SetIcon        (BBitmap* icon);
    BBitmap      *Icon           ();
    void          SetParentPtr   (BView* onwer);
    BView        *ParentPtr      ();
    void          SetExtraText   (BString text);
    BString      *ExtraText      ();
private:
    BBitmap      *itemicon;
    BPopUpMenu   *menu;
    BView        *parent;
    BString       extratext;
};

#endif /* __STARTUP_CUSTOM_UI_H */

