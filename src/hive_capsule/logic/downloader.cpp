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

#include "../ui/ui.h"
#include "../main.h"
#include "observer.h"
#include "util.h"

#include "downloader.h"

CSDownloadItem::CSDownloadItem() {
}

CSDownloadItem::CSDownloadItem(std::string &name, std::string &description, std::string &address) {
    this->name = name;
    this->description = description;
    this->address = address;
    this->buffer = NULL;
    this->bufferSize = 0;
}

CSDownloadItem::CSDownloadItem(std::string &name, std::string &description, char *buffer, size_t bufferSize) {
    this->name = name;
    this->description = description;
    this->address = name;
    this->buffer = buffer;
    this->bufferSize = bufferSize;
}

CSDownloadItem::~CSDownloadItem() {
}

std::string &CSDownloadItem::getName() {
    return this->name;
}

std::string &CSDownloadItem::getDescription() {
    return this->description;
}

std::string &CSDownloadItem::getAddress() {
    return this->address;
}

char *CSDownloadItem::getBuffer() {
    return this->buffer;
}

size_t CSDownloadItem::getBufferSize() {
    return this->bufferSize;
}

CSDownloader::CSDownloader() {
    this->tempPath = "";
}

CSDownloader::~CSDownloader() {
}

void CSDownloader::loadItem(CSDownloadItem &downloadItem) {
    // allocates space for the file name
    char fileName[MAX_PATH];

    // retrieves the temp path in char
    const char *tempPathChar = this->getTempPath().c_str();

    // tries to retrieve a temporary file name
    if(!GetTempFileName(tempPathChar, TEMP_FILE_PREFIX, NULL, fileName)) {
        throw "Unable to create temporary file name";
    }

    // opens the file for read and write, writes the contents to the file
    // and closes it (avoid leaking)
    std::fstream file = std::fstream(fileName, std::fstream::in | std::fstream::out | std::fstream::trunc | std::fstream::binary);
    file.write(downloadItem.getBuffer(), downloadItem.getBufferSize());
    file.close();

    // sets the download item file path association in the map
    this->downloadItemsFilePathMap[downloadItem.getAddress()] = fileName;
}

void CSDownloader::downloadItem(CSDownloadItem &downloadItem) {
    // creates a new (jimbo) http client
    JBHttpClient httpClient = JBHttpClient();

    // creates a new download observer object
    CSDownloadObserver downloadObserver = CSDownloadObserver();

    // sets the window handler in the download observer
    downloadObserver.setWindowHandler(*(this->handlerWindowReference));

    // sets the progress handler in the download observer
    downloadObserver.setProgressHandler(handlerProgress);

    // sets the download item in the download observer
    downloadObserver.setDownloadItem(downloadItem);

    // registers the observer for the header loaded, download
    // changed and download completed events
    httpClient.registerObserverForEvent("header_loaded", downloadObserver);
    httpClient.registerObserverForEvent("download_changed", downloadObserver);
    httpClient.registerObserverForEvent("download_completed", downloadObserver);

    // retrieves the remote contents
    httpClient.getContents(downloadItem.getAddress());

    // allocates space for the file name
    char fileName[MAX_PATH];

    // retrieves the temp path in char
    const char *tempPathChar = this->getTempPath().c_str();

    // tries to retrieve a temporary file name
    if(!GetTempFileName(tempPathChar, TEMP_FILE_PREFIX, NULL, fileName)) {
        throw "Unable to create temporary file name";
    }

    // opens the file for read and write, writes the contents to the file
    // and closes it (avoid leaking)
    std::fstream file = std::fstream(fileName, std::fstream::in | std::fstream::out | std::fstream::trunc | std::fstream::binary);
    file.write(httpClient.getMessageBuffer(), httpClient.getMessageSize());
    file.close();

    // sets the download item file path association in the map
    this->downloadItemsFilePathMap[downloadItem.getAddress()] = fileName;
}

