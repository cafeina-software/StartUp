#include <InterfaceKit.h>
#include <StorageKit.h>
#include <Catalog.h>
#include <LayoutBuilder.h>
#include <OutlineListView.h>
#include <PathFinder.h>
#include <Resources.h>
#include <kernel/fs_attr.h>
#include <algorithm>

#include "BlacklistView.h"
#include "DataLoader.h"
#include "StartUpDefs.h"

#undef B_TRANSLATION_CONTEXT
#define B_TRANSLATION_CONTEXT "bl view"

BlacklistView::BlacklistView(const char* title)
: BView(BRect(), title, 0, B_ASYNCHRONOUS_CONTROLS)
{
    _PreInitData();

    systreeview = new  MyOutlineListView("olv_sys_blacklist",
        B_SINGLE_SELECTION_LIST, syslistpopup);
    systreeview->SetSelectionMessage(new BMessage(BL_SYS_ITEM_SEL));

    sysAddButton = new BButton("bt_sys_add",
        B_TRANSLATE_COMMENT("Add", "For system list"),
        new BMessage(BL_SYS_ITEM_ADD));
    sysRemoveButton = new BButton("bt_sys_remove",
        B_TRANSLATE_COMMENT("Remove", "For system list"),
        new BMessage(BL_SYS_ITEM_REMOVE));
    sysRemoveButton->SetEnabled(false);
    sysResetButton = new BButton("bt_sys_clear",
        B_TRANSLATE_COMMENT("Clear", "For system list"),
        new BMessage(BL_SYS_LIST_CLEAR));
    sysOpenExtButton = new BButton("bt_sys_openxt",
        B_TRANSLATE_COMMENT("Open file", "For system list"),
        new BMessage(BL_SYS_LIST_OPEN));

    BView *systemview = new BView(BRect(), "System blacklist", 0, 0);
    BLayoutBuilder::Group<>(systemview, B_HORIZONTAL)
        .Add(systreeview)
        .AddGroup(B_VERTICAL)
            .SetInsets(0, 0, B_USE_DEFAULT_SPACING, 0)
            .Add(sysAddButton)
            .Add(sysRemoveButton)
            .Add(sysResetButton)
            .AddGlue()
            .Add(sysOpenExtButton)
        .End()
    .End();

    usrtreeview = new MyOutlineListView("olv_usr_blacklist",
        B_SINGLE_SELECTION_LIST, usrlistpopup);
    usrtreeview->SetSelectionMessage(new BMessage(BL_USR_ITEM_SEL));

    usrAddButton = new BButton("bt_usr_add",
        B_TRANSLATE_COMMENT("Add", "For user list"),
        new BMessage(BL_USR_ITEM_ADD));
    usrRemoveButton = new BButton("bt_usr_remove",
        B_TRANSLATE_COMMENT("Remove", "For user list"),
        new BMessage(BL_USR_ITEM_REMOVE));
    usrRemoveButton->SetEnabled(false);
    usrResetButton = new BButton("bt_usr_clear",
        B_TRANSLATE_COMMENT("Clear", "For user list"),
        new BMessage(BL_USR_LIST_CLEAR));
    usrOpenExtButton = new BButton("bt_usr_openxt",
        B_TRANSLATE_COMMENT("Open file", "For user list"),
        new BMessage(BL_USR_LIST_OPEN));

    BView *homeview = new BView(BRect(), "User blacklist", 0, 0);
    BLayoutBuilder::Group<>(homeview, B_HORIZONTAL)
        .Add(usrtreeview)
        .AddGroup(B_VERTICAL)
            .SetInsets(0, 0, B_USE_DEFAULT_SPACING, 0)
            .Add(usrAddButton)
            .Add(usrRemoveButton)
            .Add(usrResetButton)
            .AddGlue()
            .Add(usrOpenExtButton)
        .End()
    .End();

    _InitData();

    listView = new BListView("lv_main", B_SINGLE_SELECTION_LIST);
    listView->SetSelectionMessage(new BMessage(BL_SELECTED));
    listView->AddItem(new BStringItem(B_TRANSLATE("System")));
    listView->AddItem(new BStringItem(B_TRANSLATE("User")));
    listView->SetExplicitMinSize(
        BSize(listView->StringWidth(B_TRANSLATE("System")),
        B_SIZE_UNSET));
    BScrollView *scrollView = new BScrollView(NULL, listView, 0, false, true);
    scrollView->SetExplicitMinSize(BSize());

    float buttonMinWidth = std::max(scrollView->MinSize().Width(),
        StringWidth(B_TRANSLATE("Restore all")));
    updateButton = new BButton(B_TRANSLATE("Update"), new BMessage(BL_UPDATE));
    updateButton->SetExplicitMinSize(BSize(buttonMinWidth, B_SIZE_UNSET));
    restoreAllButton = new BButton(B_TRANSLATE("Restore all"), new BMessage(BL_RESTORE));
    restoreAllButton->SetExplicitMinSize(BSize(buttonMinWidth, B_SIZE_UNSET));

    cardview = new BCardView("cv_main");
    cardview->AddChild(systemview);
    cardview->AddChild(homeview);

    BLayoutBuilder::Group<>(this, B_HORIZONTAL)
        .AddSplit(B_HORIZONTAL, 4.0f)
            .AddGroup(B_VERTICAL)
                .SetInsets(B_USE_DEFAULT_SPACING, 0, 0, 0)
                .Add(scrollView)
                .Add(updateButton)
                .Add(restoreAllButton)
            .End()
            .Add(cardview)
        .End()
    .End();
}

