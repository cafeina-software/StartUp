#include <Roster.h>
#include <StorageKit.h>
#include <SupportKit.h>
#include <regex>
#include <cctype>
#include <string>
#include <sstream>
#include <vector>

#include "DataLoader.h"

node_flavor entry_type(BEntry entry);
bool is_space(char c);
bool is_string_digit(const char* in);
status_t load_data_from_file(const char* in_filename, BString *out_data);
status_t save_data_to_file(const char* in_filename, const char* in_data, int in_length);

// #pragma mark - General functions

void launch(const char* mimetype, const char* path)
{
	if(path) {
		char* args[] = { const_cast<char*>("app"), const_cast<char*>(path) };
   		be_roster->Launch(mimetype, 1, args+1);
	}
	else
		be_roster->Launch(mimetype);
}

bool exists(const char* filename)
{
    BEntry entry(filename);
    return entry.Exists();
}

bool find_entry(const char* in_data, const char* pattern, entry* out_data)
{
	bool result = false;

	std::istringstream iss(in_data);
	std::string line;
	std::smatch match;
	std::regex rgx(pattern);

	while(std::getline(iss, line)) {
		if(std::regex_search(line, rgx)) {
			if(std::regex_search(line, match, rgx)) {
				result = true;
				if(out_data != NULL) {
					if(match[1].str() == "#")
						out_data->enabled = false;
					else
						out_data->enabled = true;
					out_data->key = match[2].str().c_str();
                    out_data->value = match[3].str().c_str();
				}
			}
		}
	}
	return result;
}


// #pragma mark - Helper functions

node_flavor entry_type(BEntry entry)
{
    if(entry.IsFile())
        return B_FILE_NODE;
    else if(entry.IsDirectory())
        return B_DIRECTORY_NODE;
    else if(entry.IsSymLink())
        return B_SYMLINK_NODE;
    else
        return B_ANY_NODE;
}

bool is_space(char c)
{
    return c == ' ';
}

bool is_string_digit(const char* in)
{
    bool result = true;

    const unsigned char* i = (const unsigned char*)in;
    while(*i != '\0' && isdigit(*i))
        ++i;
    if(*i != '\0')
        result = false;
    return result;
}

status_t load_data_from_file(const char* in_filename, BString *out_data)
{
	status_t status = B_OK;

    BFile file(in_filename, B_READ_WRITE);
    if((status = file.InitCheck()) != B_OK)
    	return status;

	off_t size;
	file.GetSize(&size);
	BString buffer;
	file.Read(buffer.LockBuffer(size), size);
	*out_data = buffer;

	return status;
}

status_t save_data_to_file(const char* in_filename, const char* in_data, int in_length)
{
	status_t status = B_OK;
	ssize_t dataread;

	BFile file(in_filename, B_READ_WRITE | B_CREATE_FILE | B_ERASE_FILE);
	if((status = file.InitCheck()) != B_OK)
		return status;

	if((dataread = file.Write((void*)in_data, in_length)) != in_length) {
        fprintf(stderr, "%s: Error: not all the data was written. Expected data: %d. Data written: %zd.\n",
            __func__, in_length, dataread);
		return B_ERROR;
    }

	file.Unset();

	return B_OK;
}

status_t watch_fs_entry(BEntry* entry, BMessenger target)
{
	node_ref ref;
	entry->GetNodeRef(&ref);
	uint32 flags = B_WATCH_NAME | B_WATCH_STAT | B_WATCH_ATTR;
	if(entry->IsDirectory())
		flags |= B_WATCH_DIRECTORY;
	return watch_node(&ref, flags, target);
}

status_t unwatch_fs_entry(BEntry* entry, BMessenger target)
{
	node_ref ref;
	entry->GetNodeRef(&ref);
	return watch_node(&ref, B_STOP_WATCHING, target);
}

bool compare_fs_entry_node (const char* entrypath, dev_t dev, ino_t ino)
{
    struct stat st;
    BEntry entry(entrypath);
    entry.GetStat(&st);
    return st.st_dev == dev && st.st_ino == ino;
}

// #pragma mark - Autolaunch

status_t autolaunch_load(std::vector<autolaunch_entry>& entrylist)
{
    BDirectory autolaunchdir(USER_AUTOLAUNCH_DIR);
    BEntry entry;
    BPath path;
    node_flavor flavor;
    BString data;

    while (autolaunchdir.GetNextEntry(&entry) == B_OK) {
        entry.GetPath(&path);
        flavor = entry_type(entry);

        if(flavor == B_FILE_NODE) {
            BFile file(path.Path(), B_READ_ONLY);
            BNodeInfo info(&file);
            char mime[256];
            if(info.GetType(mime) != B_OK)
                strcpy(mime, "");
            data = mime;
        }
        else if(flavor == B_SYMLINK_NODE) {
            BSymLink symlink(&entry);
            if(symlink.IsAbsolute()) {
                char destination[B_PATH_NAME_LENGTH];
                symlink.ReadLink(destination, B_PATH_NAME_LENGTH);
                data = destination;
            }
            else {
                BPath realpath;
                symlink.MakeLinkedPath(USER_AUTOLAUNCH_DIR, &realpath);
                data = realpath.Path();
            }
        }
        else
            data = "";

        autolaunch_entry alentry = {path, flavor, data};
        entrylist.push_back(alentry);
    }

    return B_OK;
}

