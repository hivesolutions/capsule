// Hive Capsule Installer
// Copyright (C) 2008 Hive Solutions Lda.
//
// This file is part of Hive Capsule Installer.
//
// Hive Capsule Installer is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// Hive Capsule Installer is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with Hive Capsule Installer. If not, see <http://www.gnu.org/licenses/>.

// __author__    = João Magalhães <joamag@hive.pt>
// __version__   = 1.0.0
// __revision__  = $LastChangedRevision$
// __date__      = $LastChangedDate$
// __copyright__ = Copyright (c) 2008 Hive Solutions Lda.
// __license__   = GNU General Public License (GPL), Version 3

#include "stdafx.h"

#include "logic/logic.h"
#include "ui/ui.h"
#include "main.h"

typedef enum Operations_e {
    UNSET = 1,
    HELP,
    RUN,
    DUPLICATE,
    APPEND,
    EXTEND,
    POP,
    DUMP
} Operations;

int help(char **argv, int argc, HINSTANCE handlerInstance, int nCmdShow) {
    MessageBox(NULL, HELP_SUPPORT_MESSAGE, DEFAULT_WINDOW_TITLE, MB_ICONINFORMATION | MB_OK);
    return 0;
}




HRESULT CreateLink(LPCSTR lpszPathObj, LPCSTR lpszPathLink, LPCSTR workingDirectory, LPCSTR lpszDesc)
{
    HRESULT hres;
    IShellLink* psl;

    // Get a pointer to the IShellLink interface. It is assumed that CoInitialize
    // has already been called.
    hres = CoCreateInstance(CLSID_ShellLink, NULL, CLSCTX_INPROC_SERVER, IID_IShellLink, (LPVOID*)&psl);
    if(SUCCEEDED(hres))
    {
        IPersistFile* ppf;

        // Set the path to the shortcut target and add the description.
        psl->SetPath(lpszPathObj);
        psl->SetDescription(lpszDesc);
        psl->SetWorkingDirectory(workingDirectory);

        // Query IShellLink for the IPersistFile interface, used for saving the
        // shortcut in persistent storage.
        hres = psl->QueryInterface(IID_IPersistFile, (LPVOID*)&ppf);

        if(SUCCEEDED(hres)) {
            WCHAR wsz[MAX_PATH];

            // Ensure that the string is Unicode.
            MultiByteToWideChar(CP_ACP, 0, lpszPathLink, -1, wsz, MAX_PATH);

            // Add code here to check return value from MultiByteWideChar
            // for success.

            // Save the link by calling IPersistFile::Save.
            hres = ppf->Save(wsz, TRUE);
            ppf->Release();
        }
        psl->Release();
    }
    return hres;
}





int run(char **argv, int argc, HINSTANCE handlerInstance, int nCmdShow) {
    // shows a message box asking for confirmation
    int returnValue = MessageBox(
        NULL,
        "You are going to install capsule and the dependencies\n Continue ?",
        DEFAULT_WINDOW_TITLE,
        MB_ICONWARNING | MB_OKCANCEL
    );

    // in case the return from the message box is cancel, the installer
    // process must be aborted immediately
    if(returnValue == IDCANCEL) {
        // returns in error
        return -1;
    }

    try {
        // tries to retrieve the python install path
        std::string value = JBPython::getInstallPath(std::string("2.7ISOT E MAU"));
    } catch(char *) {
        // creates a new downloader instance and creates the
        // download window (with the installation controls
        CSDownloader downloader = CSDownloader();
        downloader.createDownloadWindow(handlerInstance, nCmdShow);

        // allocates space for the generic download item to be populated
        // across the various files
        CSDownloadItem downloadItem;

        // retrieves the current data structure from the current
        // execution process file (from the internal resources)
        struct Data_t *data = CSData::getData();

        // print as debug message into the logger
        JBLogger::getLogger("setup")->debug("Inserting %d data files into the downloader ...", data->numberFiles);

        // iterates over all the data files to register them for
        // downloading om the downloader
        for(size_t index = 0; index < data->numberFiles; index++) {
            // retrieves the current iteration data file unpacks the various
            // components of it, creates the download item with them as adds
            // the download item to the downloader object
            struct DataFile_t *dataFile = &data->dataFiles[index];
            if(dataFile->type == REMOTE) { downloadItem = CSDownloadItem(std::string(dataFile->name), std::string(dataFile->description), std::string(dataFile->url)); }
            else { downloadItem = CSDownloadItem(std::string(dataFile->name), std::string(dataFile->description), dataFile->buffer, dataFile->bufferSize); }
            downloader.addDownloadItem(downloadItem);

            // downloads the various files from their respective remote storage locations
            // in case the connection fails an exception is thrown, then unpacks their
            // file into the appropriate locations
            downloader.downloadFiles();
            std::string &targetPath = downloader.unpackFiles();

            // prints adebug message into the logger
            JBLogger::getLogger("setup")->debug("Unpacked files into '%s'", targetPath.c_str());



            // THIS IS COMPLETLY HARDCODED !!!! (SOFTCODE IT)
            char programsPath[MAX_PATH];
            ExpandEnvironmentStrings("%PROGRAMFILES%", programsPath, MAX_PATH);
            sprintf_s(programsPath, "%s\\%s", programsPath, dataFile->name);


            // prints adebug message into the logger
            JBLogger::getLogger("setup")->debug("Deploying files into '%s' ...", programsPath);



            // deploys the files into the destination directory and then
            // deletes the temporary files for the current data file
            downloader.deployFiles(programsPath);
            downloader.deleteTemporaryFiles();

            // THIS IS COMPLETLY HARDCODED !!!! (SOFTCODE IT)

            char path[MAX_PATH];

            char filePath[MAX_PATH];

            sprintf_s(filePath, "%s\\viriatum.exe", programsPath);

            ExpandEnvironmentStrings("%USERPROFILE%", path, MAX_PATH);
            sprintf_s(path, "%s\\Desktop\\Viriatum.lnk", path);


            CoInitialize(NULL);
            CreateLink(filePath, path, programsPath, dataFile->name);
        }

        // releases the data structure (avoids memory leaking)
        // this is required because the data structure is in
        // control by us
        free(data);
    }

    return 0;
}

