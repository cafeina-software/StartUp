#include <AppKit.h>
#include <InterfaceKit.h>
#include <Url.h>
#include <Catalog.h>
#include <cstdio>

#include "StartUpWin.h"
#include "StartUpDefs.h"
#include "AutolaunchView.h"
#include "KernelSettingsView.h"
#include "StartUpApp.h"
#include "UserScriptsView.h"
#include "TextEditorView.h"
#include "CustomUI.h"

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
        { B_TRANSLATE("Autolaunch"),           M_VIEW_AUTOLAUNCH,  0 },
        { B_TRANSLATE("Terminal environment"), M_VIEW_TERMPROFILE, 1 },
        { B_TRANSLATE("User scripts"),         M_VIEW_USERSCRIPTS, 2 },
        { B_TRANSLATE("Kernel settings"),      M_VIEW_KERNELSET,   3 },
        { B_TRANSLATE("Blacklist"),            M_VIEW_BLACKLIST,   4 }
    };
    SetSizeLimits(550, B_SIZE_UNLIMITED, 550, B_SIZE_UNLIMITED);

    optionsMenu = BuildMenu();

    alview = new AutolaunchView(tabs[0].name, ((StartUpApp*)be_app)->CurrentALList());
    termenvview = new TextEditorView(tabs[1].name, ((StartUpApp*)be_app)->CurrentProfileEnv(), USER_PROF_ENV);
    usview = new UserScriptsView(tabs[2].name, ((StartUpApp*)be_app)->CurrentUBS(),
        ((StartUpApp*)be_app)->CurrentUSS(), ((StartUpApp*)be_app)->CurrentUSF(), ((StartUpApp*)be_app)->CurrentUserEnv());
    kernview = new KernelSettingsView(tabs[3].name, ((StartUpApp*)be_app)->CurrentKernelSettings());
    blview = new BlacklistView(tabs[4].name);

    StartMonitoring();

    tabView = new BTabView("tabs", B_WIDTH_FROM_LABEL);
    tabView->SetBorder(B_NO_BORDER);
    tabView->AddTab(alview, new MyTab(tabs[0].id));
    tabView->AddTab(termenvview, new MyTab(tabs[1].id));
    tabView->AddTab(usview, new MyTab(tabs[2].id));
    tabView->AddTab(kernview, new MyTab(tabs[3].id));
    tabView->AddTab(blview, new MyTab(tabs[4].id));

    for(int tabindex = 0; tabindex < tabView->CountTabs(); tabindex++)
        tabSelectionMenu->AddItem(new BMenuItem(tabs[tabindex].name,
            new BMessage(tabs[tabindex].message), static_cast<char>(tabindex + 49)));
    if(tabSelectionMenu->CountItems() > 0)
        tabSelectionMenu->ItemAt(0)->SetMarked(true);

    BLayoutBuilder::Group<>(this, B_VERTICAL, 0)
        .Add(optionsMenu)
        .AddGroup(B_VERTICAL)
            .SetInsets(0, B_USE_WINDOW_INSETS)
            .Add(tabView)
        .End()
        .Add(new BSeparatorView(B_HORIZONTAL))
        .AddGroup(B_HORIZONTAL)
            .SetInsets(B_USE_WINDOW_SPACING, B_USE_DEFAULT_SPACING,
                B_USE_DEFAULT_SPACING, B_USE_WINDOW_SPACING)
            .AddGlue()
            .Add(new BButton(NULL, B_TRANSLATE("Close"), new BMessage('clse')))
        .End()
    .End();
}