BlacklistView::~BlacklistView()
{
    if(pkgicon)
        delete pkgicon;
    if(beappicon)
        delete beappicon;
    if(scripticon)
        delete scripticon;
    if(syspkglist) {
        syspkglist->Clear();
        delete syspkglist;
        syspkglist = nullptr;
    }
    if(usrpkglist) {
        usrpkglist->Clear();
        delete usrpkglist;
        usrpkglist = nullptr;
    }
    if(fp)
        delete fp;
}

// #pragma mark Public methods

void BlacklistView::AttachedToWindow()
{
    listView->SetTarget(this);
    systreeview->SetTarget(this);
    usrtreeview->SetTarget(this);
    updateButton->SetTarget(this);
    restoreAllButton->SetTarget(this);

    sysAddButton->SetTarget(this);
    sysRemoveButton->SetTarget(this);
    sysResetButton->SetTarget(this);
    sysOpenExtButton->SetTarget(this);

    usrAddButton->SetTarget(this);
    usrRemoveButton->SetTarget(this);
    usrResetButton->SetTarget(this);
    usrOpenExtButton->SetTarget(this);

    syspkgpopup->SetTargetForItems(this);
    sysitempopup->SetTargetForItems(this);
    syslistpopup->SetTargetForItems(this);

    usrpkgpopup->SetTargetForItems(this);
    usritempopup->SetTargetForItems(this);
    usrlistpopup->SetTargetForItems(this);
}

