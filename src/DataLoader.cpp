#include "DataLoader.h"
#include <StorageKit.h>
#include <AppKit.h>
#include <cstdio>
#include <iostream>
#include <regex>
#include <string>
#include <sstream>
#include <vector>

node_flavor entry_type(BEntry entry);
bool is_space(char c);
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
    bool result;
    BEntry entry(filename);
    return entry.Exists();
}

status_t create_file_data(void* in_what)
{
	status_t status = B_OK;

    if(strcmp(reinterpret_cast<const char*>(in_what), USER_UBS) == 0)
        status = create_userbootscript();
    else if(strcmp(reinterpret_cast<const char*>(in_what), USER_PROF_ENV) == 0)
        status = create_usertermprofileenv();
    else {
        status = B_ENTRY_NOT_FOUND;
    }

    return status;
}

void get_default_data(void* in_what, BString* out_data)
{
    if(strcmp(reinterpret_cast<const char*>(in_what), USER_UBS) == 0)
        default_userbootscript(out_data);
    else if(strcmp(reinterpret_cast<const char*>(in_what), USER_PROF_ENV) == 0)
        default_usertermprofileenv(out_data);
    else
        *out_data = "";
}

status_t load_data(void* in_what, BString* out_data)
{
   status_t status = B_OK;

    if(strcmp(reinterpret_cast<const char*>(in_what), USER_UBS) == 0)
        status = load_userbootscript(out_data);
    else if(strcmp(reinterpret_cast<const char*>(in_what), USER_PROF_ENV) == 0)
        status = load_usertermprofileenv(out_data);
    else {
        *out_data = "";
        status = B_ENTRY_NOT_FOUND;
    }

    return status;
}

status_t save_data(void* in_what, const char* in_data, int length)
{
    status_t status = B_OK;

    if(strcmp(reinterpret_cast<const char*>(in_what), USER_UBS) == 0)
        status = save_userbootscript(in_data, length);
    else if(strcmp(reinterpret_cast<const char*>(in_what), USER_PROF_ENV) == 0)
        status = save_usertermprofileenv(in_data, length);
    else
        status = B_ENTRY_NOT_FOUND;

    return status;
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

	if((dataread = file.Write((void*)in_data, in_length)) < 0)
		return B_ERROR;

	return B_OK;
}

// #pragma mark - UserBootscript

status_t create_userbootscript()
{
	BString data;
	default_userbootscript(&data);
	size_t size = strlen(data.String());
	return save_userbootscript(data.String(), size);
}

status_t load_userbootscript(BString* data)
{
    return load_data_from_file(USER_UBS, data);
}

