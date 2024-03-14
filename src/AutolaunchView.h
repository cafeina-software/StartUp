#ifndef __AL_VIEW_H
#define __AL_VIEW_H

#include <InterfaceKit.h>
#include <StorageKit.h>
#include <private/interface/ColumnListView.h>
#include <vector>

#include "DataLoader.h"

#define ALV_ADDMENU       'aaaa'
#define ALV_ADD_LINK      'addl'
#define ALV_ADD_FILE      'addf'
#define ALV_EDIT_ENTRY    'eden'
#define ALV_DELETE_ENTRY  'dlen'
#define ALV_OPEN_ENTRY    'eopn'
#define ALV_ITEM_SELECTED 'isel'
#define ALV_ITEM_UPDATE   'iupd'
#define ALV_MSG_COPY      'copy'
#define ALV_NEW_SCRIPT    'news'

class EditEntryWindow;
class NewScriptWindow;

class AutolaunchView : public BView
{
public:
                    AutolaunchView      (const char* title, std::vector<autolaunch_entry> autolaunch_list);
    virtual        ~AutolaunchView      ();
    virtual void    MessageReceived     (BMessage* msg);
    virtual void    AttachedToWindow    ();
    void            _Update             ();
private:
    void            _Init               ();
private:
    BColumnListView        *alListView;
    BButton                *addButton,
                           *editButton,
                           *deleteButton,
                           *updateButton,
                           *openextButton;
    BPopUpMenu             *addMenu;
    BFilePanel             *filepanel,
                           *copypanel;
    EditEntryWindow        *editwindow;
    NewScriptWindow        *scriptwindow;

    std::vector<autolaunch_entry> _list;
};

class EditEntryWindow : public BWindow
{
public:
                 EditEntryWindow     ();
    virtual void MessageReceived     (BMessage* msg);
private:
    BTextControl *entryName,
                 *entryType,
                 *entryInfo;
    BButton      *saveButton,
                 *cancelButton;
};

class NewScriptWindow : public BWindow
{
public:
                 NewScriptWindow     (BView* parent = nullptr);
    virtual void AttachedToWindow    ();
    virtual void MessageReceived     (BMessage* msg);
private:
	BString       name;
	BString       shell;

	BView        *_parent;

    BTextControl *entryName;
    BMenuField   *shellMenu;
    BPopUpMenu   *shellPopUp;
    BButton      *saveButton,
                 *cancelButton;
};

#endif /* __AL_VIEW_H */
