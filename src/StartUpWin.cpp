#include <AppKit.h>
#include <InterfaceKit.h>
#include <Url.h>
#include <Catalog.h>
#include <cstdio>

#include "StartUpWin.h"
#include "AutolaunchView.h"
#include "KernelSettingsView.h"
#include "StartUpApp.h"
#include "UserScriptsView.h"
#include "TextEditorView.h"
#include "CustomUI.h"
// #include "EnvironmentView.h"

#undef B_TRANSLATION_CONTEXT
#define B_TRANSLATION_CONTEXT "Main window"

StartUpWin::StartUpWin()
: BWindow(BRect(50,50,1000,600), B_TRANSLATE_SYSTEM_NAME("StartUp"), B_TITLED_WINDOW,
    B_CLOSE_ON_ESCAPE | B_QUIT_ON_WINDOW_CLOSE | B_ASYNCHRONOUS_CONTROLS),
    kswarningshown(false)
{
    struct tabsdata {
        const char* name;
        uint32 message;
        uint32 id;
    } tabs [] = {
        { B_TRANSLATE("Autolaunch"),           'tab0', 0 },
        { B_TRANSLATE("Terminal environment"), 'tab1', 1 },
        { B_TRANSLATE("User scripts"),         'tab2', 2 },
        { B_TRANSLATE("Kernel settings"),      'tab3', 3 }
    };
    SetSizeLimits(550, B_SIZE_UNLIMITED, 550, B_SIZE_UNLIMITED);

    BMenu* tabSelectionMenu = new BMenu(B_TRANSLATE("Go to"), B_ITEMS_IN_COLUMN);
    tabSelectionMenu->SetRadioMode(true);

    helpMenu = new BMenu(B_TRANSLATE("Help topics"), B_ITEMS_IN_COLUMN);
    helpMenu->AddItem(new BMenuItem(B_TRANSLATE("Bash and scripting"), new BMessage('hbsh')));
    helpMenu->AddItem(new BMenuItem(B_TRANSLATE("Boot Loader"), new BMessage('hboo')));
    helpMenu->AddItem(new BMenuItem(B_TRANSLATE("Disabling components of packages"), new BMessage('hdis')));

    settingsMenu = new BMenu(B_TRANSLATE("Settings"), B_ITEMS_IN_COLUMN);
    settingsMenu->AddItem(new BMenuItem(B_TRANSLATE("Do not warn about kernel settings"), new BMessage('diss')));

    optionsMenu = new BPopUpMenu(B_TRANSLATE("Options"), false, false, B_ITEMS_IN_COLUMN);
    optionsMenu->AddItem(tabSelectionMenu);
    optionsMenu->AddItem(helpMenu);
    optionsMenu->AddItem(settingsMenu);
    optionsMenu->AddItem(new BMenuItem(B_TRANSLATE("About" B_UTF8_ELLIPSIS), new BMessage('abtt')));
    optionsMenu->AddSeparatorItem();
    optionsMenu->AddItem(new BMenuItem(B_TRANSLATE("Open BootManager"), new BMessage('boot')));

    optionsMenuField = new BMenuField("mf_opt", NULL, optionsMenu, false);

    alview = new AutolaunchView(tabs[0].name, ((StartUpApp*)be_app)->CurrentALList());
    termenvview = new TextEditorView(tabs[1].name, ((StartUpApp*)be_app)->CurrentProfileEnv(),USER_PROF_ENV);
    usview = new UserScriptsView(tabs[2].name, ((StartUpApp*)be_app)->CurrentUBS(),
        ((StartUpApp*)be_app)->CurrentUSS(), ((StartUpApp*)be_app)->CurrentUSF(), ((StartUpApp*)be_app)->CurrentUserEnv());
    kernview = new KernelSettingsView(tabs[3].name, ((StartUpApp*)be_app)->CurrentKernelSettings());

    tabView = new BTabView("tabs", B_WIDTH_FROM_LABEL);
    tabView->SetBorder(B_NO_BORDER);
    tabView->AddTab(alview, new MyTab(tabs[0].id));
    tabView->AddTab(termenvview, new MyTab(tabs[1].id));
    tabView->AddTab(usview, new MyTab(tabs[2].id));
    tabView->AddTab(kernview, new MyTab(tabs[3].id));

    for(int tabindex = 0; tabindex < tabView->CountTabs(); tabindex++)
        tabSelectionMenu->AddItem(new BMenuItem(tabs[tabindex].name, new BMessage(tabs[tabindex].message)));
    if(tabSelectionMenu->CountItems() > 0)
        tabSelectionMenu->ItemAt(0)->SetMarked(true);

    BLayoutBuilder::Group<>(this, B_VERTICAL, 0)
        .AddGroup(B_VERTICAL)
            .SetInsets(0, B_USE_WINDOW_INSETS)
            .Add(tabView)
        .End()
        .Add(new BSeparatorView(B_HORIZONTAL))
        .AddGroup(B_HORIZONTAL)
            .SetInsets(B_USE_WINDOW_SPACING, B_USE_DEFAULT_SPACING, B_USE_DEFAULT_SPACING, B_USE_WINDOW_SPACING)
            .Add(optionsMenuField)
            .AddGlue()
            .Add(new BButton(NULL, B_TRANSLATE("Close"), new BMessage('clse')))
        .End()
    .End();
}