int duplicate(char **argv, int argc, HINSTANCE handlerInstance, int nCmdShow) {
    // allocates space for the path to the target
    // capsule installer file to be created (cloned)
    char *targetPath;

    // allocates space for the file name of the current
    // executable file name then retrieves it (module name)
    char fileName[MAX_PATH];
    GetModuleFileName(NULL, fileName, MAX_PATH);

    // checks if the target path is specified in case
    // it's not the default naming is used instead
    if(argc > 2) { targetPath = argv[2]; }
    else { targetPath = DEFAULT_SETUP_NAME; }

    // prints an info message into the logger
    JBLogger::getLogger("setup")->info("Duplicating file into '%s'", targetPath);

    // executes the copy operation duplicating the current
    // executing file into a duplicate (replica)
    CopyFile(fileName, targetPath, FALSE);

    return 0;
}

int append(char **argv, int argc, HINSTANCE handlerInstance, int nCmdShow) {
    char *filePath;
    struct DataFile_t dataFile;
    char index = 0;

    if(argc > 5) { filePath = argv[2]; index++; }
    else { filePath = DEFAULT_SETUP_NAME; }

    // retrieves the various arguments from the command line
    // representing the various components of the data file
    char *name = argv[2 + index];
    char *description = argv[3 + index];
    char *url = argv[4 + index];

    // retrieves the appropriate sizes from the string values
    // for memory copy of their values
    size_t nameSize = strlen(name);
    size_t descriptionSize = strlen(description);
    size_t urlSize = strlen(url);

    // copies the various data file attributes into the data file
    // structure (populates the structure)
    memcpy(dataFile.name, name, nameSize + 1);
    memcpy(dataFile.description, description, descriptionSize + 1);
    memcpy(dataFile.url, url, urlSize + 1);

    // sets the buffer related attributes to their default values
    // (this will unset the buffer behavior in this data file)
    dataFile.buffer = NULL;
    dataFile.bufferSize = 0;
    dataFile.bufferOffset = 0;

    // sets the data file type as remote (data file must
    // be retrieved using http)
    dataFile.type = REMOTE;

    // adds the current data file to the current executable file
    // persists the value (should raise an exception on error)
    CSData::appendDataFile(filePath, &dataFile);

    return 0;
}

int extend(char **argv, int argc, HINSTANCE handlerInstance, int nCmdShow) {
    char *filePath;
    struct DataFile_t dataFile;
    char index = 0;

    if(argc > 5) { filePath = argv[2]; index++; }
    else { filePath = DEFAULT_SETUP_NAME; }

    // retrieves the various arguments from the command line
    // representing the various components of the data file
    char *name = argv[2 + index];
    char *description = argv[3 + index];
    char *dataPath = argv[4 + index];

    // retrieves the appropriate sizes from the string values
    // for memory copy of their values
    size_t nameSize = strlen(name);
    size_t descriptionSize = strlen(description);

    // opens the data file from the retrieved data path
    FILE *file;
    fopen_s(&file, dataPath, "rb");

    // prints an info message into the logger
    JBLogger::getLogger("setup")->info("Reading data file '%s'", dataPath);

    // retrieves the size of teh data file
    // this is going to be used to set the
    // buffer size
    fseek(file, 0, SEEK_END);
    size_t bufferSize = ftell(file);
    fseek(file, 0, SEEK_SET);

    // allocates space for the file buffer and then
    // reads the file into it (complete file read)
    char *buffer = (char *) malloc(bufferSize);
    fread(buffer, sizeof(char), bufferSize, file);

    // closes the buffer file (no leaking)
    fclose(file);

    // copies the various data file attributes into the data file
    // structure (populates the structure)
    memcpy(dataFile.name, name, nameSize + 1);
    memcpy(dataFile.description, description, descriptionSize + 1);

    // updates the url value to an empty string and then sets the
    // proper buffer values, the offset is not set yet
    dataFile.url[0] = '\0';
    dataFile.buffer = buffer;
    dataFile.bufferSize = bufferSize;
    dataFile.bufferOffset = 0;

    // sets the data file type as remote (data file is
    // stored in the file resources)
    dataFile.type = LOCAL;

    // adds the current data file to the current executable file
    // persists the value (should raise an exception on error)
    CSData::appendDataFile(filePath, &dataFile);

    // releases the buffer (no more need to use it)
    // avoids memory leaks
    free(buffer);

    return 0;
}