status_t autolaunch_create_link(BString origin)
{
    BFile file(origin, B_READ_ONLY);
    if(file.InitCheck() != B_OK)
        return B_ENTRY_NOT_FOUND;

    BEntry file_entry(origin);
    char name[B_FILE_NAME_LENGTH];
    file_entry.GetName(name);

    BDirectory autolaunchdir(USER_AUTOLAUNCH_DIR);
    BString destination("./");
    destination.Append(name);
    BSymLink symlink(origin);
    status_t status;
    if((status = autolaunchdir.CreateSymLink(destination, origin, &symlink)) != B_OK)
        return status;

    return B_OK;
}

status_t autolaunch_copy_file(BString origin)
{
    status_t status = B_OK;

    BFile originfile(origin, B_READ_ONLY);
    if(originfile.InitCheck() != B_OK) {
        fprintf(stderr, "Origin file could not be found.\n");
        return B_ENTRY_NOT_FOUND;
    }

    BEntry file_entry(origin);
    char name[B_FILE_NAME_LENGTH];
    file_entry.GetName(name);

    BString destinationfilename(USER_AUTOLAUNCH_DIR);
    destinationfilename.Append("/").Append(name);
    BFile destinationfile(destinationfilename, B_READ_WRITE | B_CREATE_FILE | B_FAIL_IF_EXISTS);
    if(destinationfile.InitCheck() != B_OK) {
        fprintf(stderr, "Destination file could not be created.\n");
        return B_NO_INIT;
    }

    off_t size;
    originfile.GetSize(&size);
    char* buffer = new char[size];
    ssize_t result = originfile.Read(buffer, size);
    if(result == size) {
        ssize_t out = destinationfile.Write(buffer, size);
        if(out != size) {
            fprintf(stderr, "Destination file could not be written correctly.\n");
            return B_ERROR;
        }
        status_t permresult = destinationfile.SetPermissions(S_IRWXU | S_IRGRP | S_IXGRP);
        if(permresult != B_OK)
            fprintf(stderr, "Destination file permissions could not be set. It will not be executed at start.\n");
    }
    else {
        fprintf(stderr, "Origin file could not be read.\n");
        return B_ERROR;
    }
    delete[] buffer;
    return status;
}

status_t autolaunch_create_shell(BString name, BString shell)
{
    status_t status = B_OK;

    BString filepath(USER_AUTOLAUNCH_DIR);
    filepath.Append("/").Append(name).Append(".sh");
    BFile script(filepath, B_READ_WRITE | B_CREATE_FILE | B_FAIL_IF_EXISTS);
    if((status = script.InitCheck()) != B_OK)
        return status;

    BString data;
    data.Append("#!").Append(shell).Append("\n");
    size_t length = data.Length();
    ssize_t result = script.Write(data.String(), length);
    if(result == length)
        script.SetPermissions(S_IRWXU | S_IRGRP | S_IXGRP);
    else {
        fprintf(stderr, "Error: not all the data could be written. The corrupted file will be deleted.\n");
        BEntry entry(filepath);
        entry.Remove();
        status = B_ERROR;
    }

    return status;
}

// #pragma mark - User scripts

status_t userscript_create(BString path)
{
    BString buffer;
    BPath _path(path);
    userscript_default(_path.Leaf(), &buffer);
    return userscript_save(path, buffer);
}

void userscript_default(BString what, BString* out_data)
{
    out_data->SetTo("");

    if(what == "UserBootscript") {
        out_data->Append("#!/bin/sh\n")
                 .Append("\n")
                 .Append("# DO NOT EDIT!\n")
                 .Append("#=====================================================================\n")
                 .Append("# Start programs and open files in the boot launch folder\n")
                 .Append("for file in $HOME/config/settings/boot/launch/* \n")
                 .Append("do \n")
                 .Append("	/bin/open \"$file\" &\n")
                 .Append("done\n")
                 .Append("#=====================================================================\n")
                 .Append("# FEEL FREE TO EDIT BELOW\n")
                 .Append("\n")
                 .Append("# Add custom commands to execute at every startup here.\n")
                 .Append("\n")
                 .Append("# This file is a standard bash script. For more information regarding shell \n")
                 .Append("# scripts, refer to any online documentation for \"bash scripting\"\n")
                 .Append("\n")
                 .Append("# During boot, the commands listed in this script will be executed.\n")
                 .Append("# Typically, you will want to include a trailing '&' on each line.\n")
                 .Append("# This will allow the script to execute that command and process the next line.\n")
                 .Append("\n")
                 .Append("# To launch certain applications at boot-up, put links to those applications in\n")
                 .Append("# the above boot launch directory.\n")
                 .Append("\n");
    }
    else if(what == "UserSetupEnvironment") {
        out_data->Append("# Quick start for file UserSetupEnvironment.  Copy or rename this file as\n")
                 .Append("#	\"/boot/home/config/settings/boot/UserSetupEnvironment\".\n")
                 .Append("# Add custom environment variables to initialize\n")
                 .Append("# This file is a standard bash script.  For more information regarding shell\n")
                 .Append("# scripts, refer to any online documentation for \"bash scripting\"\n")
                 .Append("\n")
                 .Append("# Set important variables\n")
                 .Append("# export IS_COMPUTER_ON=1\n")
                 .Append("\n");
    }
}

