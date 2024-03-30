#include <AppKit.h>
#include <Catalog.h>
#include <InterfaceKit.h>
#include <StorageKit.h>
#include <private/interface/ColumnListView.h>
#include <private/interface/ColumnTypes.h>
#include <cstdio>

#include "AutolaunchView.h"

#undef B_TRANSLATION_CONTEXT
#define B_TRANSLATION_CONTEXT "Autolaunch view"

AutolaunchView::AutolaunchView(const char* title, std::vector<autolaunch_entry> autolaunch_list)
: BView(BRect(), title, B_FOLLOW_ALL, B_ASYNCHRONOUS_CONTROLS),
  _list(autolaunch_list)
{
    alListView = new BColumnListView("name", 0);
    alListView->SetSelectionMode(B_SINGLE_SELECTION_LIST);
    alListView->SetSelectionMessage(new BMessage(ALV_ITEM_SELECTED));
    alListView->AddColumn(new BStringColumn(B_TRANSLATE("Command"), 200, 100, 500, 0), 0);
    alListView->AddColumn(new BStringColumn(B_TRANSLATE("Type"), 200, 100, 500, 0), 1);
    alListView->AddColumn(new BStringColumn(B_TRANSLATE("Data"), 200, 100, 500, 0), 2);
    _Init();
    alListView->ResizeAllColumnsToPreferred();

    addButton = new BButton("bt_add", B_TRANSLATE("Add"), new BMessage(ALV_ADDMENU));
    addButton->SetBehavior(BButton::B_POP_UP_BEHAVIOR);
    editButton = new BButton(NULL, B_TRANSLATE("Edit"), new BMessage(ALV_EDIT_ENTRY));
    deleteButton = new BButton(NULL, B_TRANSLATE("Delete"), new BMessage(ALV_DELETE_ENTRY));
    openextButton = new BButton(NULL, B_TRANSLATE("Open folder"), new BMessage(ALV_OPEN_ENTRY));
    updateButton = new BButton(NULL, B_TRANSLATE("Update"), new BMessage(ALV_ITEM_UPDATE));

    addMenu = new BPopUpMenu("", false, false);
    addMenu->AddItem(new BMenuItem(B_TRANSLATE("Add file link" B_UTF8_ELLIPSIS), new BMessage(ALV_ADD_LINK)));
    addMenu->AddItem(new BMenuItem(B_TRANSLATE("Copy file" B_UTF8_ELLIPSIS), new BMessage(ALV_ADD_FILE)));
    addMenu->AddSeparatorItem();
    addMenu->AddItem(new BMenuItem(B_TRANSLATE("Create shell script" B_UTF8_ELLIPSIS), new BMessage(ALV_NEW_SCRIPT)));

    BLayoutBuilder::Group<>(this, B_HORIZONTAL)
        .Add(alListView)
        .AddGroup(B_VERTICAL)
            .SetInsets(0, 0, B_USE_DEFAULT_SPACING, 0)
            .Add(addButton)
            // .Add(editButton)
            .Add(deleteButton)
            .AddStrut(10.0f)
            .Add(updateButton)
            .AddGlue()
            .Add(openextButton)
        .End()
    .End();
}

AutolaunchView::~AutolaunchView()
{
    if(filepanel)
        delete filepanel;
    if(copypanel)
        delete copypanel;
    if(editwindow && editwindow->Lock())
        editwindow->Quit();
    if(scriptwindow && scriptwindow->Lock())
        scriptwindow->Quit();
}

void AutolaunchView::_Init()
{
    for (const auto& entry : _list) {
        BString flavor, data;
        switch(entry.flavor){
            case B_FILE_NODE:
            flavor = B_TRANSLATE("File");
            data.Append("Mime: ").Append(entry.data);
            break;
            case B_DIRECTORY_NODE:
            flavor = B_TRANSLATE("Directory");
            data = entry.data;
            break;
            case B_SYMLINK_NODE:
            flavor = B_TRANSLATE("Symlink");
            data.Append(B_TRANSLATE("Destination: ")).Append(entry.data);
            break;
            default:
            flavor = B_TRANSLATE("Unknown");
            data = entry.data;
            break;
        }

        BRow* row = new BRow();
        row->SetField(new BStringField(entry.path.Path()), 0);
        row->SetField(new BStringField(flavor.String()), 1);
        row->SetField(new BStringField(data), 2);
        alListView->AddRow(row);
    }
}

void AutolaunchView::AttachedToWindow()
{
    addButton->SetTarget(this);
    editButton->SetTarget(this);
    deleteButton->SetTarget(this);
    openextButton->SetTarget(this);
    updateButton->SetTarget(this);
    addMenu->SetTargetForItems(this);
    // alListView->SetTarget(this);

    float width, maxwidth, height;
    addButton->GetPreferredSize(&width, &height);
    maxwidth = width;
    editButton->GetPreferredSize(&width, NULL);
    if(width > maxwidth)
        maxwidth = width;
    deleteButton->GetPreferredSize(&width, NULL);
    if(width > maxwidth)
        maxwidth = width;
    openextButton->GetPreferredSize(&width, NULL);
    if(width > maxwidth)
        maxwidth = width;
    updateButton->GetPreferredSize(&width, NULL);
    if(width > maxwidth)
        maxwidth = width;
    addButton->SetExplicitSize(BSize(maxwidth, height));
    editButton->SetExplicitSize(BSize(maxwidth, height));
    deleteButton->SetExplicitSize(BSize(maxwidth, height));
    openextButton->SetExplicitSize(BSize(maxwidth, height));
    updateButton->SetExplicitSize(BSize(maxwidth, height));
}