StartUpWin::~StartUpWin()
{
	StopMonitoring();
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
                        "Some of the options in this section could make your system "
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
                        optionsMenu->FindItem(B_TRANSLATE("Do not warn about kernel settings"))->SetMarked(kswarningshown);
                    }
                }

                optionsMenu->SubmenuAt(optionsMenu->IndexOf(tabSelectionMenu))->ItemAt(id)->SetMarked(true);
            }
            break;
        }
        case M_ABOUT_REQUESTED:
            be_app->PostMessage(B_ABOUT_REQUESTED);
            break;
        case M_QUIT_REQUESTED:
            be_app->PostMessage(B_QUIT_REQUESTED);
            break;
        case M_LAUNCH_BOOTMAN:
            launch("application/x-vnd.Haiku-BootManager", NULL);
            break;
        case M_LAUNCH_VMEM:
            launch("application/x-vnd.Haiku-VirtualMemory", NULL);
            break;
        case M_VIEW_AUTOLAUNCH:
            if(tabView->CountTabs() > 0)
                tabView->Select(0);
            break;
        case M_VIEW_TERMPROFILE:
            if(tabView->CountTabs() > 1)
                tabView->Select(1);
            break;
        case M_VIEW_USERSCRIPTS:
            if(tabView->CountTabs() > 2)
                tabView->Select(2);
            break;
        case M_VIEW_KERNELSET:
            if(tabView->CountTabs() > 3)
                tabView->Select(3);
            break;
        case M_VIEW_BLACKLIST:
            if(tabView->CountTabs() > 4)
                tabView->Select(4);
            break;
        case M_HELP_DOCS:
            if(launch("text/html", "./index.html") != B_OK) {
                BAlert* alert = new BAlert(B_TRANSLATE("Documentation: error"),
                    B_TRANSLATE("The documentation files could not be found.\n"),
                    B_TRANSLATE("OK"));
                alert->Go();
            }
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
        case M_SETTINGS_KSWARN:
            kswarningshown = !kswarningshown;
            optionsMenu->FindItem(B_TRANSLATE("Do not warn about kernel settings"))->SetMarked(kswarningshown);
            break;
        case M_RESTORE_DEFAULT:
            RestoreAllConfigs();
            break;
        case B_NODE_MONITOR:
        	NodeMonitor(message);
        	break;
	    default:
		{
			BWindow::MessageReceived(message);
			break;
		}
	}
}

void StartUpWin::StartMonitoring()
{
    LoadWatcher(USER_AUTOLAUNCH_DIR, false);
    LoadWatcher("/boot/home/config/settings", false);
    LoadWatcher("/boot/home/config/settings/boot", false);
    LoadWatcher("/boot/home/config/settings/kernel/drivers", false);
    LoadWatcher("/boot/system/settings", false);

    LoadWatcher(USER_PROF_ENV);
    LoadWatcher(USER_BOOT_SCRIPT);
    LoadWatcher(USER_SHUTDOWN_SCRIPT);
    LoadWatcher(USER_SHUTDOWN_FINISH_SCRIPT);
    LoadWatcher(USER_SETUP_ENVIRONMENT);
    LoadWatcher(KERNEL_SETTINGS);
    LoadWatcher(BLACKLIST_SYSTEM);
    LoadWatcher(BLACKLIST_USER);

    for(const auto& it : cached_values)
        __trace("Cached: file(%s), device(%d), node(%ld)\n", it.first, it.second.device, it.second.node);
}

void StartUpWin::StopMonitoring()
{
    UnloadWatcher(USER_AUTOLAUNCH_DIR);
    UnloadWatcher(USER_PROF_ENV);
    UnloadWatcher(USER_BOOT_SCRIPT);
    UnloadWatcher(USER_SHUTDOWN_SCRIPT);
    UnloadWatcher(USER_SHUTDOWN_FINISH_SCRIPT);
    UnloadWatcher(USER_SETUP_ENVIRONMENT);
    UnloadWatcher(KERNEL_SETTINGS);
    UnloadWatcher(BLACKLIST_SYSTEM);
    UnloadWatcher(BLACKLIST_USER);

    UnloadWatcher("/boot/home/config/settings");
    UnloadWatcher("/boot/home/config/settings/boot");
    UnloadWatcher("/boot/home/config/settings/kernel/drivers");
    UnloadWatcher("/boot/system/settings");

    stop_watching(this);
}