void BlacklistView::MessageReceived(BMessage *msg)
{
    switch(msg->what)
    {
        case BL_SELECTED:
        {
            int32 index = msg->GetInt32("index", 0);
            if(index >= 0)
                cardview->CardLayout()->SetVisibleItem(index);
            break;
        }
        case BL_LOCATION:
            break;
        case BL_SYS_VIEW_PKG:
            _ViewPkg(systreeview, syspkglist);
            break;
        case BL_USR_VIEW_PKG:
            _ViewPkg(usrtreeview, usrpkglist);
            break;
        case BL_TOGGLE_COLLAPSE:
            if(systreeview->IsFocus())
                _ToggleCollapse(systreeview);
            else if(usrtreeview->IsFocus())
                _ToggleCollapse(usrtreeview);
            break;
        case BL_UPDATE:
            Update();
            break;
        case BL_RESTORE:
        {
            BString text(B_TRANSLATE("Do you want to restore all the lists of blocked "
                "entries to the default settings?\nThis will delete the following "
                "lists:\n\n%lists%\n\n"));
            text.ReplaceAll("%lists%", BLACKLIST_SYSTEM "\n" BLACKLIST_USER);

            BAlert* alert = new BAlert(B_TRANSLATE("Blocking lists: restore all configs"),
                text, B_TRANSLATE("Do not restore"), B_TRANSLATE("Restore"),
                NULL, B_WIDTH_AS_USUAL, B_WARNING_ALERT);
            int32 result = alert->Go();

            if(result == 1)
                RestoreDefault();
            break;
        }
        case BL_SYS_ITEM_ADD:
        {
            BEntry entry("/boot/system");
            entry_ref ref;
            entry.GetRef(&ref);
            _AddToPanel(syspkglist, &ref);
            break;
        }
        case BL_USR_ITEM_ADD:
        {
            BEntry entry("/boot/home/config");
            entry_ref ref;
            entry.GetRef(&ref);
            _AddToPanel(usrpkglist, &ref);
            break;
        }
        case B_REFS_RECEIVED:
        {
            status_t ref_status;
            entry_ref ref;
            if(msg->FindRef("refs", &ref) == B_OK) {
                BEntry entry(&ref, true);
                BPath path;
                entry.GetPath(&path);

                PackageList* target = (PackageList*)msg->GetPointer("bl_target");

                if((ref_status = _AddToList(path.Path(), target)) == B_OK) {
                    blacklist_save(_ListBasePath(target), *target);
                    Update();
                }
                else {
                    BAlert *alert = new BAlert();
                    alert->SetTitle(B_TRANSLATE("Blocking list: error"));
                    alert->AddButton(B_TRANSLATE("OK"));
                    alert->SetType(B_STOP_ALERT);
                    if(ref_status == E_SYSPKGBIN_IN_USRLST)
                        alert->SetText(B_TRANSLATE("The selected executable is "
                            "part of a system installed package. It cannot be "
                            "added to the current user's blocked entries list."));
                    else if(ref_status == E_USRPKGBIN_IN_SYSLST)
                        alert->SetText(B_TRANSLATE("The selected executable is "
                            "part of an user installed package. It cannot be "
                            "added to the system-wide's blocked entries list."));
                    else if(ref_status == B_ENTRY_NOT_FOUND)
                        alert->SetText(B_TRANSLATE("This entry cannot be added "
                            "because it is not part of a HPKG package."));
                    else
                        alert->SetText(B_TRANSLATE("This entry cannot be added.\n\n"
                            "This could be because there is a problem with the "
                            "file or due to an unexpected error."));
                    alert->Go();
                }
            }
            break;
        }
        case BL_SYS_ITEM_SEL:
            sysRemoveButton->SetEnabled(true);
            break;
        case BL_USR_ITEM_SEL:
            usrRemoveButton->SetEnabled(true);
            break;
        case BL_SYS_ITEM_REMOVE:
            _Remove(systreeview, syspkglist);
            break;
        case BL_USR_ITEM_REMOVE:
            _Remove(usrtreeview, usrpkglist);
            break;
        case BL_SYS_LIST_CLEAR:
            _ClearList(BLACKLIST_SYSTEM, syspkglist);
            Update();
            break;
        case BL_USR_LIST_CLEAR:
            _ClearList(BLACKLIST_USER, usrpkglist);
            Update();
            break;
        case BL_SYS_LIST_OPEN:
            _OpenList(BLACKLIST_SYSTEM);
            break;
        case BL_USR_LIST_OPEN:
            _OpenList(BLACKLIST_USER);
            break;
        default:
            BView::MessageReceived(msg);
            break;
    }
}

void BlacklistView::Update()
{
    __trace("[Blacklist] Updating...\n");
    systreeview->MakeEmpty();
    syspkglist->Clear();

    usrtreeview->MakeEmpty();
    usrpkglist->Clear();

    _InitData();
}

void BlacklistView::RestoreDefault()
{
    blacklist_delete(BLACKLIST_SYSTEM, syspkglist);
    blacklist_delete(BLACKLIST_USER, usrpkglist);
    Update();
}

// #pragma mark Data initialization

