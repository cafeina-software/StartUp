#ifndef __STARTUP_WIN_H
#define __STARTUP_WIN_H

#include <InterfaceKit.h>
#include <unordered_map>

#include "AutolaunchView.h"
#include "BlacklistView.h"
#include "KernelSettingsView.h"
#include "TextEditorView.h"
#include "UserScriptsView.h"

#define M_HELP_DOCS        'docs'
#define M_RESTORE_DEFAULT  'defs'
#define M_ABOUT_REQUESTED  'abtt'
#define M_QUIT_REQUESTED   'clse'
#define M_LAUNCH_BOOTMAN   'boot'
#define M_LAUNCH_VMEM      'vmem'
#define M_SETTINGS_KSWARN  'diss'
#define M_SETTINGS_BLWARN  'blis'
#define M_VIEW_AUTOLAUNCH  'tab0'
#define M_VIEW_TERMPROFILE 'tab1'
#define M_VIEW_USERSCRIPTS 'tab2'
#define M_VIEW_KERNELSET   'tab3'
#define M_VIEW_BLACKLIST   'tab4'

class StartUpWin : public BWindow
{
public:
                         StartUpWin          ();
    virtual             ~StartUpWin          ();
    virtual void         MessageReceived     (BMessage* msg);
    void                 StartMonitoring     ();
    void                 StopMonitoring      ();
    status_t             LoadSettings        (BMessage* indata);
    status_t             SaveSettings        (BMessage* outdata);
private:
    BMenuBar            *BuildMenu           ();
    void                 DisplayWarning      (bool& setting, const char* title,
                                              const char* text, const char* menuitemname);

    void                 NodeMonitor         (BMessage* msg);
    bool                 LoadWatcher         (const char* path, bool addToCache = true);
    void                 UnloadWatcher       (const char* path);

    void                 OpenDocumentation   ();
    void                 OpenUGDocumentation (const char* entry);
    void                 RestoreAllConfigs   ();
private:
    BTabView            *tabView;
    BMenuBar            *optionsMenu;
    BMenu               *tabSelectionMenu;
    AutolaunchView      *alview;
    UserScriptsView     *usview;
    TextEditorView      *ubsview,
                        *termenvview;
    KernelSettingsView  *kernview;
    BlacklistView       *blview;
    bool                 kswarningshown,
                         blwarningshown;
    std::unordered_map<const char*,node_ref> cached_values;
};

#endif /* __STARTUP_WIN_H */
