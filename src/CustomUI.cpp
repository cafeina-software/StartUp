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
