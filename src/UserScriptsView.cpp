#include <Catalog.h>
#include "UserScriptsView.h"
#include "DataLoader.h"
#include "TextEditorView.h"

#undef B_TRANSLATION_CONTEXT
#define B_TRANSLATION_CONTEXT "User scripts view"

UserScriptsView::UserScriptsView(const char* title, BString _ubs, BString _uss, BString _usf, BString _use)
: BView(BRect(), title, B_FOLLOW_ALL, B_ASYNCHRONOUS_CONTROLS),
  ubs(_ubs), uss(_uss), usf(_usf), use(_use)
{
    fListView = new BListView("usv_list");
    fListView->SetSelectionMessage(new BMessage(US_ITEM_SELECTED));
    fListView->SetExplicitMinSize(BSize(fListView->StringWidth("UserShutdownFinishScript"),
        B_SIZE_UNSET));
    fListView->AddItem(new BStringItem("UserBootscript"));
    fListView->AddItem(new BStringItem("UserShutdownScript"));
    fListView->AddItem(new BStringItem("UserShutdownFinishScript"));
    fListView->AddItem(new BStringItem("UserSetupEnvironment"));
    BScrollView *listScrollView = new BScrollView("sc_usv", fListView,
        0, false, true, B_FANCY_BORDER);
    fListView->SetExplicitMinSize(BSize(fListView->StringWidth("UserShutdownFinishScript"),
        B_SIZE_UNSET));

    TextEditorView *ubsview = new TextEditorView("UserBootscript", ubs, "UserBootscript");
    TextEditorView *ussview = new TextEditorView("UserShutdownScript", uss, "UserShutdownScript");
    TextEditorView *usfview = new TextEditorView("UserShutdownFinishScript", usf, "UserShutdownFinishScript");
    TextEditorView *useview = new TextEditorView("UserSetupEnvironment", use, "UserSetupEnvironment");

    fCardView = new BCardView();
    fCardView->AddChild(ubsview);
    fCardView->AddChild(ussview);
    fCardView->AddChild(usfview);
    fCardView->AddChild(useview);

    BLayoutBuilder::Group<>(this, B_HORIZONTAL)
        .Add(listScrollView, 1)
        .Add(fCardView, 3)
    .End();
}

void UserScriptsView::AttachedToWindow()
{
    fListView->SetTarget(this);
}

void UserScriptsView::MessageReceived(BMessage* msg)
{
    switch(msg->what)
    {
        case US_ITEM_SELECTED:
        {
            int32 index;
            if(msg->FindInt32("index", &index) == B_OK)
                if(index >= 0)
                    fCardView->CardLayout()->SetVisibleItem(index);
            break;
        }
        default:
            BView::MessageReceived(msg);
            break;
    }
}
