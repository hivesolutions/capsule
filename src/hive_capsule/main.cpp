// Hive Capsule Installer
// Copyright (c) 2008-2016 Hive Solutions Lda.
//
// This file is part of Hive Capsule Installer.
//
// Hive Capsule Installer is free software: you can redistribute it and/or modify
// it under the terms of the Apache License as published by the Apache
// Foundation, either version 2.0 of the License, or (at your option) any
// later version.
//
// Hive Capsule Installer is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// Apache License for more details.
//
// You should have received a copy of the Apache License along with
// Hive Capsule Installer. If not, see <http://www.apache.org/licenses/>.

// __author__    = João Magalhães <joamag@hive.pt>
// __version__   = 1.0.0
// __revision__  = $LastChangedRevision$
// __date__      = $LastChangedDate$
// __copyright__ = Copyright (c) 2008-2016 Hive Solutions Lda.
// __license__   = Apache License, Version 2.0

#include "stdafx.h"

#include "logic/logic.h"
#include "ui/ui.h"
#include "main.h"

enum OperationsType {
    UNSET = 1,
    HELP,
    RUN,
    DUPLICATE,
    APPEND,
    EXTEND,
    POP,
    DUMP,
    TEST
};

int Help(char **argv, int argc, HINSTANCE handler_instance, int cmd_show) {
    MessageBox(NULL, HELP_SUPPORT_MESSAGE, DEFAULT_WINDOW_TITLE, MB_ICONINFORMATION | MB_OK);
    return 0;
}




HRESULT CreateLink(
    LPCSTR path_object,
    LPCSTR path_link,
    LPCSTR working_directory,
    LPCSTR description
) {
    // allocates space for the various files that are going to be
    // used for the creation of the shortcut link in storage
    HRESULT result;
    IShellLink *shell_link;
    IPersistFile *persist_file;

    // tries to retrieve a pointer to the shell interface, that
    // is going to be used for the creation of the link (shortcuty)
    result = CoCreateInstance(
        CLSID_ShellLink,
        NULL,
        CLSCTX_INPROC_SERVER,
        IID_IShellLink,
        (LPVOID *) &shell_link
    );
    if(SUCCEEDED(result) == FALSE) { return result; }

    // sets the path to the shortcut target and adds the description
    // this should correctly identify the shortcut to be created
    shell_link->SetPath(path_object);
    shell_link->SetDescription(description);
    shell_link->SetWorkingDirectory(working_directory);

    // queries the shell link for the persist file interface, used for
    // saving the shortcut in persistent storage
    result = shell_link->QueryInterface(IID_IPersistFile, (LPVOID *) &persist_file);
    if(SUCCEEDED(result) == FALSE) {
        shell_link->Release();
        return result;
    }

    // allocates space for the wide version of the path link
    // that is going to be used to store the shortcut data
    WCHAR path_link_w[MAX_PATH];

    // ensures that the string isof type unicode, required for
    // the creation of the link file (shortcut) and then saves
    // it, persisting it to the current user's interface
    MultiByteToWideChar(CP_ACP, 0, path_link, -1, path_link_w, MAX_PATH);
    result = persist_file->Save(path_link_w, TRUE);

    // releases the complete set of resources and returns the
    // last result to the caller method/function
    persist_file->Release();
    shell_link->Release();
    return result;
}





int Run(char **argv, int argc, HINSTANCE handler_instance, int cmd_show) {
    // shows a message box asking for confirmation
    int return_value = MessageBox(
        NULL,
        "You are going to install capsule and the dependencies\n Continue ?",
        DEFAULT_WINDOW_TITLE,
        MB_ICONWARNING | MB_OKCANCEL
    );

    // in case the return from the message box is cancel, the installer
    // process must be aborted immediately
    if(return_value == IDCANCEL) {
        // returns in error
        return -1;
    }

    try {
        // tries to retrieve the python install path
        std::string value = JBPython::GetInstallPath(std::string("2.7ISOT E MAU"));
    } catch(char *) {
        // creates a new downloader instance and creates the
        // download window (with the installation controls
        CSDownloader downloader = CSDownloader();
        downloader.CreateDownloadWindow(handler_instance, cmd_show);

        // allocates space for the generic download item to be populated
        // across the various files
        CSDownloadItem download_item;

        // retrieves the current data structure from the current
        // execution process file (from the internal resources)
        struct DataType *data = CSData::GetData();

        // print as Debug message into the logger
        JBLogger::GetLogger("setup")->Debug("Inserting %d data files into the downloader ...", data->number_files);

        // iterates over all the data files to register them for
        // downloading om the downloader
        for(size_t index = 0; index < data->number_files; index++) {
            // retrieves the current iteration data file unpacks the various
            // components of it, creates the download item with them as adds
            // the download item to the downloader object
            struct DataFileType *data_file = &data->data_files[index];
            if(data_file->type == REMOTE) { download_item = CSDownloadItem(std::string(data_file->name), std::string(data_file->description), std::string(data_file->url)); }
            else { download_item = CSDownloadItem(std::string(data_file->name), std::string(data_file->description), data_file->buffer, data_file->buffer_size); }
            downloader.AddDownloadItem(download_item);

            // downloads the various files from their respective remote storage locations
            // in case the connection fails an exception is thrown, then unpacks their
            // file into the appropriate locations
            downloader.DownloadFiles();
            std::string &target_path = downloader.UnpackFiles();

            // prints adebug message into the logger
            JBLogger::GetLogger("setup")->Debug("Unpacked files into '%s'", target_path.c_str());



            // THIS IS COMPLETLY HARDCODED !!!! (SOFTCODE IT)
            char programs_path[MAX_PATH];
            ExpandEnvironmentStrings("%PROGRAMFILES%", programs_path, MAX_PATH);
            sprintf_s(programs_path, "%s\\%s", programs_path, data_file->name);


            // prints adebug message into the logger
            JBLogger::GetLogger("setup")->Debug("Deploying files into '%s' ...", programs_path);



            // deploys the files into the destination directory and then
            // deletes the temporary files for the current data file
            downloader.DeployFiles(programs_path);
            downloader.DeleteTemporaryFiles();

            // THIS IS COMPLETLY HARDCODED !!!! (SOFTCODE IT)

            char path[MAX_PATH];

            char file_path[MAX_PATH];

            sprintf_s(file_path, "%s\\viriatum.exe", programs_path);

            ExpandEnvironmentStrings("%USERPROFILE%", path, MAX_PATH);
            sprintf_s(path, "%s\\Desktop\\Viriatum.lnk", path);


            CoInitialize(NULL);
            CreateLink(file_path, path, programs_path, data_file->name);
        }

        // releases the data structure (avoids memory leaking)
        // this is required because the data structure is in
        // control by us
        free(data);
    }

    return 0;
}