void StartUpWin::MessageReceived(BMessage* message)
{
	switch (message->what)
    {
        case TAB_SELECTED:
        {
            uint32 id;
            if(message->FindUInt32("id", &id) == B_OK) {
                if(id == 3 && !kswarningshown) {
                    BAlert *alert = new BAlert(B_TRANSLATE("Kernel configuration"),
                        B_TRANSLATE(
                        "Some of the options in this sections could make your system "
                        "unable to boot due to hardware compatibility issues.\n\n"
                        "If a problem during boot occurs after you changed any "
                        "configuration options here, you can temporarily boot into "
                        "a good configuration by using the options in the Haiku "
                        "boot loader and once the system and the Desktop are loaded, "
                        "open this application and use the button \"Restore\" "
                        "of the \"Kernel settings\" view."),
                        B_TRANSLATE("Understood"), NULL, NULL, B_WIDTH_AS_USUAL,
                        B_WARNING_ALERT);
                    int32 result = alert->Go();

                    if(result == 0) {
                        kswarningshown = true;
                        settingsMenu->ItemAt(0)->SetMarked(kswarningshown);
                    }
                }
            }
            break;
        }
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
        case 'hbsh':
            OpenDocumentation("bash-scripting");
            break;
        case 'hboo':
            OpenDocumentation("bootloader");
            break;
        case 'hdis':
            launch("application/x-vnd.Be.URL.https",
                "https://www.haiku-os.org/guides/daily-tasks/disable-package-entries");
            break;
        case 'diss':
            kswarningshown = !kswarningshown;
            settingsMenu->ItemAt(0)->SetMarked(kswarningshown);
            break;
	    default:
		{
			BWindow::MessageReceived(message);
			break;
		}
	}
}

status_t StartUpWin::LoadSettings(BMessage* indata)
{
    BRect rect;
    if(indata->FindRect("pos", &rect) == B_OK) {
        MoveTo(rect.LeftTop());
        ResizeTo(rect.Width(), rect.Height());
    }

    if(indata->FindBool("ui.dismissKSWarning", &kswarningshown) == B_OK)
        settingsMenu->ItemAt(0)->SetMarked(kswarningshown);

    return B_OK;
}

status_t StartUpWin::SaveSettings(BMessage* outdata)
{
    if(outdata->ReplaceRect("pos", Frame ()) != B_OK)
        outdata->AddRect("pos", Frame());
    if(outdata->ReplaceBool("ui.dismissKSWarning", kswarningshown) != B_OK)
        outdata->AddBool("ui.dismissKSWarning", kswarningshown);

    return B_OK;
}

// pragma mark Private methods

void StartUpWin::OpenDocumentation(const char* what)
{
    BString localdoc("file:///boot/system/documentation/userguide/%lang%/%doc%.html");
    localdoc.ReplaceAll("%lang%", B_TRANSLATE_COMMENT("en",
        "This is the language code of the local user guide. "
        "If there is no translation, leave the English \"en\" default "
        "or use the language code of the closest language to yours."));
    localdoc.ReplaceAll("%doc%", what);

    BString webdoc("https://www.haiku-os.org/docs/userguide/%lang%/%doc%.html");
    webdoc.ReplaceAll("%lang%", B_TRANSLATE_COMMENT("en",
        "This is the language code of the local user guide. "
        "If there is no translation, leave the English \"en\" default "
        "or use the language code of the closest language to yours."));
    webdoc.ReplaceAll("%doc%", what);

    if(BUrl(localdoc.String()).OpenWithPreferredApplication(false) != B_OK)
        if(BUrl(webdoc.String()).OpenWithPreferredApplication(false) != B_OK)
            fprintf(stderr, "Error. It was not possible to open the local or remote documentation.\n");
}