int pop(char **argv, int argc, HINSTANCE handlerInstance, int nCmdShow) {
    char *filePath;

    if(argc > 2) { filePath = argv[2]; }
    else { filePath = DEFAULT_SETUP_NAME; }

    CSData::popDataFile(filePath);

    return 0;
}

int dump(char **argv, int argc, HINSTANCE handlerInstance, int nCmdShow) {
    char *filePath;

    if(argc > 2) { filePath = argv[2]; }
    else { filePath = DEFAULT_DUMP_NAME; }

    // prints an info message into the logger
    JBLogger::getLogger("setup")->info("Dumping data into '%s'", filePath);

    std::ofstream dumpFile;
    dumpFile.open(filePath);
    CSData::printData(dumpFile);
    dumpFile.close();

    return 0;
}

int call(enum Operations_e operation, char **argv, int argc, HINSTANCE handlerInstance, int nCmdShow) {
    switch(operation) {
        case HELP:
            return help(argv, argc, handlerInstance, nCmdShow);
            break;

        case RUN:
            return run(argv, argc, handlerInstance, nCmdShow);
            break;

        case DUPLICATE:
            return duplicate(argv, argc, handlerInstance, nCmdShow);
            break;

        case APPEND:
            return append(argv, argc, handlerInstance, nCmdShow);
            break;

        case EXTEND:
            return extend(argv, argc, handlerInstance, nCmdShow);
            break;

        case POP:
            return pop(argv, argc, handlerInstance, nCmdShow);
            break;

        case DUMP:
            return dump(argv, argc, handlerInstance, nCmdShow);
            break;
    }

    return 0;
}

int APIENTRY _tWinMain(HINSTANCE handlerInstance, HINSTANCE hPrevInstance, LPTSTR lpCmdLine, int nCmdShow) {
    // allocates space for the command line arguments values
    // and for the variable that will hold the argument count
    char **argv;
    int argc;

    // sets the handler instance global value, for latter
    // global reference (must be accessible by everyone)
    hInst = handlerInstance;

    // starts the console to allow output and input
    // from the default interaction mechanisms
    CSUtil::startConsole();

    // starts the logger sub system, the logger level
    // is dependent in thje current run mode, debug mode
    // sets an higher level of verbosity
    CSUtil::sartLogger();

    // initializes the common controls and
    // registers the (handler instance) class
    InitCommonControls();
    registerClass(handlerInstance);

    // creates the variable that will hold the operation
    // to be executed for the current context
    enum Operations_e operation = UNSET;

    // retrieves the current command line and unpacks it into
    // the array of commands provided, it's important to respect
    // the default string escaping sequences
    char *commandLine = GetCommandLineA();
    argv = JBWindows::commandLineToArgv(commandLine, &argc);

    try {
        if(argc == 1) {
            operation = RUN;
        } else {
            if(!strcmp(argv[1], "help")) { operation = HELP; }
            else if(!strcmp(argv[1], "run")) { operation = RUN; }
            else if(!strcmp(argv[1], "duplicate")) { operation = DUPLICATE; }
            else if(!strcmp(argv[1], "clone")) { operation = DUPLICATE; }
            else if(!strcmp(argv[1], "append")) { operation = APPEND; }
            else if(!strcmp(argv[1], "extend")) { operation = EXTEND; }
            else if(!strcmp(argv[1], "pop")) { operation = POP; }
            else if(!strcmp(argv[1], "dump")) { operation = DUMP; }
            else { throw "Invalid command line option"; }
        }

        call(operation, argv, argc, handlerInstance, nCmdShow);
    } catch(char *exception) {
        int returnValue = MessageBox(NULL, (std::string("Error:\n") + exception).c_str(), "Installation error", MB_ICONERROR | MB_OK);
        return -1;
    }

    // stops the console system, releasing all the remaining
    // resources in the associated system
    CSUtil::stopConsole();

    // returns normally
    return 0;
}
