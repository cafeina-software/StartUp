#include <cstdio>
#include <AppKit.h>
#include <Catalog.h>
#include <private/interface/AboutWindow.h>

#include "StartUpApp.h"
#include "DataLoader.h"

StartUpApp::StartUpApp()
: BApplication(kAppSignature)
{
    if(userscript_load(USER_BOOT_SCRIPT, &ubsdata) != B_OK)
        fprintf(stderr, "UBS not loaded\n");

    if(userscript_load(USER_SHUTDOWN_SCRIPT, &ussdata) != B_OK)
        fprintf(stderr, "USS not loaded\n");

    if(userscript_load(USER_SHUTDOWN_FINISH_SCRIPT, &usfdata) != B_OK)
        fprintf(stderr, "USFS not loaded\n");

    if(autolaunch_load(autolaunch_list) != B_OK)
        fprintf(stderr, "Autolaunch not loaded\n");

    if(userscript_load(USER_SETUP_ENVIRONMENT, &usedata) != B_OK)
        fprintf(stderr, "USE not loaded\n");

    if(userscript_load(USER_PROF_ENV, &profdata) != B_OK)
        fprintf(stderr, "\"profile\" file not loaded\n");

    if(kernelsettings_load(&kernelsettings_list) != B_OK) {
        fprintf(stderr, "\"kernelsettings\" not loaded\n");
        kernelsettings_create();
        kernelsettings_load(&kernelsettings_list);
    }

    win = new StartUpWin();
    _LoadSettings();
    win->Show();
}

StartUpApp::~StartUpApp()
{
}

bool StartUpApp::QuitRequested()
{
    _SaveSettings();
    return BApplication::QuitRequested();
}

void StartUpApp::MessageReceived(BMessage* msg)
{
    switch(msg->what)
    {
        case B_QUIT_REQUESTED:
            QuitRequested();
            break;
        default:
            BApplication::MessageReceived(msg);
            break;
    }
}

#undef B_TRANSLATION_CONTEXT
#define B_TRANSLATION_CONTEXT "About box"

void StartUpApp::AboutRequested()
{
	const char* extraCopyrights[] = {
		// If you helped me with this program, add yourself:
		// "<Year span> <Your name>",
		NULL
	};
	const char* authors[] = {
		B_TRANSLATE("Cafeina (original author)"),
		// If you helped me with this program, add yourself:
		// B_TRANSLATE("<Your name> (<your role>)"),
		NULL
	};
	const char* website[] = {
        kAppHomePage,
		NULL
	};
	const char* thanks[] = {
		B_TRANSLATE("The BeOS and Haiku community members."),
		NULL
	};
	const char* history[] = {
        B_TRANSLATE("0.2\tKernel settings options added."),
		B_TRANSLATE("0.1\tInitial version."),
		NULL
	};

	BAboutWindow *about = new BAboutWindow(B_TRANSLATE_SYSTEM_NAME(kAppName),
		kAppSignature);
	about->AddDescription(B_TRANSLATE("Start-up items editor."));
	about->AddCopyright(2024, "Cafeina", extraCopyrights);
	about->AddAuthors(authors);
	about->AddSpecialThanks(thanks);
	about->AddVersionHistory(history);
	about->AddExtraInfo(B_TRANSLATE("Check README for more information."));
	about->AddText(B_TRANSLATE("Website:"), website);
	about->SetVersion(kAppVersionStr);
	about->Show();
}

BString StartUpApp::CurrentUBS()
{
    return ubsdata;
}

BString StartUpApp::CurrentUSS()
{
    return ussdata;
}

BString StartUpApp::CurrentUSF()
{
    return usfdata;
}

std::vector<autolaunch_entry> StartUpApp::CurrentALList()
{
    return autolaunch_list;
}

BString StartUpApp::CurrentUserEnv()
{
    return usedata;
}

BString StartUpApp::CurrentProfileEnv()
{
    return profdata;
}

std::vector<entry> StartUpApp::CurrentKernelSettings()
{
    return kernelsettings_list;
}

// #pragma mark Private methods

status_t StartUpApp::_LoadSettings()
{
    status_t status = B_OK;
    BPath usrsetpath;
    if((status = find_directory(B_USER_SETTINGS_DIRECTORY, &usrsetpath)) != B_OK) {
        fprintf(stderr, "%s > user settings dir could not even be found\n", __func__);
        return status;
    }

    usrsetpath.Append(kAppName ".settings");
    BFile file(usrsetpath.Path(), B_READ_ONLY);
    if((status = file.InitCheck()) != B_OK) {
        fprintf(stderr, "%s > user settings file \"%s\" could not even be opened\n", __func__, usrsetpath.Path());
        return status;
    }

    if((status = currentSettings.Unflatten(&file)) != B_OK) {
        fprintf(stderr, "%s > settings could not be unflattened from the file \n", __func__);
        return status;
    }

    if((status = win->LoadSettings(&currentSettings)) != B_OK) {
        fprintf(stderr, "%s > settings could not be loaded to the window\n", __func__);
        return status;
    }

    return status;
}

status_t StartUpApp::_SaveSettings()
{
    status_t status = B_OK;
    if(win->Lock()) {
        win->SaveSettings(&currentSettings);
        win->Unlock();
    }
    BPath usrsetpath;
    if((status = find_directory(B_USER_SETTINGS_DIRECTORY, &usrsetpath)) != B_OK) {
        fprintf(stderr, "%s > user settings dir could not even be found\n", __func__);
        return status;
    }

    usrsetpath.Append(kAppName ".settings");
    BFile file(usrsetpath.Path(), B_READ_WRITE | B_CREATE_FILE | B_ERASE_FILE);
    if((status = file.InitCheck()) != B_OK) {
        fprintf(stderr, "%s > user settings file \"%s\" could not even be set\n", __func__, usrsetpath.Path());
        return status;
    }
    file.SetPermissions(DEFFILEMODE);

    if((status = currentSettings.Flatten(&file)) != B_OK) {
        fprintf(stderr, "%s > settings could not be flattened to the file \n", __func__);
        return status;
    }

    return status;
}

