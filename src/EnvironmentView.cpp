#include <Catalog.h>
#include <InterfaceKit.h>
#include <Roster.h>
#include <cstdio>
#include <private/interface/ColumnTypes.h>
#include "EnvironmentView.h"

#undef B_TRANSLATION_CONTEXT
#define B_TRANSLATION_CONTEXT "Environment view"

EnvironmentView::EnvironmentView(const char* title, std::vector<entry> entrylist)
: BView(BRect(0, 0, 10, 10), title, B_FOLLOW_ALL, B_ASYNCHRONOUS_CONTROLS),
  _list(entrylist),
  current(nullptr)
{
    environmentView = new BColumnListView("env", 0);
    environmentView->SetSelectionMode(B_SINGLE_SELECTION_LIST);
    environmentView->SetSelectionMessage(new BMessage(ENV_ITEM_SELECTED));
    // environmentView->AddColumn(new BCheckBoxColumn(B_TRANSLATE("Status"), 100, 100, 100), 0);
    environmentView->AddColumn(new BStringColumn(B_TRANSLATE("Status"), 150, 75, 250, 0), 0);
    environmentView->AddColumn(new BStringColumn(B_TRANSLATE("Name"), 150, 75, 250, 0), 1);
    environmentView->AddColumn(new BStringColumn(B_TRANSLATE("Value"), 150, 75, 450, 0), 2);

    _Init();

    environmentView->ResizeAllColumnsToPreferred();

    addButton = new BButton("bt_add", B_TRANSLATE("Add"), new BMessage(ENV_ITEM_ADD));
    editButton = new BButton(NULL, B_TRANSLATE("Edit"), new BMessage(ENV_ITEM_EDIT));
    deleteButton = new BButton(NULL, B_TRANSLATE("Delete"), new BMessage(ENV_ITEM_DELETE));
    updateButton = new BButton(NULL, B_TRANSLATE("Update"), new BMessage(ENV_ITEMLIST_UPDATE));
    resetButton = new BButton(NULL, B_TRANSLATE("Default"), new BMessage(ENV_ITEM_DEFAULT));
    openextButton = new BButton(NULL, B_TRANSLATE("Open file"), new BMessage(ENV_OPEN_EXT));

    BLayoutBuilder::Group<>(this, B_HORIZONTAL)
        .Add(environmentView)
        .AddGroup(B_VERTICAL)
            .SetInsets(0, 0, B_USE_DEFAULT_SPACING, 0)
            .Add(addButton)
            .Add(editButton)
            .Add(deleteButton)
            .AddStrut(10.0f)
            .Add(updateButton)
            .AddStrut(10.0f)
            .Add(resetButton)
            .AddStrut(10.0f)
            .AddGlue()
            .Add(openextButton)
        .End()
    .End();
}

EnvironmentView::~EnvironmentView()
{
    if(addenvWindow && addenvWindow->Lock())
        addenvWindow->Quit();
    if(edenvWindow && edenvWindow->Lock())
        edenvWindow->Quit();
}

void EnvironmentView::_Init()
{
    if(_list.empty()) {
        fprintf(stderr, "List is empty.\n");
        return;
    }

    for (const auto& entry : _list) {
        BString enabled(entry.enabled ? B_TRANSLATE("Enabled") : B_TRANSLATE("Disabled"));

        BRow* row = new BRow();
        // row->SetField(new BCheckBoxField(false), 0);
        row->SetField(new BStringField(enabled), 0);
        row->SetField(new BStringField(entry.key), 1);
        row->SetField(new BStringField(entry.value), 2);
        environmentView->AddRow(row);
    }
}

void EnvironmentView::AttachedToWindow()
{
    addButton->SetTarget(this);
    editButton->SetTarget(this);
    editButton->SetEnabled(false);
    deleteButton->SetTarget(this);
    updateButton->SetTarget(this);
    openextButton->SetTarget(this);
    resetButton->SetEnabled(false);

    float width, maxwidth, height;
    addButton->GetPreferredSize(&width, &height);
    maxwidth = width;
    editButton->GetPreferredSize(&width, NULL);
    if(width > maxwidth)
        maxwidth = width;
    deleteButton->GetPreferredSize(&width, NULL);
    if(width > maxwidth)
        maxwidth = width;
    resetButton->GetPreferredSize(&width, NULL);
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
    resetButton->SetExplicitSize(BSize(maxwidth, height));
    updateButton->SetExplicitSize(BSize(maxwidth, height));
}

