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

#include "../ui/ui.h"
#include "../util.h"
#include "observer.h"

#include "downloader.h"

CColonyDownloadItem::CColonyDownloadItem() {
}

CColonyDownloadItem::CColonyDownloadItem(std::string &name, std::string &description, std::string &address) {
    this->name = name;
    this->description = description;
    this->address = address;
}

CColonyDownloadItem::~CColonyDownloadItem() {
}

std::string &CColonyDownloadItem::getName() {
    return this->name;
}

std::string &CColonyDownloadItem::getDescription() {
    return this->description;
}

std::string &CColonyDownloadItem::getAddress() {
    return this->address;
}

CColonyDownloader::CColonyDownloader() {
    this->tempPath = "";
};

CColonyDownloader::~CColonyDownloader() {
};

void CColonyDownloader::downloadItem(CColonyDownloadItem &downloadItem) {
    // creates a new (jimbo) http client
    JBHttpClient httpClient = JBHttpClient();

    // creates a new colony download observer object
    CColonyDownloadObserver colonyDownloadObserver = CColonyDownloadObserver();

    // sets the window handler in the colony download observer
    colonyDownloadObserver.setWindowHandler(*(this->handlerWindowReference));

    // sets the progress handler in the colony download observer
    colonyDownloadObserver.setProgressHandler(handlerProgress);

    // sets the download item in the colony download observer
    colonyDownloadObserver.setDownloadItem(downloadItem);

    // registers the observer for the header loaded event
    httpClient.registerObserverForEvent("header_loaded", colonyDownloadObserver);

    // registers the observer for the download changed event
    httpClient.registerObserverForEvent("download_changed", colonyDownloadObserver);

    // registers the observer for the download completed event
    httpClient.registerObserverForEvent("download_completed", colonyDownloadObserver);

    // retrieves the remote contents
    httpClient.getContents(downloadItem.getAddress());

    // allocates space for the file name
    char fileName[MAX_PATH];

    // retrieves the temp path in char
    const char *tempPathChar = this->getTempPath().c_str();

    // tries to retrieve a temporary file name
    if(!GetTempFileName(tempPathChar, TEMP_FILE_PREFIX, NULL, fileName))
        throw "Unable to create temporary file name";

    // opens the file for read and write
    std::fstream file = std::fstream(fileName, std::fstream::in | std::fstream::out | std::fstream::trunc | std::fstream::binary);

    // writes the contents to the file
    file.write(httpClient.getMessageBuffer(), httpClient.getMessageSize());

    // closes the file
    file.close();

    // sets the download item file path association in the map
    this->downloadItemsFilePathMap[downloadItem.getAddress()] = fileName;
}

void CColonyDownloader::unpackItem(CColonyDownloadItem &downloadItem, std::string &targetPath) {
    // retrieves the download item file path
    std::string &downloadItemFilePath = this->getDownloadItemFilePath(downloadItem);

    // constructs the tar file path from the download item file path
    std::string tarFilePath = downloadItemFilePath + ".tar";

    // unpacks the gz file to the tar file
    JBPackerGz::unpackFile(downloadItemFilePath, tarFilePath);

    // unpacks the tar file to the temp path
    JBPackerTar::unpackFile(tarFilePath, targetPath + "/");

    this->temporaryFiles.push_back(downloadItemFilePath);
    this->temporaryFiles.push_back(tarFilePath);
}

void CColonyDownloader::generateTempPath() {
    // allocates space for the temp path
    char tempPath[1024];

    // retrieves the local temp path
    GetTempPath(1024, tempPath);

    this->tempPath = std::string(tempPath);
}

std::string &CColonyDownloader::getTempPath() {
    if(this->tempPath == "")
        this->generateTempPath();

    return this->tempPath;
}

