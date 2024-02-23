#ifndef __DATA_LOADER_H
#define __DATA_LOADER_H

#include <Path.h>
#include <String.h>
#include <vector>

#define USER_UBS "/boot/home/config/settings/boot/UserBootscript"
#define USER_AUTOLAUNCH_DIR "/boot/home/config/settings/boot/launch"
#define USER_USENV "/boot/home/config/settings/boot/UserSetupEnvironment"
#define USER_PROF_ENV "/boot/home/config/settings/profile"

struct autolaunch_entry {
    BPath path;
    node_flavor flavor;
    BString data;
};
struct environment_entry {
    bool enabled;
    BString name;
    BString value;
};

bool exists(const char* filename);
void launch(const char* mimetype, const char* path);

status_t create_file_data(void* in_what);
void get_default_data(void* in_what, BString* out_data);
status_t load_data(void* in_what, BString* out_data);
status_t save_data(void* in_what, const char* in_data, int in_length);

status_t create_userbootscript();
status_t load_userbootscript(BString* data);
void default_userbootscript(BString* data);
status_t save_userbootscript(const char* data, int length);

status_t load_userautolaunch(std::vector<autolaunch_entry>& entrylist);
status_t create_userautolaunch_link(BString origin);
status_t copy_to_userautolaunch(BString origin);
status_t create_sh_script_userautolaunch(BString name, BString shell = "/bin/bash");

status_t load_usersetupenvironment(std::vector<environment_entry>& entrylist);
status_t add_to_usersetupenvironment(environment_entry entry, std::vector<environment_entry>& entrylist);
status_t edit_entry_usersetupenvironment(BString key, environment_entry newdata,std::vector<environment_entry>& entrylist);
status_t delete_entry_usersetupenvironment(BString key,  std::vector<environment_entry>& entrylist);
status_t create_usersetupenvironment();
void default_usersetupenvironment(BString* data);

status_t create_usertermprofileenv();
void default_usertermprofileenv(BString *out_data);
status_t save_usertermprofileenv(const char *out_data, int length);
status_t load_usertermprofileenv(BString*);

#endif /* __DATA_LOADER_H */
