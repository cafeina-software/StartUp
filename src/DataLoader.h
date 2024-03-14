#ifndef __DATA_LOADER_H
#define __DATA_LOADER_H

#include <AppKit.h>
#include <SupportKit.h>
#include <Path.h>
#include <variant>
#include <vector>

#define USER_BOOT_SCRIPT            "/boot/home/config/settings/boot/UserBootscript"
#define USER_SHUTDOWN_SCRIPT        "/boot/home/config/settings/boot/UserShutdownScript"
#define USER_SHUTDOWN_FINISH_SCRIPT "/boot/home/config/settings/boot/UserShutdownFinishScript"
#define USER_SETUP_ENVIRONMENT      "/boot/home/config/settings/boot/UserSetupEnvironment"
#define USER_AUTOLAUNCH_DIR         "/boot/home/config/settings/boot/launch"
#define USER_PROF_ENV               "/boot/home/config/settings/profile"
#define KERNEL_SETTINGS             "/boot/home/config/settings/kernel/drivers/kernel"

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

bool     exists(const char* filename);
void     launch(const char* mimetype, const char* path);

status_t load_data_from_file(const char* in_filename, BString *out_data);
bool     find_entry(const char* in_data, const char* pattern, entry* out_data = NULL);
bool     find_key(void* in_data, const char* key, void* out_value = NULL);
status_t watch_fs_entry           (BEntry* entry, BMessenger target = be_app_messenger);
status_t unwatch_fs_entry         (BEntry* entry, BMessenger target = be_app_messenger);
bool     compare_fs_entry_node    (const char* entrypath, dev_t dev, ino_t ino);

status_t autolaunch_load          (std::vector<autolaunch_entry>& entrylist);
status_t autolaunch_create_link   (BString origin);
status_t autolaunch_copy_file     (BString origin);
status_t autolaunch_create_shell  (BString name, BString shell = "/bin/bash");

status_t userscript_create        (BString path);
void     userscript_default       (BString what, BString* out_data);
status_t userscript_load          (BString path, BString* out_data);
status_t userscript_save          (BString path, BString in_data);

status_t kernelsettings_create    ();
void     kernelsettings_default   (std::vector<entry>* out_data);
status_t kernelsettings_load      (std::vector<entry>* entrylist);
status_t kernelsettings_entry_add (std::vector<entry>* entrylist, entry item);
status_t kernelsettings_entry_edit(std::vector<entry>* entrylist, const char* key, BString value, bool enabled);
status_t kernelsettings_save      (std::vector<entry> entrylist);

#endif /* __DATA_LOADER_H */