void AutolaunchView::MessageReceived(BMessage* msg)
{
    switch(msg->what)
    {
        case ALV_ADDMENU:
        {
            BRect rect = addButton->Bounds();
            BPoint point = addButton->ConvertToScreen(rect.LeftBottom());
            addMenu->Go(point, true);
            break;
        }
        case ALV_ADD_LINK:
        {
            BMessenger msgr(this);
            entry_ref ref;
            filepanel = new BFilePanel(B_OPEN_PANEL, &msgr, NULL, B_FILE_NODE,
                false, /*new BMessage('link')*/NULL, NULL, false, true);
            filepanel->Show();
            break;
        }
        case B_REFS_RECEIVED:
        {
            entry_ref ref;
            if(msg->FindRef("refs", &ref) == B_OK) {
                BEntry entry(&ref, true);
                BPath path;
                entry.GetPath(&path);
                BString origin(path.Path());
                if(autolaunch_create_link(origin) == B_OK)
                    Update();
            }
            break;
        }
        case ALV_ADD_FILE:
        {
            BMessenger msgr(this);
            copypanel = new BFilePanel(B_OPEN_PANEL, &msgr, NULL, B_FILE_NODE,
                false, new BMessage(ALV_MSG_COPY), NULL, false, true);
            copypanel->Show();
            break;
        }
        case ALV_MSG_COPY:
        {
            entry_ref ref;
            if(msg->FindRef("refs", &ref) == B_OK) {
                BEntry entry(&ref, true);
                BPath path;
                entry.GetPath(&path);
                BString origin(path.Path());
                if(autolaunch_copy_file(origin) == B_OK)
                    Update();
                else
                    fprintf(stderr, "Something went wrong with the copying procedure.\n");
            }
            break;
        }
        case ALV_DELETE_ENTRY:
        {
            if(alListView->CurrentSelection() == NULL) {
                BAlert *alert = new BAlert(B_TRANSLATE_COMMENT("Error", "This will not be visible"),
                    B_TRANSLATE("Error, an entry must be selected first."), B_TRANSLATE("Close"));
                alert->SetShortcut(0, B_ESCAPE);
                alert->Go();
                break;
            }
            else {
                BAlert *alert = new BAlert(B_TRANSLATE_COMMENT("Delete entry", "This will not be visible"),
                    B_TRANSLATE("Do you want to delete this entry?"), B_TRANSLATE("Cancel"),
                    B_TRANSLATE("Delete entry"));
                alert->SetShortcut(0, B_ESCAPE);
                int32 result = alert->Go();
                if(result == 1) {
                    BEntry entry(((BStringField*)alListView->CurrentSelection()->GetField(0))->String());
                    entry.Remove();
                    alListView->RemoveRow(alListView->CurrentSelection());
                    Update();
                }
            }
            break;
        }
        case ALV_EDIT_ENTRY:
        {
            if(editwindow == nullptr)
                EditEntryWindow *editwindow = new EditEntryWindow();
            editwindow->Show();
            break;
        }
        case ALV_ITEM_SELECTED:
        {
            if(alListView->CountRows() > 0)
                deleteButton->SetEnabled(true);
            else
                deleteButton->SetEnabled(false);
            break;
        }
        case ALV_NEW_SCRIPT:
        {
            if(scriptwindow == nullptr)
                scriptwindow = new NewScriptWindow(this);
            scriptwindow->Show();
            break;
        }
        case ALV_ITEM_UPDATE:
        {
            Update();
            break;
        }
        case ALV_OPEN_ENTRY:
        {
            launch("application/x-vnd.Be-directory", USER_AUTOLAUNCH_DIR);
            break;
        }
        default:
            BView::MessageReceived(msg);
            break;
    }
}

void AutolaunchView::Update()
{
    _list.clear();
    autolaunch_load(_list);
    alListView->Clear();
    _Init();
}

void AutolaunchView::RestoreDefault()
{
    autolaunch_clear_folder();
    Update();
}

#undef B_TRANSLATION_CONTEXT
#define B_TRANSLATION_CONTEXT "Autolaunch view: edit entry"

