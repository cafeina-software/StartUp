#include <AppKit.h>
#include <InterfaceKit.h>
#include <Catalog.h>
#include <cstdio>

#include "StartUpWin.h"
#include "AutolaunchView.h"
#include "StartUpApp.h"
#include "TextEditorView.h"
#include "EnvironmentView.h"

#undef B_TRANSLATION_CONTEXT
#define B_TRANSLATION_CONTEXT "Main window"

StartUpWin::StartUpWin()
: BWindow(BRect(0,0,1000,550), B_TRANSLATE_SYSTEM_NAME("StartUp"), B_TITLED_WINDOW,
    B_CLOSE_ON_ESCAPE | B_QUIT_ON_WINDOW_CLOSE | B_ASYNCHRONOUS_CONTROLS)
{
    struct tabsdata {
        const char* name;
        uint32 message;
    } tabs [] = {
        { B_TRANSLATE("Autolaunch"),          'tab0' },
        { B_TRANSLATE("Profile environment"), 'tab1' },
        { "UserBootscript",                   'tab2' },
        { "UserSetupEnvironment",             'tab3' }
    };

    SetSizeLimits(550, B_SIZE_UNLIMITED, 550, B_SIZE_UNLIMITED);

    BMenu* tabSelectionMenu = new BMenu(B_TRANSLATE("Go to"), B_ITEMS_IN_COLUMN);
    tabSelectionMenu->SetRadioMode(true);

    optionsMenu = new BPopUpMenu(B_TRANSLATE("Options"), false, false, B_ITEMS_IN_COLUMN);
    optionsMenu->AddItem(tabSelectionMenu);
    optionsMenu->AddSeparatorItem();
    optionsMenu->AddItem(new BMenuItem(B_TRANSLATE("Open BootManager"), new BMessage('boot')));

    optionsMenuField = new BMenuField("mf_opt", NULL, optionsMenu, false);

    alview = new AutolaunchView(tabs[0].name, ((StartUpApp*)be_app)->CurrentALList());
    termenvview = new TextEditorView(tabs[1].name, ((StartUpApp*)be_app)->CurrentProfileEnv(),USER_PROF_ENV);
    ubsview = new TextEditorView(tabs[2].name, ((StartUpApp*)be_app)->CurrentUBS(), USER_UBS);
    envview = new EnvironmentView(tabs[3].name, ((StartUpApp*)be_app)->CurrentUserEnv());

    tabView = new BTabView("tabs", B_WIDTH_FROM_LABEL);
    tabView->SetBorder(B_NO_BORDER);
    tabView->AddTab(alview);
    tabView->AddTab(termenvview);
    tabView->AddTab(ubsview);
    if(((StartUpApp*)be_app)->UnstableFeaturesEnabled())
        tabView->AddTab(envview);

    for (int tabindex = 0; tabindex < tabView->CountTabs(); tabindex++)
        tabSelectionMenu->AddItem(new BMenuItem(tabs[tabindex].name, new BMessage(tabs[tabindex].message)));

    BLayoutBuilder::Group<>(this, B_VERTICAL, 0)
        .AddGroup(B_VERTICAL)
            .SetInsets(0, B_USE_WINDOW_INSETS)
            .Add(tabView)
        .End()
        .Add(new BSeparatorView(B_HORIZONTAL))
        .AddGroup(B_HORIZONTAL)
            .SetInsets(B_USE_WINDOW_SPACING, B_USE_DEFAULT_SPACING, B_USE_DEFAULT_SPACING, B_USE_WINDOW_SPACING)
            .Add(new BButton(NULL, B_TRANSLATE("About" B_UTF8_ELLIPSIS), new BMessage('abtt')))
            .Add(optionsMenuField)
            .AddGlue().AddGlue()
            .Add(new BButton(NULL, B_TRANSLATE("Close"), new BMessage('clse')))
        .End()
    .End();

    CenterOnScreen();
}

void StartUpWin::MessageReceived(BMessage* message)
{
	switch (message->what)
    {
        case 'abtt':
            be_app->PostMessage(B_ABOUT_REQUESTED);
            break;
        case 'clse':
            be_app->PostMessage(B_QUIT_REQUESTED);
            break;
        case 'boot':
            launch("application/x-vnd.Haiku-BootManager", NULL);
            break;
        case 'tab0':
            if(tabView->CountTabs() > 0)
                tabView->Select(0);
            break;
        case 'tab1':
            if(tabView->CountTabs() > 1)
                tabView->Select(1);
            break;
        case 'tab2':
            if(tabView->CountTabs() > 2)
                tabView->Select(2);
            break;
        case 'tab3':
            if(tabView->CountTabs() > 3)
                tabView->Select(3);
            break;
		default:
		{
			BWindow::MessageReceived(message);
			break;
		}
	}
}