void CColonyDownloader::createDownloadWindow(HINSTANCE handlerInstance, int nCmdShow) {
    // allocates space for the thread id
    DWORD threadId;

    // creates the window event to controll the window creation
    HANDLE windowEvent = CreateEvent(NULL, false, false, NULL);

    // allocates space for the thread arguments
    std::vector<void *> threadArguments;

    // adds the arguments to the thread arguments list
    threadArguments.push_back(handlerInstance);
    threadArguments.push_back(&nCmdShow);
    threadArguments.push_back(&this->handlerWindowReference);
    threadArguments.push_back(windowEvent);

    // creates the thread and retrieves the thread handle
    HANDLE threadHandle = CreateThread(NULL, 0, windowThread, (void *) &threadArguments, 0, &threadId);

    // waits for the window event to be released
    WaitForSingleObject(windowEvent, INFINITE);

    // closes the window event handle
    CloseHandle(windowEvent);
}

void CColonyDownloader::downloadFiles() {
    BOOST_FOREACH(CColonyDownloadItem &downloadItem, this->downloadItems) {
        this->downloadItem(downloadItem);
    }
}

std::string CColonyDownloader::unpackFiles(std::string targetPath) {
    // sends the message to change the label of the window
    SendMessage(*this->handlerWindowReference, changeLabelEventValue, (WPARAM) "Uncompressing installation files", NULL);

    // invalidates the window rectangle
    InvalidateRect(*this->handlerWindowReference, NULL, true);

    // sends the message to change the progress bar to marquee
    SendMessage(*this->handlerWindowReference, changeProgressEventValue, 2, NULL);

    if(targetPath == "") {
        // allocates space for the file name
        char fileName[MAX_PATH];

        // retrieves the temp path in char
        const char *tempPathChar = this->getTempPath().c_str();

        // tries to retrieve a temporary file name
        if(!GetTempFileName(tempPathChar, TEMP_FILE_PREFIX, NULL, fileName))
            throw "Unable to create temporary file name";

        std::string &directoryName = std::string(fileName) + std::string("dir");

        // creates the temporary directory
        if(!CreateDirectory(directoryName.c_str(), NULL))
            throw "Unable to create directory";

        // converts the the char array to string
        targetPath = directoryName;

        // adds the temporary file name to the list of temporary files
        this->temporaryFiles.push_back(std::string(fileName));
    }

    BOOST_FOREACH(CColonyDownloadItem &downloadItem, this->downloadItems) {
        this->unpackItem(downloadItem, targetPath);
    }

    this->temporaryDirectories.push_back(targetPath);

    // closes the window
    SendMessage(*this->handlerWindowReference, WM_CLOSE, NULL, NULL);

    return targetPath;
}

void CColonyDownloader::deleteTemporaryFiles() {
    BOOST_FOREACH(std::string &temporaryDirectory, this->temporaryDirectories) {
        DeleteDirectoryRecursiveShell(temporaryDirectory.c_str(), false);
    }

    BOOST_FOREACH(std::string &temporaryFile, this->temporaryFiles) {
        DeleteFile(temporaryFile.c_str());
    }
}

void CColonyDownloader::setBaseDownloadAddress(std::string &baseDownloadAddress){
    this->baseDownloadAddress = baseDownloadAddress;
}

void CColonyDownloader::addDownloadItem(CColonyDownloadItem &colonyDownloadItem) {
    this->downloadItems.push_back(colonyDownloadItem);
}

CColonyDownloadItem &CColonyDownloader::addDownloadFile(std::string &name, std::string &description) {
    CColonyDownloadItem    colonyDownloadItem = CColonyDownloadItem(name, description, this->baseDownloadAddress + "/" + name);

    this->downloadItems.push_back(colonyDownloadItem);

    return this->downloadItems.back();
}

std::string &CColonyDownloader::getDownloadItemFilePath(CColonyDownloadItem &colonyDownloadItem) {
    std::string &colonyDownloadItemAddress = colonyDownloadItem.getAddress();

    std::string &filePath = this->downloadItemsFilePathMap[colonyDownloadItemAddress];

    return filePath;
}