int Duplicate(char **argv, int argc, HINSTANCE handler_instance, int cmd_show) {
    // allocates space for the path to the target
    // capsule installer file to be created (cloned)
    char *target_path;

    // allocates space for the file name of the current
    // executable file name then retrieves it (module name)
    char file_name[MAX_PATH];
    GetModuleFileName(NULL, file_name, MAX_PATH);

    // checks if the target path is specified in case
    // it's not the default naming is used instead
    if(argc > 2) { target_path = argv[2]; }
    else { target_path = DEFAULT_SETUP_NAME; }

    // prints an Info message into the logger
    JBLogger::GetLogger("setup")->Info("Duplicating file into '%s'", target_path);

    // executes the copy operation duplicating the current
    // executing file into a duplicate (replica)
    CopyFile(file_name, target_path, FALSE);

    return 0;
}

int Append(char **argv, int argc, HINSTANCE handler_instance, int cmd_show) {
    char *file_path;
    struct DataFileType data_file;
    char index = 0;

    if(argc > 5) { file_path = argv[2]; index++; }
    else { file_path = DEFAULT_SETUP_NAME; }

    // retrieves the various arguments from the command line
    // representing the various components of the data file
    char *name = argv[2 + index];
    char *description = argv[3 + index];
    char *url = argv[4 + index];

    // retrieves the appropriate sizes from the string values
    // for memory copy of their values
    size_t name_size = strlen(name);
    size_t description_size = strlen(description);
    size_t url_size = strlen(url);

    // copies the various data file attributes into the data file
    // structure (populates the structure)
    memcpy(data_file.name, name, name_size + 1);
    memcpy(data_file.description, description, description_size + 1);
    memcpy(data_file.url, url, url_size + 1);

    // sets the buffer related attributes to their default values
    // (this will unset the buffer behavior in this data file)
    data_file.buffer = NULL;
    data_file.buffer_size = 0;
    data_file.buffer_offset = 0;

    // sets the data file type as remote (data file must
    // be retrieved using http)
    data_file.type = REMOTE;

    // adds the current data file to the current executable file
    // persists the value (should raise an exception on error)
    CSData::AppendDataFile(file_path, &data_file);

    return 0;
}

int Extend(char **argv, int argc, HINSTANCE handler_instance, int cmd_show) {
    char *file_path;
    struct DataFileType data_file;
    char index = 0;

    if(argc > 5) { file_path = argv[2]; index++; }
    else { file_path = DEFAULT_SETUP_NAME; }

    // retrieves the various arguments from the command line
    // representing the various components of the data file
    char *name = argv[2 + index];
    char *description = argv[3 + index];
    char *data_path = argv[4 + index];

    // retrieves the appropriate sizes from the string values
    // for memory copy of their values
    size_t name_size = strlen(name);
    size_t description_size = strlen(description);

    // opens the data file from the retrieved data path
    FILE *file;
    fopen_s(&file, data_path, "rb");

    // prints an Info message into the logger
    JBLogger::GetLogger("setup")->Info("Reading data file '%s'", data_path);

    // retrieves the size of teh data file
    // this is going to be used to set the
    // buffer size
    fseek(file, 0, SEEK_END);
    size_t buffer_size = ftell(file);
    fseek(file, 0, SEEK_SET);

    // allocates space for the file buffer and then
    // reads the file into it (complete file read)
    char *buffer = (char *) malloc(buffer_size);
    fread(buffer, sizeof(char), buffer_size, file);

    // closes the buffer file (no leaking)
    fclose(file);

    // copies the various data file attributes into the data file
    // structure (populates the structure)
    memcpy(data_file.name, name, name_size + 1);
    memcpy(data_file.description, description, description_size + 1);

    // updates the url value to an empty string and then sets the
    // proper buffer values, the offset is not set yet
    data_file.url[0] = '\0';
    data_file.buffer = buffer;
    data_file.buffer_size = buffer_size;
    data_file.buffer_offset = 0;

    // sets the data file type as remote (data file is
    // stored in the file resources)
    data_file.type = LOCAL;

    // adds the current data file to the current executable file
    // persists the value (should raise an exception on error)
    CSData::AppendDataFile(file_path, &data_file);

    // releases the buffer (no more need to use it)
    // avoids memory leaks
    free(buffer);

    return 0;
}