status_t userscript_load(BString path, BString* out_data)
{
    return load_data_from_file(path.String(), out_data);
}

status_t userscript_save(BString path, BString in_data)
{
    int length = strlen(in_data.String());
    return save_data_to_file(path.String(), in_data.String(), length);
}

// #pragma mark - Kernel settings

status_t kernelsettings_create()
{
    if(exists(KERNEL_SETTINGS))
        return B_FILE_EXISTS;

    std::vector<entry> entrylist;
    kernelsettings_default(&entrylist);
    return kernelsettings_save(entrylist);
}

void kernelsettings_default(std::vector<entry>* out_data)
{
    out_data->push_back({false, "disable_smp", "true"});
    out_data->push_back({false, "disable_ioapic", "true"});
    out_data->push_back({false, "apm", "true"});
    out_data->push_back({false, "acpi", "false"});
    out_data->push_back({false, "4gb_memory_limit", "true"});
    out_data->push_back({false, "fail_safe_video_mode", "true"});
    out_data->push_back({false, "bluescreen", "false"});
    out_data->push_back({true, "load_symbols", "true"});
    out_data->push_back({false, "emergency_keys", "false"});
    out_data->push_back({false, "serial_debug_output", "false"});
    out_data->push_back({false, "serial_debug_port", "1"});
    out_data->push_back({false, "serial_debug_speed", "57600"});
    out_data->push_back({false, "syslog_debug_output", "false"});
    out_data->push_back({false, "syslog_buffer_size", "131768"});
    out_data->push_back({false, "syslog_time_stamps", "true"});
    out_data->push_back({false, "syslog_max_size", "20MB"});
    out_data->push_back({false, "bochs_debug_output", "true"});
    out_data->push_back({false, "laplinkll_debug_output", "true"});
    out_data->push_back({false, "qemu_single_step_hack", "true"});
}

status_t kernelsettings_load(std::vector<entry>* entrylist)
{
    status_t status = B_OK;

    BString buffer;
    status = load_data_from_file(KERNEL_SETTINGS, &buffer);
    if(status != B_OK)
        return B_ERROR;

    bool result;
    std::vector<const char*> keys = {"disable_smp", "disable_ioapic", "apm",
        "acpi", "4gb_memory_limit", "fail_safe_video_mode", "bluescreen",
        "load_symbols", "emergency_keys", "serial_debug_output", "serial_debug_port",
        "serial_debug_speed", "syslog_debug_output", "syslog_buffer_size",
        "syslog_time_stamps", "syslog_max_size", "bochs_debug_output",
        "laplinkll_debug_output", "qemu_single_step_hack"};
    for(const auto& key : keys) {
        BString regex_str;
        regex_str.Append("^(#)?\\s*(").Append(key).Append(")\\s+(\\w+)\\s*$");

        entry e;
        result = find_entry(buffer.String(), regex_str.String(), &e);
        if(result)
            entrylist->push_back({e.enabled, e.key, e.value});
    }
    return status;
}

status_t kernelsettings_entry_add (std::vector<entry>* entrylist, entry item)
{
    if(!entrylist)
        return B_ERROR;

    entrylist->push_back({item.enabled, item.key, item.value});
    return B_OK;
}

status_t kernelsettings_entry_edit(std::vector<entry>* entrylist, const char* key, BString value, bool enabled)
{
    auto x = entrylist->begin();
    while(x != entrylist->end() && strcmp(x->key, key) != 0)
        ++x;
    if(x != entrylist->end()) {
        x->value = value;
        x->enabled = enabled;
        return B_OK;
    }
    else
        return B_ENTRY_NOT_FOUND;
}

status_t kernelsettings_save(std::vector<entry> entrylist)
{
    BString buffer;
    for(const auto& item : entrylist) {
        if(!item.enabled)
            buffer.Append("#");
        buffer.Append(item.key);
        buffer.Append(" ");
        buffer.Append(item.value);
        buffer.Append("\n");
    }
    int length = strlen(buffer.String());
    return save_data_to_file(KERNEL_SETTINGS, buffer.String(), length);
}
