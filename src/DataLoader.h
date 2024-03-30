#ifndef __DATA_LOADER_H
#define __DATA_LOADER_H

#include <AppKit.h>
#include <SupportKit.h>
#include <Path.h>
#include <list>
#include <vector>

#define USER_BOOT_SCRIPT            "/boot/home/config/settings/boot/UserBootscript"
#define USER_SHUTDOWN_SCRIPT        "/boot/home/config/settings/boot/UserShutdownScript"
#define USER_SHUTDOWN_FINISH_SCRIPT "/boot/home/config/settings/boot/UserShutdownFinishScript"
#define USER_SETUP_ENVIRONMENT      "/boot/home/config/settings/boot/UserSetupEnvironment"
#define USER_AUTOLAUNCH_DIR         "/boot/home/config/settings/boot/launch"
#define USER_PROF_ENV               "/boot/home/config/settings/profile"
#define KERNEL_SETTINGS             "/boot/home/config/settings/kernel/drivers/kernel"
#define BLACKLIST_SYSTEM            "/boot/system/settings/packages"
#define BLACKLIST_USER              "/boot/home/config/settings/packages"

struct autolaunch_entry {
    BPath path;
    node_flavor flavor;
    BString data;
};
struct entry {
    bool enabled;
    BString key;
    BString value;
};

class Package
{
public:
                        Package             (BString n);
                       ~Package             ();
    BString             Name                ();
    std::list<BString> *Blacklist           ();
    void                AddBlacklistItem    (BString item);
    void                RemoveBlacklistItem (BString item);
    bool                IsBlacklistEmpty    ();
    void                ClearBlacklist      ();
private:
    BString             name;
    std::list<BString>  blacklist;
};

class PackageList
{
public:
                        PackageList         ();
                       ~PackageList         ();
    void                AddPackage          (Package pkg);
    void                AddPackage          (BString pkgname);
    void                RemovePackage       (BString pkgname);
    Package*            PackageByName       (BString pkgname);
    std::list<Package>& Packages            ();
    bool                HasPackage          (BString pkgname);
    bool                IsEmpty             ();
    void                Clear               ();
private:
    std::list<Package>  packages;
};

bool     exists                   (const char* filename);
status_t launch                   (const char* mimetype, const char* path);

bool     find_entry               (const char* in_data, const char* pattern, entry* out_data = NULL);
bool     find_key                 (void* in_data, const char* key, void* out_value = NULL);
status_t watch_fs_entry           (BEntry* entry, BMessenger target = be_app_messenger);
status_t unwatch_fs_entry         (BEntry* entry, BMessenger target = be_app_messenger);
bool     compare_fs_entry_node    (const char* entrypath, dev_t dev, ino_t ino);

status_t autolaunch_load          (std::vector<autolaunch_entry>& entrylist);
status_t autolaunch_create_link   (BString origin);
status_t autolaunch_copy_file     (BString origin);
status_t autolaunch_create_shell  (BString name, BString shell = "/bin/bash");
void     autolaunch_clear_folder  ();

status_t userscript_create        (BString path);
void     userscript_default       (BString what, BString* out_data);
status_t userscript_load          (BString path, BString* out_data);
status_t userscript_save          (BString path, BString in_data);
void     userscript_delete        (BString path);

status_t kernelsettings_create    ();
void     kernelsettings_default   (std::vector<entry>* out_data);
status_t kernelsettings_load      (std::vector<entry>* entrylist);
status_t kernelsettings_entry_add (std::vector<entry>* entrylist, entry item);
status_t kernelsettings_entry_edit(std::vector<entry>* entrylist, const char* key, BString value, bool enabled);
status_t kernelsettings_save      (std::vector<entry> entrylist);

status_t blacklist_load           (BString path, PackageList* pkglist);
status_t blacklist_save           (BString path, PackageList  pkglist);
status_t blacklist_delete         (BString path, PackageList* pkglist);

#endif /* __DATA_LOADER_H */