void BlacklistView::_PreInitData()
{
    pkgicon = new BBitmap(BRect(0, 0, 24, 24), B_RGBA32);
    beappicon = new BBitmap(BRect(0, 0, 24, 24), B_RGBA32);
    scripticon = new BBitmap(BRect(0, 0, 24, 24), B_RGBA32);

    BMimeType hpkgtype("application/x-vnd.haiku-package");
    BMimeType beapptype("application/x-vnd.Be-elfexecutable");
    BMimeType srccodetype("text/x-source-code");

    hpkgtype.GetIcon(pkgicon, B_MINI_ICON);
    beapptype.GetIcon(beappicon, B_MINI_ICON);
    srccodetype.GetIcon(scripticon, B_MINI_ICON);

    syspkglist = new PackageList();
    usrpkglist = new PackageList();

    BMessage updtmsg;
    updtmsg.AddString("class", "BMenuItem");
    updtmsg.AddString("_label", B_TRANSLATE("Update"));
    updtmsg.AddMessage("_msg", new BMessage(BL_UPDATE));
    BMessage remvmsg;
    remvmsg.AddString("class", "BMenuItem");
    remvmsg.AddString("_label", B_TRANSLATE("Remove"));
    remvmsg.AddMessage("_msg", NULL);
    BMessage exclmsg;
    exclmsg.AddString("class", "BMenuItem");
    exclmsg.AddString("_label", B_TRANSLATE("Expand / collapse"));
    exclmsg.AddMessage("_msg", new BMessage(BL_TOGGLE_COLLAPSE));
    BMessage viewpkgmsg;
    viewpkgmsg.AddString("class", "BMenuItem");
    viewpkgmsg.AddString("_label", B_TRANSLATE("View package"));
    viewpkgmsg.AddMessage("_msg", NULL);

    syspkgpopup = new BPopUpMenu(NULL, false, false, B_ITEMS_IN_COLUMN);
    syspkgpopup->AddItem(new BMenuItem(&exclmsg));
    syspkgpopup->AddSeparatorItem();
    syspkgpopup->AddItem(new BMenuItem(&updtmsg));
    syspkgpopup->AddSeparatorItem();
    syspkgpopup->AddItem(new BMenuItem(&viewpkgmsg));
    syspkgpopup->FindItem(B_TRANSLATE("View package"))->SetMessage(new BMessage(BL_SYS_VIEW_PKG));
    syspkgpopup->AddItem(new BMenuItem(&remvmsg));
    syspkgpopup->FindItem(B_TRANSLATE("Remove"))->SetMessage(new BMessage(BL_SYS_ITEM_REMOVE));
    sysitempopup = new BPopUpMenu(NULL, false, false, B_ITEMS_IN_COLUMN);
    sysitempopup->AddItem(new BMenuItem(&updtmsg));
    sysitempopup->AddSeparatorItem();
    sysitempopup->AddItem(new BMenuItem(&remvmsg));
    sysitempopup->FindItem(B_TRANSLATE("Remove"))->SetMessage(new BMessage(BL_SYS_ITEM_REMOVE));
    syslistpopup = new BPopUpMenu(NULL, false, false, B_ITEMS_IN_COLUMN);
    syslistpopup->AddItem(new BMenuItem(&updtmsg));
    usrpkgpopup = new BPopUpMenu(NULL, false, false, B_ITEMS_IN_COLUMN);
    usrpkgpopup->AddItem(new BMenuItem(&exclmsg));
    usrpkgpopup->AddSeparatorItem();
    usrpkgpopup->AddItem(new BMenuItem(&updtmsg));
    usrpkgpopup->AddSeparatorItem();
    usrpkgpopup->AddItem(new BMenuItem(&viewpkgmsg));
    usrpkgpopup->FindItem(B_TRANSLATE("View package"))->SetMessage(new BMessage(BL_USR_VIEW_PKG));
    usrpkgpopup->AddItem(new BMenuItem(&remvmsg));
    usrpkgpopup->FindItem(B_TRANSLATE("Remove"))->SetMessage(new BMessage(BL_USR_ITEM_REMOVE));
    usritempopup = new BPopUpMenu(NULL, false, false, B_ITEMS_IN_COLUMN);
    usritempopup->AddItem(new BMenuItem(&updtmsg));
    usritempopup->AddSeparatorItem();
    usritempopup->AddItem(new BMenuItem(&remvmsg));
    usritempopup->FindItem(B_TRANSLATE("Remove"))->SetMessage(new BMessage(BL_USR_ITEM_REMOVE));
    usrlistpopup = new BPopUpMenu(NULL, false, false, B_ITEMS_IN_COLUMN);
    usrlistpopup->AddItem(new BMenuItem(&updtmsg));
}