void CSDownloader::unpackItem(CSDownloadItem &downloadItem, std::string &targetPath) {
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

void CSDownloader::generateTempPath() {
    // allocates space for the temp path then retrieves
    // the local temp path (by copy) and converts it into
    // string setting it in the current object
    char tempPath[1024];
    GetTempPath(1024, tempPath);
    this->tempPath = std::string(tempPath);
}

std::string &CSDownloader::getTempPath() {
    // in case no temp path is currently set in the object
    // a new one must be generated (on demand generation)
    if(this->tempPath == "") { this->generateTempPath(); }

    // retrieves the current associated temp path, must be
    // unique in the system
    return this->tempPath;
}

void CSDownloader::createDownloadWindow(HINSTANCE handlerInstance, int nCmdShow) {
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

void CSDownloader::downloadFiles() {
    for(size_t index = 0; index < this->downloadItems.size(); index++) {
        CSDownloadItem &downloadItem = this->downloadItems[index];
        if(downloadItem.getBuffer() == NULL) { this->downloadItem(downloadItem); }
        else { this->loadItem(downloadItem); }
    }
}

std::string CSDownloader::unpackFiles(std::string targetPath) {
    // sends the message to change the label of the window,
    // then invalidates the window rectangle and after that
    // sends a message to change the progress bar to marquee
    SendMessage(*this->handlerWindowReference, changeLabelEventValue, (WPARAM) "Uncompressing installation files", NULL);
    InvalidateRect(*this->handlerWindowReference, NULL, true);
    SendMessage(*this->handlerWindowReference, changeProgressEventValue, 2, NULL);

    if(targetPath == "") {
        // allocates space for the file name
        char fileName[MAX_PATH];

        // retrieves the temp path in char
        const char *tempPathChar = this->getTempPath().c_str();

        // tries to retrieve a temporary file name
        if(!GetTempFileName(tempPathChar, TEMP_FILE_PREFIX, NULL, fileName)) {
            throw "Unable to create temporary file name";
        }

        // uses the temporary name to create a directory with it
        // appends a suffix to it for the purpose
        std::string &directoryName = std::string(fileName) + std::string("dir");

        // creates the temporary directory
        if(!CreateDirectory(directoryName.c_str(), NULL)) {
            throw "Unable to create directory";
        }

        // sets the current directory as the target path for
        // the unpacking of the files
        targetPath = directoryName;

        // adds the temporary file name to the list of temporary files
        this->temporaryFiles.push_back(std::string(fileName));
    }

    // iterates over all the downloaded items to unpack them
    // into the target path (directory)
    for(size_t index = 0; index < this->downloadItems.size(); index++) {
        // retrieves the current iteration download item
        // and then unpacks it into the target path
        CSDownloadItem &downloadItem = this->downloadItems[index];
        this->unpackItem(downloadItem, targetPath);
    }

    // adds the target path to the list of tempoary directories
    // (keeps track of them for latter removal)
    this->temporaryDirectories.push_back(targetPath);

    return targetPath;
}

std::string CSDownloader::deployFiles(std::string deployPath) {
    // prints an info message into the logger
    JBLogger::getLogger("setup")->info("Deploying files ...");

    // sends the message to change the label of the window,
    // then invalidates the window rectangle and after that
    // sends a message to change the progress bar to marquee
    SendMessage(*this->handlerWindowReference, changeLabelEventValue, (WPARAM) "Deploying files to destination", NULL);
    InvalidateRect(*this->handlerWindowReference, NULL, true);
    SendMessage(*this->handlerWindowReference, changeProgressEventValue, 2, NULL);

    // creates the directory for the deployment (this will ensure
    // that the directory exists)
    CreateDirectory(deployPath.c_str(), NULL);

    // iterates over all the temporary directories to copy them into
    // the "final" deploy target path
    for(size_t index = 0; index < this->temporaryDirectories.size(); index++) {
        std::string &temporaryDirectory = this->temporaryDirectories[index];
        JBWindows::copyRecursiveShell(deployPath, temporaryDirectory);
    }

    // returns the deploy path that was used in the deployment
    // phase (this should be used by the caller for confirmation)
    return deployPath;
}

void CSDownloader::deleteTemporaryFiles() {
    // prints an info message into the logger
    JBLogger::getLogger("setup")->info("Deleting temporary files ...");

    // sends the message to change the label of the window,
    // then invalidates the window rectangle and after that
    // sends a message to change the progress bar to marquee
    SendMessage(*this->handlerWindowReference, changeLabelEventValue, (WPARAM) "Deleting temporary files", NULL);
    InvalidateRect(*this->handlerWindowReference, NULL, true);
    SendMessage(*this->handlerWindowReference, changeProgressEventValue, 2, NULL);

    for(size_t index = 0; index < this->temporaryDirectories.size(); index++) {
        std::string &temporaryDirectory = this->temporaryDirectories[index];
        JBWindows::deleteRecursiveShell(temporaryDirectory.c_str(), false);
    }

    for(size_t index = 0; index < this->temporaryFiles.size(); index++) {
        std::string &temporaryFile = this->temporaryFiles[index];
        DeleteFile(temporaryFile.c_str());
    }

    // prints an info message into the logger
    JBLogger::getLogger("setup")->info("Finished deleting temporary files");

    // closes (destroy) the window in the most correct procedure
    // this call is done to prevent pending handles
    DestroyWindow(*this->handlerWindowReference);
}

void CSDownloader::setBaseDownloadAddress(std::string &baseDownloadAddress){
    this->baseDownloadAddress = baseDownloadAddress;
}

void CSDownloader::addDownloadItem(CSDownloadItem &downloadItem) {
    this->downloadItems.push_back(downloadItem);
}

CSDownloadItem &CSDownloader::addDownloadFile(std::string &name, std::string &description) {
    CSDownloadItem downloadItem = CSDownloadItem(name, description, this->baseDownloadAddress + "/" + name);

    this->downloadItems.push_back(downloadItem);

    return this->downloadItems.back();
}

std::string &CSDownloader::getDownloadItemFilePath(CSDownloadItem &downloadItem) {
    std::string &downloadItemAddress = downloadItem.getAddress();

    std::string &filePath = this->downloadItemsFilePathMap[downloadItemAddress];

    return filePath;
}