int Pop(char **argv, int argc, HINSTANCE handler_instance, int cmd_show) {
    char *file_path;

    if(argc > 2) { file_path = argv[2]; }
    else { file_path = DEFAULT_SETUP_NAME; }

    CSData::PopDataFile(file_path);

    return 0;
}

int Dump(char **argv, int argc, HINSTANCE handler_instance, int cmd_show) {
    char *file_path;

    if(argc > 2) { file_path = argv[2]; }
    else { file_path = DEFAULT_DUMP_NAME; }

    // prints an Info message into the logger
    JBLogger::GetLogger("setup")->Info("Dumping data into '%s'", file_path);

    std::ofstream dump_file;
    dump_file.open(file_path);
    CSData::PrintData(dump_file);
    dump_file.close();

    return 0;
}

int Test(char **argv, int argc, HINSTANCE handler_instance, int cmd_show) {
    return 0;
}

int Call(enum OperationsType operation, char **argv, int argc, HINSTANCE handler_instance, int cmd_show) {
    switch(operation) {
        case HELP:
            return Help(argv, argc, handler_instance, cmd_show);
            break;

        case RUN:
            return Run(argv, argc, handler_instance, cmd_show);
            break;

        case DUPLICATE:
            return Duplicate(argv, argc, handler_instance, cmd_show);
            break;

        case APPEND:
            return Append(argv, argc, handler_instance, cmd_show);
            break;

        case EXTEND:
            return Extend(argv, argc, handler_instance, cmd_show);
            break;

        case POP:
            return Pop(argv, argc, handler_instance, cmd_show);
            break;

        case DUMP:
            return Dump(argv, argc, handler_instance, cmd_show);
            break;

        case TEST:
            return Test(argv, argc, handler_instance, cmd_show);
            break;
    }

    return 0;
}

int APIENTRY _tWinMain(HINSTANCE handler_instance, HINSTANCE prev_instance, LPTSTR cmd_line, int cmd_show) {
    // allocates space for the command line arguments values
    // and for the variable that will hold the argument count
    char **argv;
    int argc;

    // sets the handler instance global value, for latter
    // global reference (must be accessible by everyone)
    instance = handler_instance;

    // starts the console to allow output and input
    // from the default interaction mechanisms
    CSUtil::StartConsole();

    // starts the logger sub system, the logger level
    // is dependent in thje current run mode, Debug mode
    // sets an higher level of verbosity
    CSUtil::SartLogger();

    // initializes the common controls and
    // registers the (handler instance) class
    InitCommonControls();
    RegisterClass(handler_instance);

    // creates the variable that will hold the operation
    // to be executed for the current context
    enum OperationsType operation = UNSET;

    // retrieves the current command line and unpacks it into
    // the array of commands provided, it's important to respect
    // the default string escaping sequences
    char *command_line = GetCommandLineA();
    argv = JBWindows::CommandLineToArgv(command_line, &argc);

    try {
        if(argc == 1) {
            operation = RUN;
        } else {
            if(!strcmp(argv[1], "Help")) { operation = HELP; }
            else if(!strcmp(argv[1], "run")) { operation = RUN; }
            else if(!strcmp(argv[1], "duplicate")) { operation = DUPLICATE; }
            else if(!strcmp(argv[1], "clone")) { operation = DUPLICATE; }
            else if(!strcmp(argv[1], "append")) { operation = APPEND; }
            else if(!strcmp(argv[1], "extend")) { operation = EXTEND; }
            else if(!strcmp(argv[1], "pop")) { operation = POP; }
            else if(!strcmp(argv[1], "dump")) { operation = DUMP; }
            else if(!strcmp(argv[1], "test")) { operation = TEST; }
            else { throw "Invalid command line option"; }
        }

        Call(operation, argv, argc, handler_instance, cmd_show);
    } catch(char *exception) {
        int return_value = MessageBox(
            NULL,
            (std::string("Error:\n") + exception).c_str(),
            "Installation error",
            MB_ICONERROR | MB_OK
        );
        return -1;
    }

    // stops the console system, releasing all the remaining
    // resources in the associated system
    CSUtil::StopConsole();

    // returns normally
    return 0;
}