void BlacklistView::_InitData()
{
    BString syspath(BLACKLIST_SYSTEM);
    if(blacklist_load(syspath, syspkglist) == B_OK) {
        for(auto& pkg : syspkglist->Packages()) {
            package = new IconStringItem(pkg.Name(), pkgicon, syspkgpopup);
            systreeview->AddItem(package);
            if(systreeview)
                ((IconStringItem*)package)->SetParentPtr(systreeview);

            for(auto& pkgitem : *pkg.Blacklist()) {
                item = new IconStringItem(pkgitem, beappicon, sysitempopup);
                systreeview->AddUnder(item, package);
                if(systreeview)
                    ((IconStringItem*)package)->SetParentPtr(systreeview);
            }
        }
    }
    else
        __trace("[Blacklist] Error! System blacklist could not be loaded."
                "The file may not yet exists.\n");

    BListItem* upackage, *uitem;
    BString usrpath(BLACKLIST_USER);
    if(blacklist_load(usrpath, usrpkglist) == B_OK) {
        for(auto& upkg : usrpkglist->Packages()) {
            upackage = new IconStringItem(upkg.Name(), pkgicon, usrpkgpopup);
            usrtreeview->AddItem(upackage);
            if(usrtreeview)
                ((IconStringItem*)package)->SetParentPtr(usrtreeview);

            for(auto& upkgitem : *upkg.Blacklist()) {
                uitem = new IconStringItem(upkgitem, beappicon, usritempopup);
                usrtreeview->AddUnder(uitem, upackage);
                if(usrtreeview)
                    ((IconStringItem*)package)->SetParentPtr(usrtreeview);
            }
        }
    }
    else
        __trace("[Blacklist] Error! User blacklist could not be loaded. "
                "The file may not yet exists.\n");
}

// #pragma mark Data input

void BlacklistView::_AddToPanel(PackageList* targetlist, entry_ref* ref)
{
    BMessenger msgr(this);
    BMessage fpmsg(B_REFS_RECEIVED);
    fpmsg.AddPointer("bl_target", targetlist);
    ExecutableFilter *exefilter = new ExecutableFilter();

    if(fp)
        delete fp;
    fp = new BFilePanel(B_OPEN_PANEL, &msgr, ref, B_FILE_NODE,
        false, &fpmsg, exefilter, false, true);
    fp->Show();
}

status_t BlacklistView::_AddToList(BString path, PackageList* list)
{
    status_t status = B_OK;
    BString pkgnamever, pkgname;

    __trace("[Blacklist] Received to process: %s\n", path.String());

    if((status = _IsPartOfHPKG(path.String(), &pkgnamever, &pkgname)) != B_OK) {
        __trace("[Blacklist] Error: could not be verified part of hpkg: %s\n", path.String());
        return status;
    }

    __trace("[Blacklist] At this point: \n\t(list base) %s\n\t(bin path) %s"
            "\n\t(pkgnamever) %s\n\t(pkgname) %s\n",
        _ListBasePath(list).String(), path.String(),
        pkgnamever.String(), pkgname.String());

    if(_ListBasePath(list) == BLACKLIST_SYSTEM && _TypeOfHPKG(pkgnamever) != P_TYPE_SYSTEM) {
        __trace("[Blacklist] Error: this entry: %s is part of an user installed package,"
                " cannot be added to the system blacklist.\n", path.String());
        return E_USRPKGBIN_IN_SYSLST;
    }
    else if(_ListBasePath(list) == BLACKLIST_USER && _TypeOfHPKG(pkgnamever) != P_TYPE_USER) {
        __trace("[Blacklist] Error: this package: %s is part of a system "
        "installed package, cannot be added to the user blacklist.\n", path.String());
        return E_SYSPKGBIN_IN_USRLST;
    }

    BStringList splitlist;
    path.Split("/", true, splitlist);
    BString newstring;
    for(int i = 0; i < splitlist.CountStrings(); i++) {
        if(i > 1) {
            newstring << splitlist.StringAt(i).String();
            if(i < splitlist.CountStrings() - 1)
                newstring << "/";
        }
    }

    if(list->HasPackage(pkgname))
        list->PackageByName(pkgname)->AddBlacklistItem(newstring);
    else {
        list->AddPackage(pkgname);
        list->PackageByName(pkgname)->AddBlacklistItem(newstring);
    }

    return status;
}

// #pragma mark Data deletion