status_t StartUpWin::LoadSettings(BMessage* indata)
{
    BRect rect;
    if(indata->FindRect("pos", &rect) == B_OK) {
        MoveTo(rect.LeftTop());
        ResizeTo(rect.Width(), rect.Height());
    }

    if(indata->FindBool("ui.dismissKSWarning", &kswarningshown) == B_OK)
        optionsMenu->FindItem(B_TRANSLATE("Do not warn about kernel settings"))->SetMarked(kswarningshown);

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

void StartUpWin::NodeMonitor(BMessage* msg)
{
    int32 opcode = msg->GetInt32("opcode", 0);

    switch(opcode)
    {
        case B_ENTRY_CREATED:
        {
            dev_t device = msg->GetInt32("device", 0);
            ino_t dnode = msg->GetInt64("directory", 0);
            ino_t fnode = msg->GetInt64("node", 0);
            BString fname = msg->GetString("name", "");

            node_ref nref;
            nref.device = device;
            nref.node = fnode;

            printf("created: device(%d), directory(%ld), node(%ld)\n", device, dnode, fnode);

            if(compare_fs_entry_node(USER_AUTOLAUNCH_DIR, device, dnode))
                alview->Update();
            else {
                if(compare_fs_entry_node("/boot/home/config/settings", device, dnode) &&
                  (fname == "profile" || fname == "packages")) {
                    watch_node(&nref, B_WATCH_NAME | B_WATCH_STAT, this);
                    BPath path("/boot/home/config/settings", fname);
                    cached_values.insert_or_assign(path.Path(), nref);
                    __trace("Started watching: %s\n", path.Path());

                    fname == "profile" ? termenvview->Update() : blview->Update();
                }
                else if((compare_fs_entry_node("/boot/home/config/settings/boot",
                device, dnode)) && (fname == "UserBootscript" || fname == "UserShutdownScript" ||
                fname == "UserShutdownFinishScript" || fname == "UserSetupEnvironment")) {
                    watch_node(&nref, B_WATCH_NAME | B_WATCH_STAT, this);
                    BPath path("/boot/home/config/settings/boot", fname);
                    cached_values.insert_or_assign(path.Path(), nref);
                    __trace("Started watching: %s\n", path.Path());

                    dynamic_cast<TextEditorView*>(usview->FindView(fname.String()))->Update();
                }
                else if(compare_fs_entry_node("/boot/home/config/settings/kernel/drivers", device, dnode) &&
                fname == "kernel") {
                    watch_node(&nref, B_WATCH_NAME | B_WATCH_STAT, this);
                    cached_values.insert_or_assign(KERNEL_SETTINGS, nref);
                    __trace("Started watching: %s\n", KERNEL_SETTINGS);

                    kernview->Update();
                }
                else if(compare_fs_entry_node("/boot/system/settings", device, dnode)) {
                    watch_node(&nref, B_WATCH_NAME | B_WATCH_STAT, this);
                    cached_values.insert_or_assign(BLACKLIST_SYSTEM, nref);
                    __trace("Started watching: %s\n", BLACKLIST_SYSTEM);

                    blview->Update();
                }
            }
            break;
        }
        case B_ENTRY_REMOVED:
        {
            dev_t device = msg->GetInt32("device", 0);
            ino_t dnode = msg->GetInt64("directory", 0);
            ino_t fnode = msg->GetInt64("node", 0);

            printf("removed: device(%d), directory(%ld), node(%ld)\n", device, dnode, fnode);

            if(compare_fs_entry_node(USER_AUTOLAUNCH_DIR, device, dnode))
                alview->Update();
            else {
                if(compare_fs_entry_node("/boot/home/config/settings", device, dnode) &&
                   cached_values.find(USER_PROF_ENV) != cached_values.end() &&
                   fnode == cached_values[USER_PROF_ENV].node) {
                    termenvview->Update();
                    cached_values.erase(USER_PROF_ENV);
                    printf("Deleted watched entry: %s\n", USER_PROF_ENV);
                }
                if(compare_fs_entry_node("/boot/home/config/settings/boot", device, dnode) &&
                   cached_values.find(USER_BOOT_SCRIPT) != cached_values.end() &&
                   fnode == cached_values[USER_BOOT_SCRIPT].node) {
                    dynamic_cast<TextEditorView*>(usview->FindView("UserBootscript"))->Update();
                    cached_values.erase(USER_BOOT_SCRIPT);
                    printf("Deleted watched entry: %s\n", USER_BOOT_SCRIPT);
                }
                if(compare_fs_entry_node("/boot/home/config/settings/boot", device, dnode) &&
                   cached_values.find(USER_SHUTDOWN_SCRIPT) != cached_values.end() &&
                   fnode == cached_values[USER_SHUTDOWN_SCRIPT].node) {
                    dynamic_cast<TextEditorView*>(usview->FindView("UserShutdownScript"))->Update();
                    cached_values.erase(USER_SHUTDOWN_SCRIPT);
                    printf("Deleted watched entry: %s\n", USER_SHUTDOWN_SCRIPT);
                }
                if(compare_fs_entry_node("/boot/home/config/settings/boot", device, dnode) &&
                   cached_values.find(USER_SHUTDOWN_FINISH_SCRIPT) != cached_values.end() &&
                   fnode == cached_values[USER_SHUTDOWN_FINISH_SCRIPT].node) {
                    dynamic_cast<TextEditorView*>(usview->FindView("UserShutdownFinishScript"))->Update();
                    cached_values.erase(USER_SHUTDOWN_FINISH_SCRIPT);
                    printf("Deleted watched entry: %s\n", USER_SHUTDOWN_FINISH_SCRIPT);
                }
                if(compare_fs_entry_node("/boot/home/config/settings/boot", device, dnode) &&
                   cached_values.find(USER_SETUP_ENVIRONMENT) != cached_values.end() &&
                   fnode == cached_values[USER_SETUP_ENVIRONMENT].node) {
                    dynamic_cast<TextEditorView*>(usview->FindView("UserSetupEnvironment"))->Update();
                    cached_values.erase(USER_SETUP_ENVIRONMENT);
                    printf("Deleted watched entry: %s\n", USER_SETUP_ENVIRONMENT);
                }
                if(compare_fs_entry_node("/boot/home/config/settings/kernel/drivers", device, dnode) &&
                   cached_values.find(KERNEL_SETTINGS) != cached_values.end() &&
                   fnode == cached_values[KERNEL_SETTINGS].node) {
                    kernview->Update();
                    cached_values.erase(KERNEL_SETTINGS);
                    printf("Deleted watched entry: %s\n", KERNEL_SETTINGS);
                }
                if(compare_fs_entry_node("/boot/system/settings", device, dnode) &&
                   cached_values.find(BLACKLIST_SYSTEM) != cached_values.end() &&
                   fnode == cached_values[BLACKLIST_SYSTEM].node) {
                    blview->Update();
                    cached_values.erase(BLACKLIST_SYSTEM);
                    printf("Deleted watched entry: %s\n", BLACKLIST_SYSTEM);
                }
                if(compare_fs_entry_node("/boot/home/config/settings", device, dnode) &&
                   cached_values.find(BLACKLIST_USER) != cached_values.end() &&
                   fnode == cached_values[BLACKLIST_USER].node) {
                    blview->Update();
                    cached_values.erase(BLACKLIST_USER);
                    printf("Deleted watched entry: %s\n", BLACKLIST_USER);
                }
            }
            break;
        }
        case B_ENTRY_MOVED:
        {
            dev_t device = msg->GetInt32("device", 0);
            ino_t fnode = msg->GetInt64("node", 0);
            ino_t origin_dnode = msg->GetInt64("from directory", 0);
            ino_t dest_dnode = msg->GetInt64("to directory", 0);
            BString fname = msg->GetString("name");

            printf("moved: file[device(%d), node(%ld)]:%s, origin(%ld) --> destination(%ld)\n",
                device, fnode, fname.String(), origin_dnode, dest_dnode);

            node_ref nref;
            nref.device = device;
            nref.node = fnode;

            if(compare_fs_entry_node(USER_AUTOLAUNCH_DIR, device, origin_dnode) ||
               compare_fs_entry_node(USER_AUTOLAUNCH_DIR, device, dest_dnode)) {
               __trace("Movement in autolaunch found. Updating...\n");
               alview->Update();
            }
            else if((compare_fs_entry_node("/boot/home/config/settings", device, origin_dnode) ||
                     compare_fs_entry_node("/boot/home/config/settings", device, dest_dnode)) &&
                    (fname == "profile" || fname == "packages")) {
                // Stop monitoring the file "packages" or "profile" if it was moved
                if(compare_fs_entry_node("/boot/home/config/settings", device, origin_dnode)) {
                    watch_node(&nref, B_STOP_WATCHING, this);
                    __trace("Unwatched: %s, it was moved out of dir.\n", fname.String());
                }

                // Monitor a file if it was moved here and with
                // the name "packages" or "profile"
                if(compare_fs_entry_node("/boot/home/config/settings", device, dest_dnode)) {
                    watch_node(&nref, B_WATCH_NAME | B_WATCH_STAT, this);
                    __trace("Watching: %s, it was moved to dir.\n", fname.String());
                }

                fname == "profile" ? termenvview->Update() : blview->Update();
            }
            else if((compare_fs_entry_node("/boot/home/config/settings/boot", device, origin_dnode) ||
                     compare_fs_entry_node("/boot/home/config/settings/boot", device, dest_dnode)) &&
                    (fname == "UserBootscript" || fname == "UserShutdownScript" ||
                     fname == "UserShutdownFinishScript" || fname == "UserSetupEnvironment")) {
                // Stop monitoring a file "User(.)*" if it was moved out
                if(compare_fs_entry_node("/boot/home/config/settings/boot", device, origin_dnode)) {
                    watch_node(&nref, B_STOP_WATCHING, this);
                    __trace("Unwatched: %s, it was moved out of dir.\n", fname.String());
                }

                // Monitor a file moved here if it contains the name "User(.)*"
                if(compare_fs_entry_node("/boot/home/config/settings/boot", device, dest_dnode)) {
                    watch_node(&nref, B_WATCH_NAME | B_WATCH_STAT, this);
                    __trace("Watching: %s, it was moved to dir.\n", fname.String());
                }

                dynamic_cast<TextEditorView*>(usview->FindView(fname.String()))->Update();
            }
            else if((compare_fs_entry_node("/boot/system/settings/", device, origin_dnode) ||
                    compare_fs_entry_node("/boot/system/settings/", device, dest_dnode)) &&
                    (fname == "package")) {
                // Stop monitoring "packages" if it was moved out
                if(compare_fs_entry_node("/boot/system/settings/", device, origin_dnode)) {
                    watch_node(&nref, B_STOP_WATCHING, this);
                    __trace("Unwatched: %s, it was moved out of dir.\n", fname.String());
                }

                // Monitor a file moved here if it is named "packages"
                if(compare_fs_entry_node("/boot/system/settings/", device, dest_dnode)) {
                    watch_node(&nref, B_WATCH_NAME | B_WATCH_STAT, this);
                    __trace("Watching: %s, it was moved to dir.\n", fname.String());
                }

                blview->Update();
            }
            else if((compare_fs_entry_node("/boot/home/config/settings/kernel/drivers", device, origin_dnode) ||
                     compare_fs_entry_node("/boot/home/config/settings/kernel/drivers", device, dest_dnode)) &&
                     (fname == "kernel")) {
                if(compare_fs_entry_node("/boot/home/config/settings/kernel/drivers", device, origin_dnode)) {
                    watch_node(&nref, B_STOP_WATCHING, this);
                    __trace("Unwatched: %s, it was moved out of dir.\n", fname.String());
                }

                if(compare_fs_entry_node("/boot/home/config/settings/kernel/drivers", device, dest_dnode)) {
                    watch_node(&nref, B_WATCH_NAME | B_WATCH_STAT, this);
                    __trace("Watching: %s, it was moved to dir.\n", fname.String());
                }

                kernview->Update();
            }
            break;
        }
        case B_STAT_CHANGED:
        {
            dev_t device = msg->GetInt32("device", 0);
            ino_t node = msg->GetInt64("node", 0);

            printf("changed: device(%d), node(%ld)\n", device, node);

            if(compare_fs_entry_node(USER_AUTOLAUNCH_DIR, device, node))
                alview->Update();
            else if(compare_fs_entry_node(USER_PROF_ENV, device, node))
                termenvview->Update();
            else if(compare_fs_entry_node(USER_BOOT_SCRIPT, device, node))
                dynamic_cast<TextEditorView*>(usview->FindView("UserBootscript"))->Update();
            else if(compare_fs_entry_node(USER_SHUTDOWN_SCRIPT, device, node))
                dynamic_cast<TextEditorView*>(usview->FindView("UserShutdownScript"))->Update();
            else if(compare_fs_entry_node(USER_SHUTDOWN_FINISH_SCRIPT, device, node))
                dynamic_cast<TextEditorView*>(usview->FindView("UserShutdownFinishScript"))->Update();
            else if(compare_fs_entry_node(USER_SETUP_ENVIRONMENT, device, node))
                dynamic_cast<TextEditorView*>(usview->FindView("UserSetupEnvironment"))->Update();
            else if(compare_fs_entry_node(KERNEL_SETTINGS, device, node))
                kernview->Update();
            else if(compare_fs_entry_node(BLACKLIST_SYSTEM, device, node))
                blview->Update();
            else if(compare_fs_entry_node(BLACKLIST_USER, device, node))
                blview->Update();

            break;
        }
        case B_ATTR_CHANGED:
        case B_DEVICE_MOUNTED:
        case B_DEVICE_UNMOUNTED:
            __trace("Not handled here: %d.\n", opcode);
            break;
        default:
            __trace("Unknown opcode: %d.\n", opcode);
            break;
    }
}

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

void StartUpWin::RestoreAllConfigs()
{
    BAlert *alert = new BAlert();
    alert->SetTitle(B_TRANSLATE("Restore all configurations"));
    alert->SetText(B_TRANSLATE("Do you want to restore all the configurations to"
                               " their default values? This action cannot be undone."));
    alert->SetType(B_WARNING_ALERT);
    alert->AddButton(B_TRANSLATE("Do not restore"));
    alert->AddButton(B_TRANSLATE("Restore all configurations"));
    int32 result = alert->Go();

    if(result == 1) {
        alview->RestoreDefault();
        termenvview->RestoreDefault();
        dynamic_cast<TextEditorView*>(usview->FindView("UserBootscript"))->RestoreDefault();
        dynamic_cast<TextEditorView*>(usview->FindView("UserShutdownScript"))->RestoreDefault();
        dynamic_cast<TextEditorView*>(usview->FindView("UserShutdownFinishScript"))->RestoreDefault();
        dynamic_cast<TextEditorView*>(usview->FindView("UserSetupEnvironment"))->RestoreDefault();
        kernview->RestoreDefault();
        blview->RestoreDefault();
    }
}

BMenuBar* StartUpWin::BuildMenu()
{
    BMenuBar* menu = new BMenuBar("");

    tabSelectionMenu = new BMenu(B_TRANSLATE("Go to"), B_ITEMS_IN_COLUMN);
    tabSelectionMenu->SetRadioMode(true);

    BLayoutBuilder::Menu<>(menu)
        .AddMenu(B_TRANSLATE_COMMENT(kAppName, "This is the App menu. It should be the same as the \"system name\"."))
        .AddItem(B_TRANSLATE("Quit"), M_QUIT_REQUESTED, 'Q')
        .End()
        .AddMenu(tabSelectionMenu)
        .End()
        .AddMenu(B_TRANSLATE("Tools"))
            .AddItem(B_TRANSLATE_COMMENT("Open BootManager", "Use the localized name"),
                M_LAUNCH_BOOTMAN, 'B')
            .AddItem(B_TRANSLATE_COMMENT("Open VirtualMemory", "Use the localized name"),
                M_LAUNCH_VMEM, 'M')
        .End()
        .AddMenu(B_TRANSLATE("Settings"))
            .AddItem(B_TRANSLATE("Do not warn about kernel settings"), M_SETTINGS_KSWARN)
            .AddSeparator()
            .AddItem(B_TRANSLATE("Restore all default configurations" B_UTF8_ELLIPSIS),
                M_RESTORE_DEFAULT, 'R', B_SHIFT_KEY)
        .End()
        .AddMenu(B_TRANSLATE("Help"))
            .AddItem(B_TRANSLATE("Documentation"), M_HELP_DOCS)
            .AddMenu(B_TRANSLATE("Help topics"))
                .AddItem(B_TRANSLATE("Bash and scripting"), 'hbsh')
                .AddItem(B_TRANSLATE("Boot Loader"), 'hboo')
                .AddItem(B_TRANSLATE("Disabling components of packages"), 'hdis')
            .End()
            .AddSeparator()
            .AddItem(B_TRANSLATE("About" B_UTF8_ELLIPSIS), M_ABOUT_REQUESTED)
        .End()
    .End();

    return menu;
}


bool StartUpWin::LoadWatcher(const char* path, bool addToCache)
{
    BEntry entry(path);

    if(exists(path)) {
        watch_fs_entry(&entry, this);
        node_ref nref;
        entry.GetNodeRef(&nref);
        if(addToCache)
            cached_values.insert_or_assign(path, nref);
        return true;
    }
    else {
        BPath path;
        entry.GetPath(&path);
        __trace("[%s] Watcher could not be created.\n", path.Leaf());
        return false;
    }
}

void StartUpWin::UnloadWatcher(const char* path)
{
    BEntry entry(path);
    unwatch_fs_entry(&entry, this);
}
