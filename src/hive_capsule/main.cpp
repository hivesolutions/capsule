// Hive Colony Framework
// Copyright (C) 2008 Hive Solutions Lda.
//
// This file is part of Hive Colony Framework.
//
// Hive Colony Framework is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// Hive Colony Framework is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with Hive Colony Framework. If not, see <http://www.gnu.org/licenses/>.

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
    RUN,
    DUPLICATE,
    APPEND,
    POP,
    DUMP
} Operations;

LPSTR* CommandLineToArgvA(LPSTR lpCmdLine, INT *pNumArgs)
{
    int retval;
    retval = MultiByteToWideChar(CP_ACP, MB_ERR_INVALID_CHARS, lpCmdLine, -1, NULL, 0);
    if (!SUCCEEDED(retval))
        return NULL;

    LPWSTR lpWideCharStr = (LPWSTR)malloc(retval * sizeof(WCHAR));
    if (lpWideCharStr == NULL)
        return NULL;

    retval = MultiByteToWideChar(CP_ACP, MB_ERR_INVALID_CHARS, lpCmdLine, -1, lpWideCharStr, retval);
    if (!SUCCEEDED(retval))
    {
        free(lpWideCharStr);
        return NULL;
    }

    int numArgs;
    LPWSTR* args;
    args = CommandLineToArgvW(lpWideCharStr, &numArgs);
    free(lpWideCharStr);
    if (args == NULL)
        return NULL;

    int storage = numArgs * sizeof(LPSTR);
    for (int index = 0; index < numArgs; ++index)
    {
        BOOL lpUsedDefaultChar = FALSE;
        retval = WideCharToMultiByte(CP_ACP, 0, args[index], -1, NULL, 0, NULL, &lpUsedDefaultChar);
        if (!SUCCEEDED(retval))
        {
            LocalFree(args);
            return NULL;
        }

        storage += retval;
    }

    LPSTR* result = (LPSTR*)LocalAlloc(LMEM_FIXED, storage);
    if (result == NULL)
    {
        LocalFree(args);
        return NULL;
    }

    int bufLen = storage - numArgs * sizeof(LPSTR);
    LPSTR buffer = ((LPSTR)result) + numArgs * sizeof(LPSTR);
    for (int index = 0; index < numArgs; ++index) {
        BOOL lpUsedDefaultChar = FALSE;
        retval = WideCharToMultiByte(CP_ACP, 0, args[index], -1, buffer, bufLen, NULL, &lpUsedDefaultChar);
        if(!SUCCEEDED(retval)) {
            LocalFree(result);
            LocalFree(args);
            return NULL;
        }

        result[index] = buffer;
        buffer += retval;
        bufLen -= retval;
    }

    LocalFree(args);

    *pNumArgs = numArgs;
    return result;
}

void sartLogger() {
    JBLogger *logger = JBLogger::getLogger("setup");
    JBLoggerFileHandler *fileHandler = new JBLoggerFileHandler(std::string("setup.log"));
    logger->addHandler(fileHandler);
    logger->setLevel(DEBUG);
    logger->info("Log system started");
}

void startConsole() {
    // allocates the console interface in the windows
    // sub system and attaches it to the curren process
    AllocConsole();

    // saves the various default stream files into temporary
    // variables to be able to use them as value references
    FILE *_stdin = stdin;
    FILE *_stdout = stdout;
    FILE *_stderr = stderr;

    // reopens the standard input, output and error files
    // and sets then to the console input and output systems
    freopen_s(&_stdin, "CONIN$", "rb", stdin);
    freopen_s(&_stdout, "CONOUT$", "wb", stdout);
    freopen_s(&_stderr, "CONOUT$", "wb", stderr);
}

void stopConsole() {
    FreeConsole();
}

int run(char **argv, int argc, HINSTANCE handlerInstance, int nCmdShow) {
    try {
        // shows a message box asking for confirmation
        int returnValue = MessageBox(NULL, "You are going to install colony and the dependencies\n Continue ?", "Colony Install", MB_ICONWARNING | MB_OKCANCEL);

        // in case the retur from the message box is cancel, the installer
        // process must be aborted immediately
        if(returnValue == IDCANCEL) {
            // returns in error
            return -1;
        }

        try {
            // tries to retrieve the python install path
            std::string value = JBPython::getInstallPath(std::string("2.7asdad"));
        } catch(char *) {
            // creates a new colony downloader instance and creates
            // the download window (with the installation controls
            CColonyDownloader colonyDownloader = CColonyDownloader();
            colonyDownloader.createDownloadWindow(handlerInstance, nCmdShow);

            struct Data_t *data = CData::getData();

			JBLogger::getLogger("setup")->debug("Inserting %d data files into the downloader ...", data->numberFiles);

            for(size_t index = 0; index < data->numberFiles; index++) {
                struct DataFile_t *dataFile = &data->dataFiles[index];
                CColonyDownloadItem downloadItem = CColonyDownloadItem(std::string(dataFile->name), std::string(dataFile->description), std::string(dataFile->url));
                colonyDownloader.addDownloadItem(downloadItem);
            }

            // downloads the various files from their respective remote storage locations
            // in case the connection fails an exception is thrown
            colonyDownloader.downloadFiles();

            // unpacks the files and retrieves the target path
            std::string &targetPath = colonyDownloader.unpackFiles();

            // deletes the temporary files
            colonyDownloader.deleteTemporaryFiles();
        }
    } catch (char *exception) {
        int returnValue = MessageBox(NULL, (std::string("Problem in installation:\n\n") + exception).c_str() , "Installation error", MB_ICONERROR | MB_OK);

        // returns in error
        return -1;
    }

    return 0;
}