void BlacklistView::_Remove(MyOutlineListView* olv, PackageList* list)
{
    BAlert* alert = new BAlert();
    alert->SetTitle(B_TRANSLATE("Blocking list: delete item"));

    BListItem* selItem;
    int32 selected;
    if((selected = olv->CurrentSelection()) >= 0) {
        selItem = olv->ItemAt(selected);
        BListItem* super = olv->Superitem(selItem);

        BString text(B_TRANSLATE("Do you want to remove this %type% from the "
                                 "blocked entries list:\n\n%what%"));
        text.ReplaceAll("%type%", super == NULL ?
            B_TRANSLATE("package and its contents") : B_TRANSLATE("item"));
        text.ReplaceAll("%what%", dynamic_cast<IconStringItem*>(selItem)->Text());
        alert->SetText(text);
        alert->SetType(B_WARNING_ALERT);
        alert->AddButton(B_TRANSLATE("Do not remove"));
        alert->AddButton(B_TRANSLATE("Remove"));
        alert->SetShortcut(0, B_ESCAPE);
        int32 result = alert->Go();

        if(result == 1) {
            if(super == NULL)
                _RemovePkg(list, dynamic_cast<IconStringItem*>(selItem));

            else
                _RemoveEntry(list,
                    dynamic_cast<IconStringItem*>(olv->Superitem(selItem)),
                    dynamic_cast<IconStringItem*>(selItem));
        }
    }
    else {
        fprintf(stderr, "[%s] Target not valid.\n", __func__);
        alert->SetType(B_STOP_ALERT);
        alert->SetText(B_TRANSLATE("Target not valid. A item or a package "
            "must be selected first."));
        alert->AddButton(B_TRANSLATE("OK"));
        alert->Go();
    }
}

status_t BlacklistView::_RemovePkg(PackageList* list, IconStringItem* item)
{
    __trace("[Blacklist] Package to be deleted: %s\n", item->Text());
    if(list->HasPackage(item->Text())) {
        list->RemovePackage(item->Text());
        blacklist_save(_ListBasePath(list), *list);
        Update();
        return B_OK;
    }
    return B_ENTRY_NOT_FOUND;
}

status_t BlacklistView::_RemoveEntry(PackageList* list, IconStringItem* parent, IconStringItem* item)
{
    __trace("[Blacklist] Item to be deleted: %s->%s\n", parent->Text(), item->Text());
    if (list->HasPackage(parent->Text())) {
        list->PackageByName(parent->Text())->RemoveBlacklistItem(item->Text());
        if(list->PackageByName(parent->Text())->IsBlacklistEmpty())
            list->RemovePackage(parent->Text());
        blacklist_save(_ListBasePath(list), *list);
        Update();
        return B_OK;
    }
    return B_ENTRY_NOT_FOUND;
}

void BlacklistView::_ClearList(BString path, PackageList* list)
{
    BAlert *alert = new BAlert();
    alert->SetTitle("Blocking list: clear");

    if(exists(path.String())) {
        alert->SetText(B_TRANSLATE("Do you want to clear this list? "
            "This cannot be undone."));
        alert->AddButton(B_TRANSLATE("Do not clear"));
        alert->AddButton(B_TRANSLATE("Clear"));
        alert->SetType(B_WARNING_ALERT);
    }
    else {
        alert->SetText(B_TRANSLATE("The list cannot be cleared because the "
            "blocked entries file currently does not exist."));
        alert->AddButton(B_TRANSLATE("OK"));
        alert->SetType(B_IDEA_ALERT);
    }
    int32 result = alert->Go();
    if(result == 1) {
        // list->MakeEmpty();
        blacklist_delete(path, list);
    }
}

// #pragma mark Checks

status_t BlacklistView::_IsPartOfHPKG(BString path, BString* hpkgname, BString* hpkgshort)
{
    __trace("[Blacklist] At this point, path is: %s\n", path.String());
    BNode binary(path);

    if(hpkgname != NULL)
        hpkgname->SetTo("");
    if(hpkgshort != NULL)
        hpkgshort->SetTo("");

    attr_info info;
    status_t status = binary.GetAttrInfo("SYS:PACKAGE_FILE", &info);
    if(status != B_OK) { // attr not found or file not initialized
        __trace("[Blacklist] Error trying to find attrib in: %s\n", path.String());
        return status;
    }

    char* data = new char[info.size];
    ssize_t readbytes = binary.ReadAttr("SYS:PACKAGE_FILE", info.type, 0, (void*) data, info.size);
    if(readbytes <= 0) {// no valid data
        __trace("[Blacklist] Error trying to read attrib in: %s\n", path.String());
        delete[] data;
        return B_ERROR;
    }

    if(hpkgname != NULL)
        hpkgname->SetTo(data);
    if(hpkgshort != NULL) {
        BString nameonly(*hpkgname);
        BStringList splitted;
        nameonly.Split("-", true, splitted);

        if(splitted.CountStrings() > 0) {
            hpkgshort->SetTo(splitted.First());
            __trace("[Blacklist] Splitted: %s\n", splitted.First().String());
        }
    }
    __trace("[Blacklist] At this point, path %s is: OK\n", path.String());
    delete[] data;
    return B_OK;
}

