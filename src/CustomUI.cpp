#include "CustomUI.h"
#include <InterfaceKit.h>

// #pragma mark Custom tab implementation

MyTab::MyTab(uint32 id, BView* contentsView)
: BTab(contentsView), identificador(id)
{
}

void MyTab::Select(BView* owner)
{
    BTab::Select(owner);

    BMessage msg(TAB_SELECTED);
    msg.AddUInt32("id", Id());
    owner->Window()->PostMessage(&msg);
}

uint32 MyTab::Id()
{
    return identificador;
}

// #pragma mark Custom BOutlineListView implementation

MyOutlineListView::MyOutlineListView(const char* name, list_view_type type,
    BPopUpMenu* _menu, uint32 flags)
: BOutlineListView(name, type, flags), menu(_menu)
{
}

void MyOutlineListView::MouseDown(BPoint where)
{
    BOutlineListView::MouseDown(where);

    uint32 buttons;
    GetMouse(&where, &buttons, true);

    if(buttons == B_SECONDARY_MOUSE_BUTTON)
    {
        int32 index = IndexOf(where);

        if(index >= 0) {
            dynamic_cast<IconStringItem*>(ItemAt(index))->MouseDown(this, where, buttons,
                (Superitem(ItemAt(index)) == NULL), ItemAt(index)->IsExpanded());
        }
        else {
            if(menu) {
                Parent()->ConvertToScreen(&where);
                menu->Go(where, true, true, true);
            }
        }
    }

}

void MyOutlineListView::MessageReceived(BMessage* msg)
{
    BOutlineListView::MessageReceived(msg);
}

// #pragma mark Custom BStringItem implementation

IconStringItem::IconStringItem(const char* text, BBitmap* icon, BPopUpMenu* _menu,
    BView* owner, uint32 outlineLevel, bool expanded, BString _extra)
: BStringItem(text, outlineLevel, expanded), itemicon(icon), menu(_menu),
  parent(owner), extratext(_extra)
{
}

void IconStringItem::DrawItem(BView* owner, BRect frame, bool complete)
{
    if(Text() == NULL)
        return;

    if(IsSelected()) {
        owner->SetLowColor(ui_color(B_LIST_SELECTED_BACKGROUND_COLOR));
        owner->SetHighColor(ui_color(B_LIST_SELECTED_ITEM_TEXT_COLOR));
    }
    else {
        owner->SetLowColor(ui_color(B_LIST_BACKGROUND_COLOR));
        owner->SetHighColor(ui_color(B_LIST_ITEM_TEXT_COLOR));
    }

    owner->FillRect(frame, B_SOLID_LOW);

    float padding = be_control_look->DefaultLabelSpacing();
    BPoint point(frame.left + padding, frame.top);
    owner->MovePenTo(point);
    if(itemicon != nullptr) {
        owner->SetDrawingMode(B_OP_ALPHA);
        owner->DrawBitmapAsync(itemicon, point);
        point = BPoint(frame.left + itemicon->Bounds().right + 4.0f + padding,
            frame.top + BaselineOffset());
    }
    else
        point = BPoint(frame.left + 4.0f + padding, frame.top + BaselineOffset());
    owner->MovePenTo(point);
    owner->SetDrawingMode(B_OP_COPY);
    owner->DrawString(Text(), point);
}

void IconStringItem::MouseDown(BOutlineListView* owner, BPoint where,
    uint32 buttons, bool issuper, bool isexpanded)
{
    if (menu != nullptr && buttons == B_SECONDARY_MOUSE_BUTTON)
    {
        owner->ConvertToScreen(&where);
        menu->Go(where, true, true, true);
    }
}

void IconStringItem::SetIcon(BBitmap* icon)
{
    itemicon = icon;
}

BBitmap* IconStringItem::Icon()
{
    return itemicon;
}

void IconStringItem::SetParentPtr(BView* owner)
{
    parent = owner;
}

BView* IconStringItem::ParentPtr()
{
    return parent;
}

void IconStringItem::SetExtraText(BString text)
{
    extratext = text;
}

BString* IconStringItem::ExtraText()
{
    return &extratext;
}