int duplicate(char **argv, int argc, HINSTANCE handlerInstance, int nCmdShow) {
    // allocates space for the path to the target
    // capsule installer file to be created (cloned)
    char *targetPath;

    char szFileName[MAX_PATH];
    GetModuleFileName(NULL, szFileName, MAX_PATH);

    // checks if the target path is specified in case
    // it's not the default naming is used instead
    if(argc > 2) { targetPath = argv[2]; }
    else { targetPath = DEFAULT_SETUP_NAME; }

    // prints an info message into the logger
    JBLogger::getLogger("setup")->info("Duplicating file into '%s'", targetPath);

    // executes the copy operation duplicating the current
    // executing file into a duplicate (replica)
    CopyFile(szFileName, targetPath, FALSE);

    return 0;
}

int append(char **argv, int argc, HINSTANCE handlerInstance, int nCmdShow) {
    char *filePath;
    struct DataFile_t dataFile;
    char index = 0;

    if(argc > 5) { filePath = argv[2]; index++; }
    else { filePath = DEFAULT_SETUP_NAME; }

    char *name = argv[2 + index];
    char *description = argv[3 + index];
    char *url = argv[4 + index];

    size_t nameSize = strlen(name);
    size_t descriptionSize = strlen(description);
    size_t urlSize = strlen(url);

    memcpy(dataFile.name, name, nameSize + 1);
    memcpy(dataFile.description, description, descriptionSize + 1);
    memcpy(dataFile.url, url, urlSize + 1);

    CData::appendDataFile(filePath, &dataFile);

    return 0;
}

int pop(char **argv, int argc, HINSTANCE handlerInstance, int nCmdShow) {
    char *filePath;

    if(argc > 2) { filePath = argv[2]; }
    else { filePath = DEFAULT_SETUP_NAME; }

    CData::popDataFile(filePath);

    return 0;
}

int dump(char **argv, int argc, HINSTANCE handlerInstance, int nCmdShow) {
    char *filePath;

    if(argc > 2) { filePath = argv[3]; }
    else { filePath = DEFAULT_DUMP_NAME; }

    std::ofstream dumpFile;
    dumpFile.open(filePath);
    CData::printData(dumpFile);
    dumpFile.close();

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
    startConsole();

    // starts the logger sub system, the logger level
    // is dependent in thje current run mode, debug mode
    // sets an higher level of verbosity
    sartLogger();

    // initializes the common controls and
    // registers the (handler instance) class
    InitCommonControls();
    registerClass(handlerInstance);

    // creates the variable that will hold the operation
    // to be executed for the current context
    enum Operations_e operation = UNSET;

    argv = CommandLineToArgvA(GetCommandLineA(), &argc);

    try {
        if(argc == 1) {
            operation = RUN;
        } else {
            if(!strcmp(argv[1], "run")) { operation = RUN; }
            else if(!strcmp(argv[1], "duplicate")) { operation = DUPLICATE; }
            else if(!strcmp(argv[1], "clone")) { operation = DUPLICATE; }
            else if(!strcmp(argv[1], "append")) { operation = APPEND; }
            else if(!strcmp(argv[1], "pop")) { operation = POP; }
            else if(!strcmp(argv[1], "dump")) { operation = DUMP; }
            else { throw "Invalid command line option"; }
        }
    } catch(char *exception) {
        std::cout << exception;
        return -1;
    }

    switch(operation) {
        case RUN:
            return run(argv, argc, handlerInstance, nCmdShow);
            break;

        case DUPLICATE:
            return duplicate(argv, argc, handlerInstance, nCmdShow);
            break;

        case APPEND:
            return append(argv, argc, handlerInstance, nCmdShow);
            break;

        case POP:
            return pop(argv, argc, handlerInstance, nCmdShow);
            break;

        case DUMP:
            return dump(argv, argc, handlerInstance, nCmdShow);
            break;
    }

    // stops the console system, releasing all the remaining
    // resources in the associated system
    startConsole();

    // returns normally
    return 0;
}