pkgtype_t BlacklistView::_TypeOfHPKG(BString pkgnamever)
{
    __trace("[Blacklist] Started. Data received: %s\n", pkgnamever.String());
    BPathFinder finder;
    BStringList matchlist;
    pkgtype_t result;
    bool found = false;

    status_t status = finder.FindPaths(B_FIND_PATH_PACKAGES_DIRECTORY, matchlist);
    int i = 0;
    while(i < matchlist.CountStrings() && !found) {
        BPath path;
        path.SetTo(matchlist.StringAt(i), pkgnamever, true);

        __trace("[Blacklist] Check: %s %s exist.\n", path.Path(),
            exists(path.Path()) ? "does" : "does not");

        if(exists(path.Path())) {
            found = true;
            if(matchlist.StringAt(i) == "/boot/system/packages")
                result = P_TYPE_SYSTEM;
            else if(matchlist.StringAt(i) == "/boot/system/non-packaged/packages")
                result = P_TYPE_SYSTEM_NP;
            else if(matchlist.StringAt(i) == "/boot/home/config/packages")
                result = P_TYPE_USER;
            else if(matchlist.StringAt(i) == "/boot/home/config/non-packaged/packages")
                result = P_TYPE_USER_NP;
            else
                result = P_TYPE_UNKNOWN;
        }
        i++;
    }
    return result;
}

// #pragma mark Other utils

BString BlacklistView::_ListBasePath(PackageList* list)
{
    if(list == syspkglist)
        return BLACKLIST_SYSTEM;
    else
        return BLACKLIST_USER;
}

void BlacklistView::_OpenList(BString listpath)
{
    if(exists(listpath))
        launch("text/plain", listpath);
    else {
        BString text(B_TRANSLATE("The file could not be opened. This could be "
            "because the file %filename% does not exist or there is not enough "
            "permissions to open it."));
        text.ReplaceAll("%filename%", listpath);

        BAlert* alert = new BAlert(B_TRANSLATE("Blocking list: list not found"),
            text, B_TRANSLATE("OK"), NULL, NULL, B_WIDTH_AS_USUAL, B_STOP_ALERT);
        alert->Go();
    }
}

void BlacklistView::_ToggleCollapse(MyOutlineListView* olv)
{
    int32 selindex;
    if((selindex = olv->CurrentSelection()) >= 0) {
        if(olv->Superitem(olv->ItemAt(selindex)) == NULL) {
            if(olv->IsExpanded(selindex))
                olv->Collapse(olv->ItemAt(selindex));
            else
                olv->Expand(olv->ItemAt(selindex));
        }
    }
}

void BlacklistView::_ViewPkg(MyOutlineListView* olv, PackageList* list)
{
    int32 selindex;
    if((selindex = olv->CurrentSelection()) >= 0) {
        if(olv->Superitem(olv->ItemAt(selindex)) == NULL) {
            BString targetname(dynamic_cast<IconStringItem*>(olv->ItemAt(selindex))->Text());
            __trace("Target is: %s\n", targetname.String());

            entry_ref ref;
            auto packagepath = [=](PackageList* list) {
                return list == syspkglist ? "/boot/system/packages" :
                                            "/boot/home/config/packages";
            };

            bool found = false;
            BString fstr;
            BDirectory pkgpathdir(packagepath(list));
            while(pkgpathdir.GetNextRef(&ref) == B_OK && !found) {
                BEntry entry(&ref);
                BPath path;
                entry.GetPath(&path);

                if(strncmp(path.Leaf(), targetname.String(), strlen(targetname.String())) == 0) {
                    found = true;
                    fstr = path.Path();
                }
            }
            if(found)
                launch("application/x-vnd.haiku-package", fstr.String());
        }
    }
}