void EnvironmentView::MessageReceived(BMessage* msg)
{
    switch(msg->what)
    {
        case ENV_ITEM_SELECTED:
        {
            current = environmentView->CurrentSelection();
            break;
        }
        case ENV_ITEM_ADD:
        {
            if(addenvWindow == nullptr)
                addenvWindow = new EditEnvWindow(this, &_list);
            addenvWindow->Show();
            break;
        }
        case ENV_ITEM_EDIT:
        {
            current = environmentView->CurrentSelection();

            if(edenvWindow)
                delete edenvWindow;
            edenvWindow = new EditEnvWindow(this, &_list, current);
            edenvWindow->Show();
            break;
        }
        case ENV_ITEM_DELETE:
        {
            current = environmentView->CurrentSelection();
            if(current == NULL) {
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
                    BString targetkey = ((BStringField*)environmentView->CurrentSelection()->GetField(1))->String();
                    // status_t status = delete_entry_usersetupenvironment(targetkey, _list);
                    // if(status != B_OK)
                        // fprintf(stderr, "Error: could not be deleted or something went wrong.\n");
                    environmentView->RemoveRow(current);
                    _Update();
                }
            }
            break;
        }
        case ENV_ITEMLIST_UPDATE:
        {
            _Update();
            break;
        }
        case ENV_ITEM_DEFAULT:
        {
            break;
        }
        case ENV_OPEN_EXT:
        {
            // if(!exists(USER_USENV)) {
                // BAlert *alert = new BAlert(B_TRANSLATE_COMMENT("Entry missing", "This will not be visible"),
                        // B_TRANSLATE("This file currently does not exist. Do you want to create it?"),
                        // B_TRANSLATE("Do not create"), B_TRANSLATE("Create"));
                // alert->SetShortcut(0, B_ESCAPE);
                // int32 result = alert->Go();
                // if(result == 0)
                    // break;
                // status_t status = create_usersetupenvironment();
                // if(status != B_OK)
                    // fprintf(stderr, "Error: file USE could not be created.\n");
            // }
//
            // launch("text/x-source-code", USER_USENV);
            break;
        }
        default:
            BView::MessageReceived(msg);
            break;
    }
}

void EnvironmentView::_Update()
{
    _list.clear();
    // load_usersetupenvironment(_list);
    environmentView->Clear();
    _Init();
    environmentView->ResizeAllColumnsToPreferred();
}

#undef B_TRANSLATION_CONTEXT
#define B_TRANSLATION_CONTEXT "Autolaunch view: edit entry"

EditEnvWindow::EditEnvWindow(BView* parent, std::vector<entry>* plist, BRow* target)
: BWindow(BRect(100,100,400,400), "", B_FLOATING_WINDOW, B_ASYNCHRONOUS_CONTROLS),
  _parent(parent), _target(target), _plist(plist)
{
    if(_target != nullptr) {
        enabled = (strcmp(((BStringField*)_target->GetField(0))->String(), B_TRANSLATE("Enabled")) == 0 ? true : false);
        key = ((BStringField*)_target->GetField(1))->String();
        value = ((BStringField*)_target->GetField(2))->String();
        fprintf(stderr, "Key: %s. Value: %s.\n", key.String(), value.String());

        this->SetTitle(B_TRANSLATE("Edit entry"));
    }
    else {
        enabled = true;
        key = "";
        value = "";
        fprintf(stderr, "Key: %s. Value: %s.\n", key.String(), value.String());

        this->SetTitle(B_TRANSLATE("Add entry"));
    }

    entryKey = new BTextControl(NULL, key, new BMessage('ekey'));
    entryValue = new BTextControl(NULL, value, new BMessage('eval'));

    isEnabled = new BCheckBox(B_TRANSLATE("Enabled"), new BMessage('enab'));
    isEnabled->SetValue(enabled ? B_CONTROL_ON : B_CONTROL_OFF);

    saveButton = new BButton("bt_save", B_TRANSLATE("Save"), new BMessage('save'));
    cancelButton = new BButton("bt_cancel", B_TRANSLATE("Cancel"), new BMessage('cncl'));

    BLayoutBuilder::Group<>(this, B_VERTICAL, 0)
        .AddGroup(B_VERTICAL)
            .SetInsets(B_USE_WINDOW_INSETS)
            .AddGrid(B_USE_DEFAULT_SPACING, B_USE_DEFAULT_SPACING, 1.0f)
                .Add(new BStringView(NULL, B_TRANSLATE("Key")), 0, 0)
                .Add(entryKey, 1, 0)
                .Add(new BStringView(NULL, B_TRANSLATE("Value")), 0, 1)
                .Add(entryValue, 1, 1)
            .End()
            .Add(isEnabled)
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

void EditEnvWindow::AttachedToWindow()
{
    saveButton->SetTarget(this);
    cancelButton->SetTarget(this);
}

void EditEnvWindow::MessageReceived(BMessage* msg)
{
    switch(msg->what)
    {
        case 'ekey':
            key = entryKey->Text();
            break;
        case 'eval':
            value = entryValue->Text();
            break;
        case 'enab':
            enabled = (isEnabled->Value() == B_CONTROL_ON ? true : false);
            break;
        case 'save':
        {
            if(key.Length() > 0 && value.Length() > 0) {
                entry newentry;
                newentry.enabled = enabled;
                newentry.key = key;
                newentry.value = value;

                if(_target == NULL) {
                    // status_t status = add_to_usersetupenvironment(newentry, *_plist);
                    // if (status != B_OK)
                        // fprintf(stderr, "Error: could not be added.\n");
                }
                else {
                    // status_t status = edit_entry_usersetupenvironment(key, newentry, *_plist);
                    // if (status != B_OK)
                        // fprintf(stderr, "Error: could not be set.\n");

                }

                if(_parent != nullptr) {
                    BMessenger msgr(_parent, NULL);
                    BMessage mymsg(ENV_ITEMLIST_UPDATE);
                    msgr.SendMessage(&mymsg);
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
