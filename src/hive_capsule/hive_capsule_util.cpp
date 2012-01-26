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

#include <shellapi.h>
#include <string>

#include "hive_capsule_util.h"

int DeleteDirectoryRecursive(const std::string &rootDirectory, bool deleteSubdirectories) {
    bool subdirectory = false;
    HANDLE handlerFile;
    std::string filePath;
    std::string pattern;
    WIN32_FIND_DATA fileInformation;

    pattern = rootDirectory + "\\*.*";
    handlerFile = FindFirstFile(pattern.c_str(), &fileInformation);

    // in case the file handler is not invalid
    if(handlerFile != INVALID_HANDLE_VALUE) {
        do {
            if(fileInformation.cFileName[0] != '.') {
                filePath.erase();
                filePath = rootDirectory + "\\" + fileInformation.cFileName;

                if(fileInformation.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
                    if(deleteSubdirectories) {
                        // deletes the subdirectory
                        int returnValue = DeleteDirectoryRecursive(filePath, deleteSubdirectories);

                        // in case the return value is invalid
                        if(returnValue)
                            // returns the return value
                            return returnValue;
                    }
                    else
                        subdirectory = true;
                }
                else {
                    // sets the file attributes
                    if(SetFileAttributes(filePath.c_str(), FILE_ATTRIBUTE_NORMAL) == false)
                        // returns the last error
                        return GetLastError();

                    // deletes the file
                    if(DeleteFile(filePath.c_str()) == false)
                        // returns the last error
                        return GetLastError();
                }
            }
        } while(FindNextFile(handlerFile, &fileInformation) == TRUE);

        // closes the file handle
        FindClose(handlerFile);

        DWORD error = GetLastError();
        if(error != ERROR_NO_MORE_FILES)
            return error;
        else {
            if(!subdirectory) {
                // sets the directory attributes
                if(SetFileAttributes(rootDirectory.c_str(), FILE_ATTRIBUTE_NORMAL) == false)
                    // returns the last error
                    return GetLastError();

                // deletes the directory
                if(RemoveDirectory(rootDirectory.c_str()) == false)
                    // returns the last error
                    return GetLastError();
            }
        }
    }

    return 0;
}

int DeleteDirectoryRecursiveShell(const char *targetDirectory, bool recycleBin) {
    // retrieves the target directory length
    int targetDirectoryLength = strlen(targetDirectory);

    // allocates space
    TCHAR *targetDirectoryAux = new TCHAR[targetDirectoryLength + 2];

    strcpy_s(targetDirectoryAux, targetDirectoryLength + 1, targetDirectory);

    targetDirectoryAux[targetDirectoryLength] = 0;
    targetDirectoryAux[targetDirectoryLength + 1] = 0;

    SHFILEOPSTRUCT fileOperation;
    fileOperation.hwnd = NULL;

    // sets the operation as delete
    fileOperation.wFunc = FO_DELETE;

    // sets the source file name as double null terminated string
    fileOperation.pFrom = targetDirectoryAux;

    // no destination is required
    fileOperation.pTo = NULL;

    // do not prompt the user
    fileOperation.fFlags = FOF_NOCONFIRMATION | FOF_SILENT;

    if(!recycleBin)
        fileOperation.fFlags |= FOF_ALLOWUNDO;

    fileOperation.fAnyOperationsAborted = FALSE;
    fileOperation.lpszProgressTitle = NULL;
    fileOperation.hNameMappings = NULL;

    int returnValue = SHFileOperation(&fileOperation);

    delete [] targetDirectoryAux;

    return returnValue;
}

int GetWindowsMajorVersion() {
    // allocates space for the version variable
    unsigned long version;

    // retrievesthe current windows version
    version = GetVersion();

    // returns the major version
    return LOBYTE(LOWORD(version));
}
