#include <cstdio>
#include <AppKit.h>
#include <Catalog.h>
#include <private/interface/AboutWindow.h>

#include "StartUpApp.h"
#include "DataLoader.h"

StartUpApp::StartUpApp(bool experimental)
: BApplication(kAppSignature), _exp(experimental)
{
    if(load_userbootscript(&ubsdata) != B_OK)
        fprintf(stderr, "UBS not loaded\n");

    if(load_userautolaunch(autolaunch_list) != B_OK)
        fprintf(stderr, "UBS autolaunch not loaded\n");

    if(load_usersetupenvironment(environment_list) != B_OK)
        fprintf(stderr, "UserEnv not loaded\n");

    if(load_usertermprofileenv(&profdata) != B_OK)
        fprintf(stderr, "\"profile\" file not loaded\n");

    win = new StartUpWin();
    win->Show();

}

StartUpApp::~StartUpApp()
{
}

bool StartUpApp::QuitRequested()
{
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

std::vector<autolaunch_entry> StartUpApp::CurrentALList()
{
    return autolaunch_list;
}

std::vector<environment_entry> StartUpApp::CurrentUserEnv()
{
    return environment_list;
}

BString StartUpApp::CurrentProfileEnv()
{
    return profdata;
}
