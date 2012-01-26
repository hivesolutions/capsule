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
	DUMP,
	POP,
	APPEND,
	DUPLICATE
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
    for (int i = 0; i < numArgs; ++ i)
    {
        BOOL lpUsedDefaultChar = FALSE;
        retval = WideCharToMultiByte(CP_ACP, 0, args[i], -1, NULL, 0, NULL, &lpUsedDefaultChar);
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
    for (int i = 0; i < numArgs; ++ i)
    {
        BOOL lpUsedDefaultChar = FALSE;
        retval = WideCharToMultiByte(CP_ACP, 0, args[i], -1, buffer, bufLen, NULL, &lpUsedDefaultChar);
        if (!SUCCEEDED(retval))
        {
            LocalFree(result);
            LocalFree(args);
            return NULL;
        }

        result[i] = buffer;
        buffer += retval;
        bufLen -= retval;
    }

    LocalFree(args);

    *pNumArgs = numArgs;
    return result;
}

int duplicate(char **argv, int argc, HINSTANCE handlerInstance, int nCmdShow) {
	char szFileName[MAX_PATH];
	GetModuleFileName(NULL, szFileName, MAX_PATH);

	CopyFile(szFileName, argv[2], FALSE);

	return 0;
}

int dump(char **argv, int argc, HINSTANCE handlerInstance, int nCmdShow) {
	char *filePath;

	if(argc > 2) {
		filePath = argv[3];
	} else {
		filePath = "dump.dat";
	}

	std::ofstream dumpFile;
	dumpFile.open(filePath);
	CData::printData(dumpFile);
	dumpFile.close();

	return 0;
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
            std::string value = JBPython::getInstallPath(std::string("2.7"));
        } catch(char *) {
            // creates a new colony downloader instance
            CColonyDownloader colonyDownloader = CColonyDownloader();

            // creates the window
            colonyDownloader.createDownloadWindow(handlerInstance, nCmdShow);

            // sets the base download address
            colonyDownloader.setBaseDownloadAddress(std::string("http://srio.hive:8080"));

            // adds the various download files, includes the hive python distribution,
            // the colony python installer and the colony python installer plugins
            CColonyDownloadItem firstDownloadItem = colonyDownloader.addDownloadFile(std::string("hive_python_2.6.2.tar.gz"), std::string("Hive Python Interpreter 2.6.2"));
            CColonyDownloadItem secondDownloadItem = colonyDownloader.addDownloadFile(std::string("colony_python_installer.tar.gz"), std::string("Colony Python Installer"));
            CColonyDownloadItem thirdDownloadItem = colonyDownloader.addDownloadFile(std::string("colony_python_installer_plugins.tar.gz"), std::string("Colony Python Installer Plugins"));

            // downloads the various files from their respective remote storage locations
            // in case the connection fails an exception is thrown
            colonyDownloader.downloadFiles();

            // unpacks the files and retrieves the target path
            std::string &targetPath = colonyDownloader.unpackFiles();

            // sets the plugin system path
            std::string &pluginSystemPath = std::string(targetPath + "/workspace/pt.hive.colony/src");
            std::string &pythonPath = std::string(targetPath + "/hive_python_2.6.2");

            // sets the executable path
            std::string &executablePath = std::string(targetPath);

            // changes the current directory to the python path
            SetCurrentDirectory(pythonPath.c_str());

            // loads the hive colony python installer library
            HINSTANCE colonyPythonInstallerLibrary = LoadLibrary((targetPath + "/" + HIVE_COLONY_PYTHON_INSTALLER_LIBRARY).c_str());

            // in case there is no colony python installer library (not
            // possible to retrieve the installer library)
            if(colonyPythonInstallerLibrary == NULL) {
                // throws an exception
                throw "Unable to retrieve the hive colony python installer library";
            }

            // retrieves the start installer procedure reference from the library
            void(*startInstaller)(std::string, std::string, std::string) = (void(*)(std::string, std::string, std::string)) GetProcAddress(colonyPythonInstallerLibrary, "startInstaller");

            // start the installer in the given directory
            startInstaller(pluginSystemPath, pythonPath, executablePath);

            if(!FreeLibrary(colonyPythonInstallerLibrary)) {
                throw "Unable to release the hive colony python installer library";
            }

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


int APIENTRY _tWinMain(HINSTANCE handlerInstance, HINSTANCE hPrevInstance, LPTSTR lpCmdLine, int nCmdShow) {
    // sets the handler instance global value, for latter
	// global reference (must be accessible by everyone)
    hInst = handlerInstance;

    // initializes the common controls and 
    // registers the (handler instance) class
    InitCommonControls();
    registerClass(handlerInstance);

	enum Operations_e operation = UNSET;

	char **argv;
	int argc;
	argv = CommandLineToArgvA(GetCommandLineA(), &argc);

	try{
		if(argc == 1) {
			operation = RUN;
		} else {
			if(!strcmp(argv[1], "run")) { operation = RUN; }
			else if(!strcmp(argv[1], "duplicate")) { operation = DUPLICATE; }
			else if(!strcmp(argv[1], "dump")) { operation = DUMP; }
			else if(!strcmp(argv[1], "pop")) { operation = POP; }
			else if(!strcmp(argv[1], "append")) { operation = APPEND; }
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

		case DUMP:
			return dump(argv, argc, handlerInstance, nCmdShow);
			break;
	}


/*
	struct DataFile_t dataFile;
	memcpy(dataFile.name, "tobias", 7);
	memcpy(dataFile.description, "", 1);
	memcpy(dataFile.url, "http://www.iol.pt", 18);

	//CData::appendDataFile("c:/capsule_d.exe", &dataFile);
	CData::popDataFile("c:/capsule_d.exe");



*/





    // returns normally
    return 0;
}