EditEntryWindow::EditEntryWindow()
: BWindow(BRect(100,100,400,400), B_TRANSLATE("Edit entry"), B_FLOATING_WINDOW, B_ASYNCHRONOUS_CONTROLS)
{
    entryName = new BTextControl("tc_name", "", new BMessage('enam'));
    entryType = new BTextControl("tc_type", "", new BMessage('etyp'));
    entryInfo = new BTextControl("tc_info", "", new BMessage('edat'));

    saveButton = new BButton("bt_save", B_TRANSLATE("Save"), new BMessage('save'));
    cancelButton = new BButton("bt_cancel", B_TRANSLATE("Cancel"), new BMessage('cncl'));

    BLayoutBuilder::Group<>(this, B_VERTICAL, 0)
        .AddGroup(B_VERTICAL)
            .SetInsets(B_USE_WINDOW_INSETS)
            .AddGrid(B_USE_DEFAULT_SPACING, B_USE_DEFAULT_SPACING, 1.0f)
                .AddTextControl(entryName, 0, 0)
                .AddTextControl(entryType, 0, 1)
                .AddTextControl(entryInfo, 0, 2)
            .End()
        .End()
        .Add(new BSeparatorView(B_HORIZONTAL))
        .AddGroup(B_HORIZONTAL)
            .SetInsets(B_USE_WINDOW_INSETS)
            .AddGlue()
            .Add(cancelButton)
            .Add(saveButton)
        .End()
    .End();

    CenterOnScreen();
}

void EditEntryWindow::MessageReceived(BMessage* msg)
{
    switch(msg->what)
    {
        case 'cncl':
            Quit();
            break;
    }
}

#undef B_TRANSLATION_CONTEXT
#define B_TRANSLATION_CONTEXT "Autolaunch view: add new script"

NewScriptWindow::NewScriptWindow(BView *parent)
: BWindow(BRect(100,100,400,400), B_TRANSLATE("Edit entry"), B_FLOATING_WINDOW, B_ASYNCHRONOUS_CONTROLS | B_CLOSE_ON_ESCAPE),
  _parent(parent)
{
    entryName = new BTextControl(NULL, "", new BMessage('enam'));
    shellPopUp = new BPopUpMenu("");
    shellPopUp->AddItem(new BMenuItem("/bin/bash", new BMessage('bash')));
    shellPopUp->AddItem(new BMenuItem("/bin/sh", new BMessage('sh  ')));
    shellMenu = new BMenuField("Select", NULL, shellPopUp);

    saveButton = new BButton("bt_create", B_TRANSLATE("Create"), new BMessage('save'));
    cancelButton = new BButton("bt_cancel", B_TRANSLATE("Cancel"), new BMessage('cncl'));

    BLayoutBuilder::Group<>(this, B_VERTICAL, 0)
        .AddGroup(B_VERTICAL)
            .SetInsets(B_USE_WINDOW_INSETS)
            .AddGrid(B_USE_DEFAULT_SPACING, B_USE_DEFAULT_SPACING, 1.0f)
                .Add(new BStringView(NULL, B_TRANSLATE("Name")), 0, 0)
                .Add(entryName, 1, 0)
                .Add(new BStringView(NULL, B_TRANSLATE("Shell")), 0, 1)
                .Add(shellMenu, 1, 1)
            .End()
        .End()
        .Add(new BSeparatorView(B_HORIZONTAL))
        .AddGroup(B_HORIZONTAL)
            .SetInsets(B_USE_WINDOW_INSETS)
            .AddGlue()
            .Add(cancelButton)
            .Add(saveButton)
        .End()
    .End();

    CenterOnScreen();
    ResizeToPreferred();
}

void NewScriptWindow::AttachedToWindow()
{
    saveButton->SetTarget(this);
}

void NewScriptWindow::MessageReceived(BMessage* msg)
{
    switch(msg->what)
    {
        case 'enam':
        {
            name = entryName->Text();
            break;
        }
        case 'bash':
        {
            shell = "/bin/bash";
            break;
        }
        case 'sh  ':
        {
            shell = "/bin/sh";
            break;
        }
        case 'save':
        {
            if(name.Length() > 0 && shell.Length() > 0) {
                status_t status = autolaunch_create_shell(name, shell);
                if(status == B_OK) {
                    BString scriptpath(USER_AUTOLAUNCH_DIR);
                    scriptpath.Append("/").Append(name).Append(".sh");

                    if(_parent != nullptr) {
                        BMessenger msgr(_parent, NULL);
                        BMessage mymsg(ALV_ITEM_UPDATE);
                        msgr.SendMessage(&mymsg);
                    }

                    char* args[] = { const_cast<char*>("app"), const_cast<char*>(scriptpath.String()) };
                    be_roster->Launch("text/x-source-code", 1, args+1);
                }
                else {
                    BAlert *alert = new BAlert(B_TRANSLATE_COMMENT("Error", "This will not be visible"),
                        B_TRANSLATE("The new script could not be created"), B_TRANSLATE("OK"), NULL, NULL,
                        B_WIDTH_AS_USUAL, B_STOP_ALERT);
                    alert->Go();
                }

                Quit();
            }
            else {
                BAlert *alert = new BAlert(B_TRANSLATE_COMMENT("Error", "This will not be visible"),
                    B_TRANSLATE("All fields are mandatory."), B_TRANSLATE("OK"), NULL, NULL,
                    B_WIDTH_AS_USUAL, B_WARNING_ALERT);
                alert->Go();
            }
            break;
        }
        case 'cncl':
            Quit();
            break;
    }
}
