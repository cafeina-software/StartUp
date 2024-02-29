#ifndef __ENVIRONMENT_VIEW_H
#define __ENVIRONMENT_VIEW_H

#include "DataLoader.h"
#include <InterfaceKit.h>
#include <private/interface/ColumnListView.h>

#define ENV_ITEM_ADD        'iadd'
#define ENV_ITEM_EDIT       'iedt'
#define ENV_ITEM_DELETE     'idel'
#define ENV_ITEM_SELECTED   'isel'
#define ENV_ITEM_DEFAULT    'idef'
#define ENV_ITEMLIST_UPDATE 'iupd'
#define ENV_OPEN_EXT        'opxt'

class EditEnvWindow;

class EnvironmentView : public BView
{
public:
        EnvironmentView     (const char* title, std::vector<entry> entrylist);
    virtual
        ~EnvironmentView    ();
    virtual void
        MessageReceived	    (BMessage* msg);
    virtual void
        AttachedToWindow    ();
    void
        _Init               ();
    void
        _Update             ();
private:
    std::vector<entry>  _list;
    BRow                           *current;

    BColumnListView                *environmentView;
    BButton                        *addButton,
                                   *editButton,
                                   *deleteButton,
                                   *updateButton,
                                   *resetButton,
                                   *openextButton;
    EditEnvWindow                  *addenvWindow,
                                   *edenvWindow;
};

class EditEnvWindow : public BWindow
{
public:
        EditEnvWindow       (BView* parent, std::vector<entry>* plist, BRow* target = NULL);
    virtual void
        MessageReceived     (BMessage* msg);
    void
        AttachedToWindow();
private:
    BTextControl *entryKey,
                 *entryValue;
    BCheckBox    *isEnabled;
    BButton      *saveButton,
                 *cancelButton;
    BView        *_parent;
    BRow         *_target;

    std::vector<entry>* _plist;

    bool          enabled;
    BString       key;
    BString       value;
};

#endif /* __ENVIRONMENT_VIEW_H */

