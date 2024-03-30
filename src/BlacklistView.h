#ifndef __BLACKLIST_VIEW_H
#define __BLACKLIST_VIEW_H

#include <InterfaceKit.h>
#include <StorageKit.h>
#include <SupportKit.h>

#include "CustomUI.h"
#include "DataLoader.h"

#define BL_UPDATE            'updt'
#define BL_RESTORE           'rstr'
#define BL_SELECTED          'isel'
#define BL_LOCATION          'iloc'
#define BL_TOGGLE_COLLAPSE   'excl'

#define BL_SYS_ITEM_ADD      'sadd'
#define BL_SYS_ITEM_SEL      'ssel'
#define BL_SYS_ITEM_REMOVE   'srmv'
#define BL_SYS_LIST_CLEAR    'srst'
#define BL_SYS_LIST_OPEN     'soxt'
#define BL_SYS_VIEW_PKG      'spkg'

#define BL_USR_ITEM_ADD      'uadd'
#define BL_USR_ITEM_SEL      'usel'
#define BL_USR_ITEM_REMOVE   'urmv'
#define BL_USR_LIST_CLEAR    'urst'
#define BL_USR_LIST_OPEN     'uoxt'
#define BL_USR_VIEW_PKG      'upkg'

#define E_USRPKGBIN_IN_SYSLST (B_ERRORS_END + 1)
#define E_SYSPKGBIN_IN_USRLST (B_ERRORS_END + 2)

typedef enum {
    P_TYPE_SYSTEM,
    P_TYPE_SYSTEM_NP,
    P_TYPE_USER,
    P_TYPE_USER_NP,
    P_TYPE_UNKNOWN = B_ERROR
} pkgtype_t;

class ExecutableFilter : public BRefFilter
{
public:
    virtual bool      Filter(const entry_ref* ref, BNode* node, struct stat_beos* stat,
        const char* mimetype) {
        return (node->IsDirectory()) ||
               (strcmp(mimetype, "application/x-vnd.Be-elfexecutable") == 0) ||
               (strcmp(mimetype, "application/x-vnd.be-elfexecutable") == 0) || // Genio uses this
               (strcmp(mimetype, "text/x-shellscript") == 0) ||
               (strcmp(mimetype, "application/x-shellscript") == 0) || // older mime type for shell scripts
               (strstr(ref->name, ".sh") != NULL);
    }
};

class BlacklistView : public BView
{
public:
                       BlacklistView     (const char* title);
    virtual           ~BlacklistView     ();
    virtual void       AttachedToWindow  ();
    virtual void       MessageReceived   (BMessage* msg);
    void               Update            ();
    void               RestoreDefault    ();
private:
    void               _PreInitData      ();
    void               _InitData         ();
    void               _AddToPanel       (PackageList* targetlist, entry_ref* r = NULL);
    status_t           _AddToList        (BString path, PackageList* plist);
    void               _Remove           (MyOutlineListView* olv, PackageList* plist);
    status_t           _RemovePkg        (PackageList* plist, IconStringItem* item);
    status_t           _RemoveEntry      (PackageList* plist, IconStringItem* parent, IconStringItem* item);
    void               _ClearList        (BString path, PackageList* plist);
    status_t           _IsPartOfHPKG     (BString path, BString* hpkgname = NULL, BString* hpkgshort = NULL);
    pkgtype_t          _TypeOfHPKG       (BString pkgnamever);
    BString            _PackagePath      (PackageList* list);
    BString            _ListBasePath     (PackageList* list);
    void               _OpenList         (BString listpath);
    void               _ToggleCollapse   (MyOutlineListView* olv);
    void               _ViewPkg          (MyOutlineListView* olv, PackageList* list);
private:
    PackageList       *syspkglist,
                      *usrpkglist;
    BBitmap           *pkgicon,
                      *beappicon,
                      *scripticon;
    BCardView         *cardview;
    MyOutlineListView *systreeview,
                      *usrtreeview;
    BListView         *listView;
    BListItem         *package,
                      *item;
    BButton           *updateButton,
                      *restoreAllButton,
                      *sysAddButton,
                      *sysRemoveButton,
                      *sysResetButton,
                      *sysOpenExtButton,
                      *usrAddButton,
                      *usrRemoveButton,
                      *usrUpdateButton,
                      *usrResetButton,
                      *usrOpenExtButton;
    BFilePanel        *fp;
    BPopUpMenu        *syspkgpopup,
                      *sysitempopup,
                      *syslistpopup,
                      *usrpkgpopup,
                      *usritempopup,
                      *usrlistpopup;
};

#endif /* __BLACKLIST_VIEW_H */