void default_userbootscript(BString* data)
{
    data->SetTo("");
    data->Append("#!/bin/sh\n")
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

status_t save_userbootscript(const char* data, int length)
{
    return save_data_to_file(USER_UBS, data, length);
}

// #pragma mark - Autolaunch

status_t load_userautolaunch(std::vector<autolaunch_entry>& entrylist)
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

status_t create_userautolaunch_link(BString origin)
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

status_t copy_to_userautolaunch(BString origin)
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

status_t create_sh_script_userautolaunch(BString name, BString shell)
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

// #pragma mark - UserSetupEnvironment

status_t load_usersetupenvironment(std::vector<environment_entry>& entrylist)
{
    status_t status = B_OK;

    BFile usenvfile(USER_USENV, B_READ_ONLY);
    if((status = usenvfile.InitCheck()) != B_OK) {
        fprintf(stderr, "There is no current UserSetupEnvironment.\n");
        return status;
    }

    off_t size;
    usenvfile.GetSize(&size);
    BString buffer;
    usenvfile.Read(buffer.LockBuffer(size), size);

    std::stringstream strbuffer(buffer.String());
    std::string line;
    // std::regex envvar_rgx("^(#)?\\s*export\\s+(\\w+)\\s*=\\s*(\"([^\"]*)\"|\\S+)\\s*$");
    std::regex envvar_rgx("^(#)?\\s*export\\s+(\\w+)\\s*=\\s*(?:\"?([^\"]*)\"?)\\s*$");
    environment_entry entry;

    while(std::getline(strbuffer, line)) {
        if(std::regex_match(line, envvar_rgx)) {
            std::smatch match;
            if(std::regex_search(line, match, envvar_rgx)) {
                if(match[1].str() == "#") {
                    entry.enabled = false;
                    entry.name = match[2].str().c_str();
                    entry.value = match[3].str().c_str();
                }
                else {
                    entry.enabled = true;
                    entry.name = match[2].str().c_str();
                    entry.value = match[3].str().c_str();
                }
                entrylist.push_back(entry);
            }
        }
        else
            fprintf(stderr, "Regex failed in match.\n");
    }

    return status;
}

status_t add_to_usersetupenvironment(environment_entry entry, std::vector<environment_entry>& entrylist)
{
    status_t status = B_OK;
    bool needQuoting = false, needEOL = false;

    BFile file(USER_USENV, B_READ_WRITE | B_CREATE_FILE | B_OPEN_AT_END);
    if((status = file.InitCheck()) != B_OK)
        return status;

    const char* str = entry.value.String();
    while(*str != '\0' && !is_space(*str))
        str++;
    if(*str != '\0')
        needQuoting = true;

    char lastchar;
    file.Seek(-1, SEEK_END);
    file.Read(&lastchar, 1);
    if(lastchar != '\n')
        needEOL = true;

    BString buffer;
    buffer.Append(needEOL ? "\n" : "" )
          .Append(entry.enabled ? "" : "# ")
          .Append("export ")
          .Append(entry.name)
          .Append("=")
          .Append(needQuoting ? "\"" : "" )
          .Append(entry.value)
          .Append(needQuoting ? "\"" : "" )
          .Append("\n");
    off_t position = file.Position();
    size_t size = buffer.Length();
    file.WriteAt(position, buffer, size);

    return status;
}

status_t edit_entry_usersetupenvironment(BString key, environment_entry newdata, std::vector<environment_entry>& entrylist)
{
    status_t status = B_OK;

    fprintf(stderr, "New key to set: %s. New value: %s.\n", newdata.name.String(), newdata.value.String());
    BFile file(USER_USENV, B_READ_WRITE);
    if((status = file.InitCheck()) != B_OK)
        return status;

    off_t size;
    file.GetSize(&size);
    char* buffer = new char[size +1];
    file.Read(buffer, size);
    buffer[size] = '\0';

    std::stringstream iss(buffer);
    std::stringstream oss;
    std::string line;
    std::string rgx_str("^(#)?\\s*export\\s+");
    rgx_str.append(key).append("\\s*=\\s*(?:\"?([^\"]*)\"?)\\s*$");
    std::regex _rgx(rgx_str);

    while(std::getline(iss, line)) {
        if(!std::regex_match(line, _rgx))
            oss << line << std::endl;
        else if(std::regex_match(line, _rgx)){
            oss << (newdata.enabled ? "" : "# ")
            << "export "
            << newdata.name.String()
            << "="
            << newdata.value.String()
            << std::endl;
        }
    }

    BString outdata(oss.str().c_str());
    file.Seek(0, SEEK_SET);
    file.SetTo(USER_USENV, B_READ_WRITE | B_ERASE_FILE);
    file.Write(outdata.String(), outdata.Length());
    file.SetSize(outdata.Length());
    delete[] buffer;
    return status;
}

status_t delete_entry_usersetupenvironment(BString key, std::vector<environment_entry>& entrylist)
{
    status_t status = B_OK;
    BFile file(USER_USENV, B_READ_ONLY);
    if((status = file.InitCheck()) != B_OK)
        return status;

    off_t fsize;
    file.GetSize(&fsize);
    char* buffer = new char[fsize +1 ];
    file.Read(buffer, fsize);
    buffer[fsize] = '\0';

    std::stringstream ss(buffer);
    std::stringstream oss;
    std::string line;
    std::string rgx_str("^.*");
    rgx_str.append(key).append(".*$");
    std::regex _rgx(rgx_str);
    while(std::getline(ss, line)) {
        if(!std::regex_match(line, _rgx))
            oss << line << std::endl;
    }

    BString outdata(oss.str().c_str());
    file.SetTo(USER_USENV, B_READ_WRITE | B_ERASE_FILE);
    file.Write(outdata.String(), outdata.Length());

    delete[] buffer;
    return status;
}

status_t create_usersetupenvironment()
{
    status_t status = B_OK;

    BFile file(USER_USENV, B_READ_WRITE | B_CREATE_FILE);
    if((status = file.InitCheck()) != B_OK)
        return status;

    BString data;
    default_usersetupenvironment(&data);
    size_t size = strlen(data.String());
    printf("length %ld\n", size);
    file.Write(data.String(), size);
    file.Unset();

    return status;
}

void default_usersetupenvironment(BString *data)
{
    data->SetTo("");
    data->Append("# Quick start for file UserSetupEnvironment.  Copy or rename this file as\n")
         .Append("#	\"/boot/home/config/settings/boot/UserSetupEnvironment\".\n")
         .Append("# Add custom environment variables to initialize\n")
         .Append("# This file is a standard bash script.  For more information regarding shell\n")
         .Append("# scripts, refer to any online documentation for \"bash scripting\"\n")
         .Append("\n")
         .Append("# Set important variables\n")
         .Append("# export IS_COMPUTER_ON=1\n")
         .Append("\n");
}

// #pragma mark - profile

status_t create_usertermprofileenv()
{
	BString data;
	default_usertermprofileenv(&data);
	size_t size = strlen(data.String());
	return save_usertermprofileenv(data.String(), size);
}

void default_usertermprofileenv(BString *out_data)
{
	*out_data = "";
}

status_t load_usertermprofileenv(BString* out_data)
{
	return load_data_from_file(USER_PROF_ENV, out_data);
}

status_t save_usertermprofileenv(const char* data, int length)
{
    return save_data_to_file(USER_PROF_ENV, data, length);
}
